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

      _startType = SDB_START_NORMAL ;
      /* <-- internal status, can't be modified by config file --> */
      setDBStatus ( PMD_DB_NORMAL ) ;
      replaceDBFlag ( PMD_DBFLAG_WRITEABLE ) ;
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

   INT32 _SDB_KRCB::init ()
   {
      INT32 rc = SDB_OK ;

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

   INT32 _SDB_KRCB::enforceDiagLogPath ()
   {
      ossMemset ( _diagLogFile, 0, sizeof(_diagLogFile) ) ;

      INT32 rc = pmdBuildFullPath( _optioncb.krcbDiagLogPath(),
                                   PMD_DFT_DIAGLOG, OSS_MAX_PATHSIZE,
                                   _diagLogFile ) ;
      if ( SDB_OK == rc && 0 == ossStrlen( _pdDiagLogPath ) )
      {
         ossStrncpy( _pdDiagLogPath, _diagLogFile, sizeof(_pdDiagLogPath)-1 ) ;
      }
      return rc ;
   }

   INT32 _SDB_KRCB::enforceConfPath ()
   {
      ossMemset ( _confFile, 0, sizeof(_confFile) ) ;
      ossMemset ( _catFile,  0, sizeof(_catFile ) ) ;

      INT32 rc = pmdBuildFullPath( _optioncb.krcbConfPath(), PMD_DFT_CONF,
                                   OSS_MAX_PATHSIZE, _confFile ) ;
      if ( rc )
      {
         return rc ;
      }

      rc = pmdBuildFullPath( _optioncb.krcbConfPath(), PMD_DFT_CAT,
                             OSS_MAX_PATHSIZE, _catFile ) ;
      return rc ;
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

   void _SDB_KRCB::destroy ()
   {
      _eduMgr.reset () ;
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

