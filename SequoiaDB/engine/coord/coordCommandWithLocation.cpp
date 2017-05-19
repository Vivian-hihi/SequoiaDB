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

   Source File Name = coordCommandWithLocation.cpp

   Descriptive Name = Coord Commands

   When/how to use: this program may be used on binary and text-formatted
   versions of runtime component. This file contains code logic for
   user command processing on coordinator node.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================

   Last Changed =

*******************************************************************************/

#include "coordCommandWithLocation.hpp"
#include "pmd.hpp"
#include "rtnCB.hpp"
#include "msgMessage.hpp"
#include "pdTrace.hpp"
#include "coordTrace.hpp"

using namespace bson ;

namespace engine
{

   /*
      _coordCMDExpConfig implement
   */
   COORD_IMPLEMENT_CMD_AUTO_REGISTER( _coordCMDExpConfig,
                                      CMD_NAME_EXPORT_CONFIG,
                                      TRUE ) ;
   _coordCMDExpConfig::_coordCMDExpConfig()
   {
   }

   _coordCMDExpConfig::~_coordCMDExpConfig()
   {
   }

   INT32 _coordCMDExpConfig::_onLocalMode( INT32 flag )
   {
      return SDB_COORD_UNKNOWN_OP_REQ ;
   }

   void _coordCMDExpConfig::_preSet( pmdEDUCB *cb,
                                     coordCtrlParam &ctrlParam ) 
   {
      ctrlParam._isGlobal = FALSE ;
      ctrlParam._filterID = FILTER_ID_MATCHER ;
      ctrlParam._emptyFilterSel = NODE_SEL_ALL ;
      ctrlParam._role[ SDB_ROLE_CATALOG ] = 1 ;
      ctrlParam._role[ SDB_ROLE_DATA ] = 1 ;
   }

   UINT32 _coordCMDExpConfig::_getControlMask() const
   {
      return COORD_CTRL_MASK_ALL ;
   }

   INT32 _coordCMDExpConfig::_posExcute( MsgHeader *pMsg,
                                         pmdEDUCB *cb,
                                         ROUTE_RC_MAP &faileds )
   {
      /// do on local
      UINT32 mask = 0 ;
      INT32 rc = SDB_OK ;
      CHAR *pMatcherBuff = NULL ;

      rc = msgExtractQuery( (CHAR*)pMsg, NULL, NULL, NULL, NULL,
                            &pMatcherBuff, NULL, NULL, NULL ) ;

      try
      {
         BSONObj matcher( pMatcherBuff ) ;
         BSONElement e = matcher.getField( FIELD_NAME_REELECTION_LEVEL ) ;
         if ( e.eoo() )
         {
            mask = PMD_CFG_MASK_SKIP_UNFIELD ;
         }
         else if ( e.isNumber() )
         {
            INT32 type = e.numberInt() ;
            /// 0: ignore none
            /// 1: ignore hide default
            /// 2: ignore default
            /// 3: ignore unfield
            switch( type )
            {
               case 0 :
                  mask = 0 ;
                  break ;
               case 1 :
                  mask = PMD_CFG_MASK_SKIP_HIDEDFT ;
                  break ;
               case 2 :
                  mask = PMD_CFG_MASK_SKIP_HIDEDFT | PMD_CFG_MASK_SKIP_NORMALDFT ;
                  break ;
               default :
                  mask = PMD_CFG_MASK_SKIP_UNFIELD ;
                  break ;
            }
         }
         else
         {
            PD_LOG( PDERROR, "Field[%s] should be numberInt",
                    e.toString( TRUE, TRUE ).c_str() ) ;
            rc = SDB_INVALIDARG ;
            goto error ;
         }
      }
      catch( std::exception &e )
      {
         PD_LOG( PDERROR, "Occur exception: %s", e.what() ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }

      rc = pmdGetKRCB()->getOptionCB()->reflush2File( mask ) ;
      if ( rc )
      {
         PD_LOG( PDERROR, "Flush local config to file failed, rc: %d", rc ) ;
         goto error ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   /*
      _coordCMDInvalidateCache implement
   */
   COORD_IMPLEMENT_CMD_AUTO_REGISTER( _coordCMDInvalidateCache,
                                      CMD_NAME_INVALIDATE_CACHE,
                                      TRUE ) ;
   _coordCMDInvalidateCache::_coordCMDInvalidateCache()
   {
   }

   _coordCMDInvalidateCache::~_coordCMDInvalidateCache()
   {
   }

   void _coordCMDInvalidateCache::_preSet( pmdEDUCB * cb,
                                           coordCtrlParam & ctrlParam )
   {
      ctrlParam._isGlobal = TRUE ;
      ctrlParam._filterID = FILTER_ID_MATCHER ;
      ctrlParam._emptyFilterSel = NODE_SEL_ALL ;
   }

   INT32 _coordCMDInvalidateCache::_preExcute( MsgHeader *pMsg,
                                               pmdEDUCB *cb,
                                               coordCtrlParam &ctrlParam )
   {
      /// invalidate local catalog cache and group cache
      _pResource->invalidateCataInfo() ;
      _pResource->invalidateGroupInfo() ;
      return SDB_OK ;
   }

   UINT32 _coordCMDInvalidateCache::_getControlMask() const
   {
      return COORD_CTRL_MASK_ALL ;
   }

   /*
      _coordCMDSyncDB implement
   */
   COORD_IMPLEMENT_CMD_AUTO_REGISTER( _coordCMDSyncDB,
                                      CMD_NAME_SYNC_DB,
                                      TRUE ) ;
   _coordCMDSyncDB::_coordCMDSyncDB()
   {
   }

   _coordCMDSyncDB::~_coordCMDSyncDB()
   {
   }

   void _coordCMDSyncDB::_preSet( pmdEDUCB *cb,
                                  coordCtrlParam &ctrlParam )
   {
      ctrlParam._isGlobal = TRUE ;
      ctrlParam._filterID = FILTER_ID_MATCHER ;
      ctrlParam._emptyFilterSel = NODE_SEL_ALL ;
   }

   UINT32 _coordCMDSyncDB::_getControlMask() const
   {
      return COORD_CTRL_MASK_NODE_SELECT|COORD_CTRL_MASK_ROLE ;
   }

   INT32 _coordCMDSyncDB::_preExcute( MsgHeader *pMsg,
                                      pmdEDUCB *cb,
                                      coordCtrlParam &ctrlParam )
   {
      INT32 rc = SDB_OK ;
      CHAR *pQuery = NULL ;
      CHAR *pNewMsg = NULL ;
      rc = msgExtractQuery( (CHAR*)pMsg, NULL, NULL, NULL, NULL,
                            &pQuery, NULL, NULL, NULL ) ;
      if ( rc )
      {
         PD_LOG( PDERROR, "Extract message failed, rc: %d", rc ) ;
         goto error ;
      }

      try
      {
         const CHAR *csName = NULL ;
         BSONObj obj( pQuery ) ;
         BSONElement e = obj.getField( FIELD_NAME_COLLECTIONSPACE ) ;
         if ( String == e.type() )
         {
            csName = e.valuestr() ;
         }
         else if ( !e.eoo() )
         {
            PD_LOG( PDERROR, "Field[%s] is invalid in obj[%s]",
                    FIELD_NAME_COLLECTIONSPACE, obj.toString().c_str() ) ;
            rc = SDB_INVALIDARG ;
            goto error ;
         }

         if ( csName )
         {
            INT32 newMsgSize = 0 ;
            CoordGroupList grpLst ;
            rtnQueryOptions queryOpt ;

            queryOpt._fullName = "CAT" ;
            queryOpt._query = BSON( CAT_COLLECTION_SPACE_NAME << csName ) ;
            rc = queryOpt.toQueryMsg( &pNewMsg, newMsgSize, cb ) ;
            if ( rc )
            {
               PD_LOG( PDERROR, "Alloc query msg failed, rc: %d", rc ) ;
               goto error ;
            }
            /// change the opCode
            ((MsgHeader*)pNewMsg)->opCode = MSG_CAT_QUERY_SPACEINFO_REQ ;
            rc = executeOnCataGroup( (MsgHeader*)pNewMsg, cb, &grpLst ) ;
            if ( rc )
            {
               PD_LOG( PDERROR, "Query collectionspace[%s] info from catalog "
                       "failed, rc: %d", csName, rc ) ;
               goto error ;
            }
            ctrlParam._useSpecialGrp = TRUE ;
            ctrlParam._specialGrps = grpLst ;
         }
      }
      catch( std::exception &e )
      {
         PD_LOG( PDERROR, "Occur exception: %s", e.what() ) ;
         rc = SDB_SYS ;
         goto error ;
      }

   done:
      if ( pNewMsg )
      {
         msgReleaseBuffer( pNewMsg, cb ) ;
      }
      return rc ;
   error:
      goto done ;
   }

   /*
      _coordCmdLoadCS implement
   */
   COORD_IMPLEMENT_CMD_AUTO_REGISTER( _coordCmdLoadCS,
                                      CMD_NAME_LOAD_COLLECTIONSPACE,
                                      FALSE ) ;
   _coordCmdLoadCS::_coordCmdLoadCS()
   {
   }

   _coordCmdLoadCS::~_coordCmdLoadCS()
   {
   }

   void _coordCmdLoadCS::_preSet( pmdEDUCB * cb,
                                  coordCtrlParam &ctrlParam )
   {
      ctrlParam._isGlobal = TRUE ;
      ctrlParam._filterID = FILTER_ID_MATCHER ;
      ctrlParam._emptyFilterSel = NODE_SEL_ALL ;
      ctrlParam.resetRole() ;
      ctrlParam._role[ SDB_ROLE_DATA ] = 1 ;
   }

   UINT32 _coordCmdLoadCS::_getControlMask() const
   {
      return COORD_CTRL_MASK_NODE_SELECT ;
   }

   INT32 _coordCmdLoadCS::_preExcute( MsgHeader * pMsg,
                                      pmdEDUCB * cb,
                                      coordCtrlParam &ctrlParam )
   {
      INT32 rc = SDB_OK ;
      CHAR *pQuery = NULL ;
      CHAR *pNewMsg = NULL ;
      rc = msgExtractQuery( (CHAR*)pMsg, NULL, NULL, NULL, NULL,
                            &pQuery, NULL, NULL, NULL ) ;
      if ( rc )
      {
         PD_LOG( PDERROR, "Extract message failed, rc: %d", rc ) ;
         goto error ;
      }
      try
      {
         const CHAR *csName = NULL ;
         BSONObj obj( pQuery ) ;
         BSONElement e = obj.getField( FIELD_NAME_NAME ) ;
         if ( String == e.type() )
         {
            csName = e.valuestr() ;
         }
         else
         {
            PD_LOG( PDERROR, "Field[%s] is invalid in obj[%s]",
                    FIELD_NAME_NAME, obj.toString().c_str() ) ;
            rc = SDB_INVALIDARG ;
            goto error ;
         }

         {
            INT32 newMsgSize = 0 ;
            CoordGroupList grpLst ;
            rtnQueryOptions queryOpt ;

            queryOpt._fullName = "CAT" ;
            queryOpt._query = BSON( CAT_COLLECTION_SPACE_NAME << csName ) ;
            rc = queryOpt.toQueryMsg( &pNewMsg, newMsgSize, cb ) ;
            if ( rc )
            {
               PD_LOG( PDERROR, "Alloc query msg failed, rc: %d", rc ) ;
               goto error ;
            }
            /// chage the opCode
            ((MsgHeader*)pNewMsg)->opCode = MSG_CAT_QUERY_SPACEINFO_REQ ;
            rc = executeOnCataGroup( (MsgHeader*)pNewMsg, cb, &grpLst ) ;
            if ( rc )
            {
               PD_LOG( PDERROR, "Query collectionspace[%s] info from catalog "
                       "failed, rc: %d", csName, rc ) ;
               goto error ;
            }
            ctrlParam._useSpecialGrp = TRUE ;
            ctrlParam._specialGrps = grpLst ;
         }
      }
      catch( std::exception &e )
      {
         PD_LOG( PDERROR, "Occur exception: %s", e.what() ) ;
         rc = SDB_SYS ;
         goto error ;
      }

   done:
      if ( pNewMsg )
      {
         msgReleaseBuffer( pNewMsg, cb ) ;
      }
      return rc ;
   error:
      goto done ;
   }

   /*
      _coordCmdUnloadCS implement
   */
   _coordCommandAssit _coordCmdUnloadCSAssit ( CMD_NAME_UNLOAD_COLLECTIONSPACE,
      FALSE, (COORD_NEW_OPERATOR)_coordCmdUnloadCS::newThis ) ;

   /*
      _coordForceSession implement
   */
   COORD_IMPLEMENT_CMD_AUTO_REGISTER( _coordForceSession,
                                      CMD_NAME_FORCE_SESSION,
                                      TRUE ) ;
   _coordForceSession::_coordForceSession()
   {
   }

   _coordForceSession::~_coordForceSession()
   {
   }

   INT32 _coordForceSession::_onLocalMode( INT32 flag )
   {
      return SDB_COORD_UNKNOWN_OP_REQ ;
   }

   void _coordForceSession::_preSet( pmdEDUCB * cb,
                                     coordCtrlParam & ctrlParam )
   {
      ctrlParam._isGlobal = FALSE ;
      ctrlParam._filterID = FILTER_ID_MATCHER ;
      ctrlParam._emptyFilterSel = NODE_SEL_ALL ;
   }

   UINT32 _coordForceSession::_getControlMask() const
   {
      return COORD_CTRL_MASK_ALL ;
   }

   /*
      _coordSetPDLevel implement
   */
   COORD_IMPLEMENT_CMD_AUTO_REGISTER( _coordSetPDLevel,
                                      CMD_NAME_SET_PDLEVEL,
                                      TRUE ) ;
   _coordSetPDLevel::_coordSetPDLevel()
   {
   }

   _coordSetPDLevel::~_coordSetPDLevel()
   {
   }

   INT32 _coordSetPDLevel::_onLocalMode( INT32 flag )
   {
      return SDB_COORD_UNKNOWN_OP_REQ ;
   }

   void _coordSetPDLevel::_preSet( pmdEDUCB * cb,
                                   coordCtrlParam & ctrlParam )
   {
      ctrlParam._isGlobal = FALSE ;
      ctrlParam._filterID = FILTER_ID_MATCHER ;
      ctrlParam._emptyFilterSel = NODE_SEL_ALL ;
   }

   UINT32 _coordSetPDLevel::_getControlMask() const
   {
      return COORD_CTRL_MASK_ALL ;
   }

   /*
      _coordReloadConf implement
   */
   COORD_IMPLEMENT_CMD_AUTO_REGISTER( _coordReloadConf,
                                      CMD_NAME_RELOAD_CONFIG,
                                      TRUE ) ;
   _coordReloadConf::_coordReloadConf()
   {
   }

   _coordReloadConf::~_coordReloadConf()
   {
   }

   INT32 _coordReloadConf::_onLocalMode( INT32 flag )
   {
      return SDB_COORD_UNKNOWN_OP_REQ ;
   }

   void _coordReloadConf::_preSet( pmdEDUCB * cb,
                                   coordCtrlParam & ctrlParam )
   {
      ctrlParam._isGlobal = TRUE ;
      ctrlParam._filterID = FILTER_ID_MATCHER ;
      ctrlParam._emptyFilterSel = NODE_SEL_ALL ;
      ctrlParam._role[ SDB_ROLE_CATALOG ] = 1 ;
   }

   UINT32 _coordReloadConf::_getControlMask() const
   {
      return COORD_CTRL_MASK_ALL ;
   }

}

