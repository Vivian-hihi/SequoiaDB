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

namespace engine
{

   /*
      _SDB_KRCB implement
   */
   _SDB_KRCB::_SDB_KRCB ()
   {
      ossMemset( _hostName, 0, sizeof( _hostName ) ) ;

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
      _role = SDB_ROLE_STANDALONE ;

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

      // get hostname
      rc = ossGetHostName( _hostName, OSS_MAX_HOSTNAME ) ;
      PD_RC_CHECK( rc, PDERROR, "Failed to get host name, rc: %d", rc ) ;

      _role = pmdGetRoleEnum( _optioncb.krcbRole() ) ;
      pmdSetDBRole( _role ) ;

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

   void _SDB_KRCB::configChangeNty()
   {
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
         pCB->onConfigChange() ;
      }
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

