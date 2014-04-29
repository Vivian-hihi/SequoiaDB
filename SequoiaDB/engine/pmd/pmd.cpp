/*******************************************************************************


   Copyright (C) 2011-2014 SequoiaDB Ltd.

   This program is free software: you can redistribute it and/or modify
   it under the term of the GNU Affero General Public License, version 3,
   as published by the Free Software Foundation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warrenty of
   MARCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU Affero General Public License for more details.

   You should have received a copy of the GNU Affero General Public License
   along with this program. If not, see <http://www.gnu.org/license/>.

   Source File Name = pmd.cpp

   Descriptive Name = Process MoDel

   When/how to use: this program may be used on binary and text-formatted
   versions of PMD component. This file contains kernel control block object.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          09/14/2012  TW  Initial Draft

   Last Changed =

*******************************************************************************/
#include <string.h>
#include "core.hpp"
#include "pmd.hpp"
#include "pmdCB.hpp"
#include "pmdEDUMgr.hpp"

namespace engine
{

   /*
      _SDB_KRCB implement
   */
   _SDB_KRCB::_SDB_KRCB ()
   {
      _clsCB            = NULL ;
      _dpsTransCB       = NULL ;
      _dpscb            = NULL ;
      _dmscb            = NULL ;
      _rtncb            = NULL ;
      _bpscb            = NULL ;
      _catlogueCB       = NULL ;
      _coordcb          = NULL ;
      _sql              = NULL ;
      _auth             = NULL ;
      _traceCB          = NULL ;
      _aggrCB           = NULL ;
      _fmpCB            = NULL ;

      for ( INT32 i = 0 ; i < SDB_CB_MAX ; ++i )
      {
         _arrayCBs[ i ] = NULL ;
      }
      _init             = FALSE ;

      _startType = SDB_START_NORMAL ;
      /* <-- internal status, can't be modified by config file --> */
      setDBStatus ( PMD_DB_NORMAL ) ;
      /* <-- external status, can be changed by modifying config file --> */

      // standalone role by default, user may overwrite this setting
      enforceDBRole ( SDB_ROLE_STANDALONE ) ;

      setGroupName ( "" );
      // by default replication port is service port + 1

      // monitor switch initialization, no latch needed
      // for better performance these monitor swtich should be turned off
      // here, turn it on for testing
      _monCfgCB.timestampON = TRUE ;
      _monDBCB.recordActivateTimestamp () ;

      setBusinessOK( TRUE ) ;
      setExitCode( SDB_OK ) ;
   }

   _SDB_KRCB::~_SDB_KRCB ()
   {
      SAFE_DELETE( _clsCB ) ;
      SAFE_DELETE( _dpsTransCB ) ;
      SAFE_DELETE( _dpscb ) ;
      SAFE_DELETE( _dmscb ) ;
      SAFE_DELETE( _rtncb ) ;
      SAFE_DELETE( _bpscb ) ;
      SAFE_DELETE( _catlogueCB ) ;
      SAFE_DELETE( _coordcb ) ;
      SAFE_DELETE( _sql ) ;
      SAFE_DELETE( _auth ) ;
      SAFE_DELETE( _traceCB ) ;
      SAFE_DELETE( _aggrCB ) ;
      SAFE_DELETE( _fmpCB ) ;
   }

   IControlBlock* _SDB_KRCB::getCBByType( SDB_CB_TYPE type )
   {
      if ( (INT32)type < 0 || (INT32)type >= SDB_CB_MAX )
      {
         return NULL ;
      }
      return _arrayCBs[ type ] ;
   }

   BOOLEAN _SDB_KRCB::isCBValue( SDB_CB_TYPE type ) const
   {
      if ( (INT32)type < 0 || (INT32)type >= SDB_CB_MAX )
      {
         return FALSE ;
      }
      return _arrayCBs[ type ] ? TRUE : FALSE ;
   }

   INT32 _SDB_KRCB::registerCB( IControlBlock *pCB )
   {
      INT32 rc = SDB_OK ;

      SDB_ASSERT( pCB, "CB can't be NULL" ) ;
      SDB_ASSERT( FALSE == _init, "Registered cb must before init krcb" ) ;

      if ( (INT32)( pCB->cbType () ) < 0 ||
           (INT32)( pCB->cbType () ) >= SDB_CB_MAX )
      {
         rc = SDB_SYS ;
         goto error ;
      }
      _arrayCBs[ pCB->cbType () ] = pCB ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _SDB_KRCB::init ()
   {
      INT32 rc = SDB_OK ;
      INT32 index = 0 ;
      IControlBlock *pCB = NULL ;

      _init = TRUE ;

      // Init all registered cb
      for ( index = 0 ; index < SDB_CB_MAX ; ++index )
      {
         pCB = _arrayCBs[ index ] ;
         if ( !pCB )
         {
            continue ;
         }
         if ( rc = pCB->init() )
         {
            PD_LOG( PDERROR, "Init cb[Type: %d, Name: %s] failed, rc: %d",
                    pCB->cbType(), pCB->cbName(), rc ) ;
            goto error ;
         }
      }

      // Active all registered cb
      for ( index = 0 ; index < SDB_CB_MAX ; ++index )
      {
         pCB = _arrayCBs[ index ] ;
         if ( !pCB )
         {
            continue ;
         }
         if ( rc = pCB->active() )
         {
            PD_LOG( PDERROR, "Active cb[Type: %d, Name: %s] failed, rc: %d",
                    pCB->cbType(), pCB->cbName(), rc ) ;
            goto error ;
         }
      }

      SAFE_NEW_GOTO_ERROR( _dpsTransCB, dpsTransCB ) ;
      SAFE_NEW_GOTO_ERROR( _dpscb, SDB_DPSCB ) ;
      SAFE_NEW_GOTO_ERROR( _dmscb, SDB_DMSCB ) ;
      SAFE_NEW_GOTO_ERROR( _rtncb, SDB_RTNCB ) ;
      SAFE_NEW_GOTO_ERROR( _bpscb, SDB_BPSCB ) ;
      SAFE_NEW_GOTO_ERROR( _clsCB, clsCB ) ;
      SAFE_NEW_GOTO_ERROR( _catlogueCB, sdbCatalogueCB ) ;
      SAFE_NEW_GOTO_ERROR( _coordcb, CoordCB ) ;
      SAFE_NEW_GOTO_ERROR( _sql, SQL_CB ) ;
      SAFE_NEW_GOTO_ERROR( _auth, SDB_AUTHCB ) ;
      SAFE_NEW_GOTO_ERROR( _traceCB, pdTraceCB ) ;
      SAFE_NEW_GOTO_ERROR( _aggrCB, aggrBuilder ) ;
      SAFE_NEW_GOTO_ERROR( _fmpCB, spdFMPMgr ) ;
      gPDTraceCB = getTraceCB() ;

      _curTime.sample() ;

   done:
      return rc ;
   error:
      goto done ;
   }

   void _SDB_KRCB::destroy ()
   {
      INT32 rc = SDB_OK ;
      INT32 index = 0 ;
      IControlBlock *pCB = NULL ;

      // Deactive all registered cbs
      for ( index = 0 ; index < SDB_CB_MAX ; ++index )
      {
         pCB = _arrayCBs[ index ] ;
         if ( !pCB )
         {
            continue ;
         }
         if ( rc = pCB->deactive() )
         {
            PD_LOG( PDERROR, "Deactive cb[Type: %d, Name: %s] failed, rc: %d",
                    pCB->cbType(), pCB->cbName(), rc ) ;
         }
      }

      // stop all io services and edus(thread)
      _eduMgr.reset () ;

      // Fini all registered cbs
      for ( index = 0 ; index < SDB_CB_MAX ; ++index )
      {
         pCB = _arrayCBs[ index ] ;
         if ( !pCB )
         {
            continue ;
         }
         if ( rc = pCB->fini() )
         {
            PD_LOG( PDERROR, "Fini cb[Type: %d, Name: %s] failed, rc: %d",
                    pCB->cbType(), pCB->cbName(), rc ) ;
         }
      }
   }

   replCB* _SDB_KRCB::getReplCB ()
   {
      return _clsCB->getReplCB () ;
   }

   shardCB* _SDB_KRCB::getShardCB ()
   {
      return _clsCB->getShardCB () ;
   }

   void _SDB_KRCB::enforceNodeInfo ( const _MsgRouteID &id , const CHAR * host )
   {
      _clsCB->setNodeID ( id ) ;
      _clsCB->setHostName ( host ) ;
   }

   void _SDB_KRCB::enforceReplAddr( UINT16 serviceID, const CHAR *replService )
   {
      _clsCB->setReplServiceID( serviceID ) ;
      _clsCB->setReplServiceName ( replService ) ;
   }

   void _SDB_KRCB::enforceShardAddr ( UINT16 serviceID,
                                      const CHAR *shardService )
   {
      _clsCB->setShardServiceID ( serviceID ) ;
      _clsCB->setShardServiceName ( shardService ) ;
   }

   void _SDB_KRCB::enforceCataLogGrpAddrs( const _MsgRouteID &id,
                                           const CHAR *host,
                                           const CHAR *service )
   {
      _clsCB->setCatlogInfo ( id, host, service ) ;
      _coordcb->addCatNodeAddr ( id, host, service );
   }

   UINT32 _SDB_KRCB::getLogFileSz ()
   {
      return _dpscb->getLogFileSz () ;
   }

   UINT32 _SDB_KRCB::getLogFileNum ()
   {
      return _dpscb->getLogFileNum () ;
   }

   void _SDB_KRCB::enforceLogFileSz ( UINT32 logFileSz )
   {
      _dpscb->setLogFileSz ( logFileSz ) ;
   }

   void _SDB_KRCB::enforceLogFileNum ( UINT32 logFileNum )
   {
      _dpscb->setLogFileNum ( logFileNum ) ;
   }

   void _SDB_KRCB::enforceCatAddr( const _MsgRouteID &id,
                                   const CHAR *host,
                                   const CHAR *service )
   {
      _catlogueCB->setAddr(id, host, service);
   }

   void _SDB_KRCB::updateCatRouteID ( const _MsgRouteID &id )
   {
      _catlogueCB->updateRouteID ( id ) ;
   }

   ossTick _SDB_KRCB::getCurTime()
   {
      return _curTime ;
   }

   void _SDB_KRCB::syncCurTime()
   {
      _curTime.sample() ;
   }

   /*
    * kernel control block
    */
   pmdKRCB pmd_krcb ;

}

