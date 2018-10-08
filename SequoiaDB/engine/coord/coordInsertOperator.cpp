/*******************************************************************************

   Copyright (C) 2011-2018 SequoiaDB Ltd.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU Affero General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Affero General Public License for more details.

   You should have received a copy of the GNU Affero General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.

   Source File Name = coordInsertOperator.cpp

   Descriptive Name = Coord Insert

   When/how to use: this program may be used on binary and text-formatted
   versions of runtime component. This file contains code logic for
   insert options on coordinator node.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          13/04/2017  XJH Initial Draft
   Last Changed =

*******************************************************************************/

#include "coordInsertOperator.hpp"
#include "coordUtil.hpp"
#include "msgMessage.hpp"
#include "msgMessageFormat.hpp"
#include "rtnCommandDef.hpp"
#include "pdTrace.hpp"
#include "coordTrace.hpp"
#include "coordSequenceAgent.hpp"
#include <list>

using namespace bson ;

namespace engine
{

   /*
      _coordInsertOperator implement
   */
   _coordInsertOperator::_coordInsertOperator()
   {
      _insertedNum = 0 ;
      _ignoredNum = 0 ;

      const static string s_insertStr("Insert" ) ;
      setName( s_insertStr ) ;
      _pBoBuff = NULL ;
      _boBuffLen = 0 ;
      _boBuffSize = 0 ;
      _pOrgInsertor = NULL ;
      _boCount = 0 ;
   }

   _coordInsertOperator::~_coordInsertOperator()
   {
   }

   BOOLEAN _coordInsertOperator::isReadOnly() const
   {
      return FALSE ;
   }

   UINT32 _coordInsertOperator::getInsertedNum() const
   {
      return _insertedNum ;
   }

   UINT32 _coordInsertOperator::getIgnoredNum() const
   {
      return _ignoredNum ;
   }

   void _coordInsertOperator::clearStat()
   {
      _insertedNum = 0 ;
      _ignoredNum = 0 ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( COORD_INSERTOPR_EXE, "_coordInsertOperator::execute" )
   INT32 _coordInsertOperator::execute( MsgHeader *pMsg,
                                        pmdEDUCB *cb,
                                        INT64 &contextID,
                                        rtnContextBuf *buf )
   {
      INT32 rc = SDB_OK ;
      INT32 rcTmp = SDB_OK ;
      PD_TRACE_ENTRY ( COORD_INSERTOPR_EXE ) ;

      // process define
      coordSendOptions sendOpt( TRUE ) ;
      sendOpt._useSpecialGrp = TRUE ;

      coordSendMsgIn inMsg( pMsg ) ;
      coordProcessResult result ;
      ROUTE_RC_MAP nokRC ;
      result._pNokRC = &nokRC ;

      coordCataSel cataSel ;
      MsgRouteID errNodeID ;

      // fill default-reply(insert success)
      MsgOpInsert *pInsertMsg          = (MsgOpInsert *)pMsg ;
      INT32 oldFlag                    = pInsertMsg->flags ;
      pInsertMsg->flags               |= FLG_INSERT_RETURNNUM ;
      contextID                        = -1 ;

      INT32 flag = 0 ;
      CHAR *pCollectionName = NULL ;
      CHAR *pInsertor = NULL;
      INT32 count = 0 ;
      rc = msgExtractInsert( (CHAR*)pMsg, &flag,
                             &pCollectionName, &pInsertor, count ) ;
      if( rc )
      {
         PD_LOG( PDERROR, "Failed to parse insert request, rc: %d", rc ) ;
         pCollectionName = NULL ;
         goto error ;
      }

      if ( 0 == ossStrncmp( pCollectionName, CMD_ADMIN_PREFIX SYS_VIRTUAL_CS".",
                            SYS_VIRTUAL_CS_LEN + 1 ) )
      {
         rc = SDB_COORD_UNKNOWN_OP_REQ ;
         goto error ;
      }

      // add list op info
      MON_SAVE_OP_DETAIL( cb->getMonAppCB(), pMsg->opCode,
                          "Collection:%s, Insertors:%s, ObjNum:%d, "
                          "Flag:0x%08x(%u)",
                          pCollectionName,
                          BSONObj(pInsertor).toString().c_str(),
                          count, oldFlag, oldFlag ) ;

      rc = cataSel.bind( _pResource, pCollectionName, cb, FALSE, TRUE ) ;
      if ( rc )
      {
         PD_LOG( PDERROR, "Get or update collection[%s]'s catalog info "
                 "failed, rc: %d", pCollectionName, rc ) ;
         goto error ;
      }
      _pOrgInsertor = pInsertor ;
      _boCount = count ;

   retry:
      /// Do on collection
      pInsertMsg->version = cataSel.getCataPtr()->getVersion() ;
      pInsertMsg->w = 0 ;
      rcTmp = doOpOnCL( cataSel, BSONObj(), inMsg, sendOpt, cb, result ) ;
      if ( SDB_OK == rcTmp && nokRC.empty() )
      {
         goto done ;
      }
      else if ( checkRetryForCLOpr( rcTmp, &nokRC, cataSel, inMsg.msg(),
                                    cb, rc, &errNodeID, TRUE ) )
      {
         nokRC.clear() ;
         _groupSession.getGroupCtrl()->incRetry() ;
         goto retry ;
      }
      else
      {
         PD_LOG( PDERROR, "Insert failed on node[%s], rc: %d",
                 routeID2String( errNodeID ).c_str(), rc ) ;
         goto error ;
      }

   done:
      /// AUDIT
      if ( pCollectionName )
      {
         PD_AUDIT_OP( AUDIT_DML, MSG_BS_INSERT_REQ, AUDIT_OBJ_CL,
                      pCollectionName, rc, "InsertedNum:%u, IgnoredNum:%u, "
                      "ObjNum:%u, Insertor:%s, Flag:0x%08x(%u)", _insertedNum,
                      _ignoredNum, count, BSONObj(pInsertor).toString().c_str(),
                      oldFlag, oldFlag ) ;
      }
      if ( oldFlag & FLG_INSERT_RETURNNUM )
      {
         /// InsertedNum(Hi) + IgnoredNum(Lo)
         contextID = ossPack32To64( _insertedNum, _ignoredNum ) ;
      }
      msgReleaseBuffer( _pBoBuff, cb ) ;
      PD_TRACE_EXITRC ( COORD_INSERTOPR_EXE, rc ) ;
      return rc ;
   error:
      if ( buf && nokRC.size() > 0 )
      {
         *buf = rtnContextBuf( coordBuildErrorObj( _pResource, rc,
                                                   cb, &nokRC ) ) ;
      }
      goto done;
   }

   INT32 _coordInsertOperator::_prepareCLOp( coordCataSel &cataSel,
                                             coordSendMsgIn &inMsg,
                                             coordSendOptions &options,
                                             pmdEDUCB *cb,
                                             coordProcessResult &result )
   {
      INT32 rc = SDB_OK ;
      MsgOpInsert *pInsertMsg = ( MsgOpInsert* )inMsg.msg() ;
      netIOV fixed( ( CHAR*)inMsg.msg() + sizeof( MsgHeader ),
                    ossRoundUpToMultipleX ( offsetof(MsgOpInsert, name) +
                                            pInsertMsg->nameLength + 1, 4 ) -
                    sizeof( MsgHeader ) ) ;

      if ( cataSel.getCataPtr()->hasAutoIncrement() )
      {
         rc = _addAutoIncToData( cataSel.getCataPtr()->getAutoIncMap(),
                                 _pOrgInsertor, _boCount, cb,
                                 &_pBoBuff, _boBuffSize, _boBuffLen ) ;
         PD_RC_CHECK( rc, PDERROR,
                      "Failed to add autoIncrement fields to msg, rc: %d", rc ) ;

         inMsg._datas.clear() ;
         netIOVec &iovec = inMsg._datas[ cataSel.getCataPtr()->
                                         getGroupLst().begin()->first ] ;
         iovec.push_back( fixed ) ;
         iovec.push_back( netIOV( _pBoBuff, _boBuffLen ) ) ;
      }

      // clear send groups
      options._groupLst.clear() ;

      if ( !cataSel.getCataPtr()->isSharded() )
      {
         // get group
         cataSel.getCataPtr()->getGroupLst( options._groupLst ) ;
         // don't change the msg
         goto done ;
      }
      else if ( inMsg.data()->size() == 0 )
      {
         INT32 flag = 0 ;
         CHAR *pCollectionName = NULL ;
         CHAR *pInsertor = NULL ;
         INT32 count = 0 ;

         rc = msgExtractInsert( (CHAR *)inMsg.msg(), &flag, &pCollectionName,
                                &pInsertor, count ) ;
         PD_RC_CHECK( rc, PDERROR, "Extrace insert msg failed, rc: %d",
                      rc ) ;

         rc = shardDataByGroup( cataSel.getCataPtr(), count, pInsertor,
                                fixed, inMsg._datas ) ;
         PD_RC_CHECK( rc, PDERROR, "Failed to shard data by group, rc: %d",
                      rc ) ;

         // only one group, send by normal
         if ( 1 == inMsg._datas.size() )
         {
            UINT32 groupID = inMsg._datas.begin()->first ;
            options._groupLst[ groupID ] = groupID ;
            inMsg._datas.clear() ;
         }
         else
         {
            GROUP_2_IOVEC::iterator it = inMsg._datas.begin() ;
            while( it != inMsg._datas.end() )
            {
               options._groupLst[ it->first ] = it->first ;
               ++it ;
            }
         }
      }
      // reshard
      else
      {
         rc = reshardData( cataSel.getCataPtr(), fixed, inMsg._datas ) ;
         PD_RC_CHECK( rc, PDERROR, "Re-shard data failed, rc: %d", rc ) ;

         // build groups
         {
            GROUP_2_IOVEC::iterator it = inMsg._datas.begin() ;
            while( it != inMsg._datas.end() )
            {
               options._groupLst[ it->first ] = it->first ;
               ++it ;
            }
         }
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   void _coordInsertOperator::_doneCLOp( coordCataSel &cataSel,
                                         coordSendMsgIn &inMsg,
                                         coordSendOptions &options,
                                         pmdEDUCB *cb,
                                         coordProcessResult &result )
   {
      // remove the datas by succeed group
      if ( inMsg._datas.size() > 0 )
      {
         CoordGroupList::iterator it = result._sucGroupLst.begin() ;
         while( it != result._sucGroupLst.end() )
         {
            inMsg._datas.erase( it->second ) ;
            ++it ;
         }
      }

      // clear all succeed group
      result._sucGroupLst.clear() ;
   }

   void _coordInsertOperator::_prepareForTrans( pmdEDUCB *cb, MsgHeader *pMsg )
   {
      pMsg->opCode = MSG_BS_TRANS_INSERT_REQ ;
   }

   void _coordInsertOperator::_onNodeReply( INT32 processType,
                                            MsgOpReply *pReply,
                                            pmdEDUCB *cb,
                                            coordSendMsgIn &inMsg )
   {
      if ( pReply->contextID > 0 )
      {
         UINT32 hi1 = 0, lo1 = 0 ;
         /// (UINT32)insertedNum + (UINT32)ignoredNum
         ossUnpack32From64( pReply->contextID, hi1, lo1 ) ;
         _insertedNum += hi1 ;
         _ignoredNum += lo1 ;
      }
   }

   // PD_TRACE_DECLARE_FUNCTION ( COORD_INSERTOPR_SHARDANOBJ, "_coordInsertOperator::shardAnObj" )
   INT32 _coordInsertOperator::shardAnObj( const CHAR *pInsertor,
                                           CoordCataInfoPtr &cataInfo,
                                           const netIOV &fixed,
                                           GROUP_2_IOVEC &datas )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( COORD_INSERTOPR_SHARDANOBJ ) ;

      try
      {
         BSONObj insertObj( pInsertor ) ;
         UINT32 roundLen = ossRoundUpToMultipleX( insertObj.objsize(), 4 ) ;
         UINT32 groupID = 0 ;

         rc = cataInfo->getGroupByRecord( insertObj, groupID ) ;
         if ( rc )
         {
            PD_LOG( PDERROR, "Failed to get the groupid for obj[%s] from "
                    "catalog info[%s], rc: %d", insertObj.toString().c_str(),
                    cataInfo->toBSON().toString().c_str(), rc ) ;
            goto error ;
         }

         // add 2 group
         {
            netIOVec &iovec = datas[ groupID ] ;
            UINT32 size = iovec.size() ;
            if( size > 0 )
            {
               if ( (const CHAR*)( iovec[size-1].iovBase ) +
                    iovec[size-1].iovLen == pInsertor )
               {
                  // only change the length
                  iovec[size-1].iovLen += roundLen ;
               }
               else
               {
                  iovec.push_back( netIOV( pInsertor, roundLen ) ) ;
               }
            }
            else
            {
               iovec.push_back( fixed ) ;
               iovec.push_back( netIOV( pInsertor, roundLen ) ) ;
            }
         }
      }
      catch ( std::exception &e )
      {
         PD_LOG( PDERROR, "Failed to shard the data, received unexpected "
                 "error: %s", e.what() ) ;
         rc = SDB_SYS ;
         goto error ;
      }

   done:
      PD_TRACE_EXITRC ( COORD_INSERTOPR_SHARDANOBJ, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( COORD_INSERTOPR_SHARDBYGROUP, "_coordInsertOperator::shardDataByGroup" )
   INT32 _coordInsertOperator::shardDataByGroup( CoordCataInfoPtr &cataInfo,
                                                 INT32 count,
                                                 const CHAR *pInsertor,
                                                 const netIOV &fixed,
                                                 GROUP_2_IOVEC &datas )
   {
      INT32 rc = SDB_OK;
      PD_TRACE_ENTRY ( COORD_INSERTOPR_SHARDBYGROUP ) ;

      while ( count > 0 )
      {
         rc = shardAnObj( pInsertor, cataInfo, fixed, datas ) ;
         if ( rc )
         {
            PD_LOG( PDERROR, "Failed to shard the obj, rc: %d", rc ) ;
            goto error ;
         }

         try
         {
            BSONObj boInsertor( pInsertor ) ;
            pInsertor += ossRoundUpToMultipleX( boInsertor.objsize(), 4 ) ;
         }
         catch ( std::exception &e )
         {
            PD_LOG( PDERROR, "Parse insert object occur exception: %s",
                    e.what() ) ;
            rc = SDB_SYS ;
            goto error ;
         }
         --count ;
      }

   done:
      PD_TRACE_EXITRC ( COORD_INSERTOPR_SHARDBYGROUP, rc ) ;
      return rc ;
   error:
      datas.clear() ;
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( COORD_INSERTOPR_RESHARD, "_coordInsertOperator::reshardData" )
   INT32 _coordInsertOperator::reshardData( CoordCataInfoPtr &cataInfo,
                                            const netIOV &fixed,
                                            GROUP_2_IOVEC &datas )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( COORD_INSERTOPR_RESHARD ) ;

      const CHAR *pData = NULL ;
      UINT32 offset = 0 ;
      UINT32 roundSize = 0 ;

      GROUP_2_IOVEC newDatas ;
      GROUP_2_IOVEC::iterator it = datas.begin() ;
      while ( it != datas.end() )
      {
         netIOVec &iovec = it->second ;
         UINT32 size = iovec.size() ;
         // skip the first
         for ( UINT32 i = 1 ; i < size ; ++i )
         {
            netIOV &ioItem = iovec[ i ] ;
            pData = ( const CHAR* )ioItem.iovBase ;
            offset = 0 ;

            while( offset < ioItem.iovLen )
            {
               try
               {
                  BSONObj obInsert( pData ) ;
                  roundSize = ossRoundUpToMultipleX( obInsert.objsize(), 4 ) ;
               }
               catch( std::exception &e )
               {
                  PD_LOG( PDERROR, "Parse the insert object occur "
                          "exception: %s", e.what() ) ;
                  rc = SDB_SYS ;
                  goto error ;
               }

               rc = shardAnObj( pData, cataInfo, fixed, newDatas ) ;
               if ( rc )
               {
                  PD_LOG( PDERROR, "Reshard the insert record failed, rc: %d",
                          rc ) ;
                  goto error ;
               }

               pData += roundSize ;
               offset += roundSize ;
            }
         }
         ++it ;
      }
      datas = newDatas ;

   done:
      PD_TRACE_EXITRC ( COORD_INSERTOPR_RESHARD, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   INT32 _coordInsertOperator::_prepareMainCLOp( coordCataSel &cataSel,
                                                 coordSendMsgIn &inMsg,
                                                 coordSendOptions &options,
                                                 pmdEDUCB *cb,
                                                 coordProcessResult &result )
   {
      INT32 rc = SDB_OK ;
      GROUP_2_IOVEC::iterator it ;
      INT32 offset = 0 ;
      INT32 roundSize = 0 ;

      MsgOpInsert *pInsertMsg = ( MsgOpInsert* )inMsg.msg() ;
      netIOV fixed( ( CHAR*)inMsg.msg() + sizeof( MsgHeader ),
                    ossRoundUpToMultipleX ( offsetof(MsgOpInsert, name) +
                                            pInsertMsg->nameLength + 1, 4 ) -
                    sizeof( MsgHeader ) ) ;

      if ( cataSel.getCataPtr()->hasAutoIncrement() )
      {
         rc = _addAutoIncToData( cataSel.getCataPtr()->getAutoIncMap(),
                                 _pOrgInsertor, _boCount, cb,
                                 &_pBoBuff, _boBuffSize, _boBuffLen ) ;
         PD_RC_CHECK( rc, PDERROR,
                      "Failed to add autoIncrement fields to msg, rc: %d", rc ) ;

         _grpSubCLDatas.clear() ;
         netIOVec &ioVec = (_grpSubCLDatas[ CAT_INVALID_GROUPID ])[ "SYS_TMP" ] ;
         while ( offset < _boBuffLen )
         {
            BSONObj obj( _pBoBuff + offset ) ;
            roundSize = ossRoundUpToMultipleX( obj.objsize(), 4 ) ;
            ioVec.push_back( netIOV( obj.objdata(), roundSize ) ) ;
            offset += roundSize ;
         }
      }

      if ( _grpSubCLDatas.size() == 0 )
      {
         INT32 flag = 0 ;
         CHAR *pCollectionName = NULL ;
         CHAR *pInsertor = NULL ;
         INT32 count = 0 ;

         rc = msgExtractInsert( (CHAR *)inMsg.msg(), &flag, &pCollectionName,
                                &pInsertor, count ) ;
         PD_RC_CHECK( rc, PDERROR, "Extrace insert msg failed, rc: %d",
                      rc ) ;

         rc = shardDataByGroup( cataSel.getCataPtr(), count, pInsertor,
                                cb, _grpSubCLDatas ) ;
         PD_RC_CHECK( rc, PDERROR, "Failed to shard data by group, rc: %d",
                      rc ) ;
      }
      else
      {
         rc = reshardData( cataSel.getCataPtr(), cb, _grpSubCLDatas ) ;
         PD_RC_CHECK( rc, PDERROR, "Re-shard data failed, rc: %d", rc ) ;
      }

      // build msg
      inMsg._datas.clear() ;

      rc = buildInsertMsg( fixed, _grpSubCLDatas, _vecObject, inMsg._datas ) ;
      PD_RC_CHECK( rc, PDERROR, "Build insert msg failed, rc: %d", rc ) ;

      // clear send groups
      options._groupLst.clear() ;
      // build group list
      it = inMsg._datas.begin() ;
      while( it != inMsg._datas.end() )
      {
         options._groupLst[ it->first ] = it->first ;
         ++it ;
      }

   done:
      return rc ;
   error:
      _vecObject.clear() ;
      goto done ;
   }

   void _coordInsertOperator::_doneMainCLOp( coordCataSel &cataSel,
                                             coordSendMsgIn &inMsg,
                                             coordSendOptions &options,
                                             pmdEDUCB *cb,
                                             coordProcessResult &result )
   {
      // remove the datas by succeed group
      if ( _grpSubCLDatas.size() > 0 )
      {
         CoordGroupList::iterator it = result._sucGroupLst.begin() ;
         while( it != result._sucGroupLst.end() )
         {
            _grpSubCLDatas.erase( it->second ) ;
            ++it ;
         }
      }

      // clear all succeed group
      result._sucGroupLst.clear() ;

      _vecObject.clear() ;
   }

   INT32 _coordInsertOperator::shardAnObj( const CHAR *pInsertor,
                                           CoordCataInfoPtr &cataInfo,
                                           pmdEDUCB * cb,
                                           GroupSubCLMap &groupSubCLMap )
   {
      INT32 rc = SDB_OK ;
      string subCLName ;
      UINT32 groupID = CAT_INVALID_GROUPID ;

      try
      {
         BSONObj insertObj( pInsertor ) ;
         CoordCataInfoPtr subClCataInfo ;
         UINT32 roundLen = ossRoundUpToMultipleX( insertObj.objsize(), 4 ) ;

         rc = cataInfo->getSubCLNameByRecord( insertObj, subCLName ) ;
         if ( rc )
         {
            PD_LOG( PDWARNING, "Couldn't find the match[%s] sub-collection "
                    "in cl's(%s) catalog info[%s], rc: %d",
                    insertObj.toString().c_str(), cataInfo->getName(),
                    cataInfo->toBSON().toString().c_str(),
                    rc ) ;
            goto error ;
         }

         /// get sub-collection's catalog info
         rc = _pResource->getOrUpdateCataInfo( subCLName.c_str(),
                                               subClCataInfo, cb ) ;
         if ( rc )
         {
            PD_LOG( PDWARNING, "Get sub-collection[%s]'s catalog info "
                    "failed, rc: %d", subCLName.c_str(), rc ) ;
            goto error ;
         }

         rc = subClCataInfo->getGroupByRecord( insertObj, groupID ) ;
         if ( rc )
         {
            PD_LOG( PDWARNING, "Couldn't find the match[%s] catalog of "
                    "sub-collection(%s), rc: %d",
                    insertObj.toString().c_str(),
                    subClCataInfo->toBSON().toString().c_str(),
                    rc ) ;
            goto error ;
         }

         /// add to group
         (groupSubCLMap[ groupID ])[ subCLName ].push_back(
            netIOV( (const void*)pInsertor, roundLen ) ) ;
      }
      catch ( std::exception &e )
      {
         PD_LOG( PDERROR, "Parse the insert object occur exception: %s",
                 e.what() ) ;
         rc = SDB_SYS ;
         goto error ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _coordInsertOperator::shardDataByGroup( CoordCataInfoPtr &cataInfo,
                                                 INT32 count,
                                                 const CHAR *pInsertor,
                                                 pmdEDUCB *cb,
                                                 GroupSubCLMap &groupSubCLMap )
   {
      INT32 rc = SDB_OK ;

      while ( count > 0 )
      {
         rc = shardAnObj( pInsertor, cataInfo, cb, groupSubCLMap ) ;
         if ( rc )
         {
            PD_LOG( PDERROR, "Failed to shard the object, rc: %d", rc ) ;
            goto error ;
         }

         try
         {
            BSONObj boInsertor( pInsertor ) ;
            pInsertor += ossRoundUpToMultipleX( boInsertor.objsize(), 4 ) ;
            --count ;
         }
         catch ( std::exception &e )
         {
            PD_LOG( PDERROR, "Parse the insert reocrd occur exception: %s",
                    e.what() ) ;
            rc = SDB_SYS ;
            goto error ;
         }
      }

   done:
      return rc;
   error:
      groupSubCLMap.clear() ;
      goto done ;
   }

   INT32 _coordInsertOperator::reshardData( CoordCataInfoPtr &cataInfo,
                                            pmdEDUCB *cb,
                                            GroupSubCLMap &groupSubCLMap )
   {
      INT32 rc = SDB_OK ;
      GroupSubCLMap groupSubCLMapNew ;

      GroupSubCLMap::iterator iterGroup = groupSubCLMap.begin() ;
      while ( iterGroup != groupSubCLMap.end() )
      {
         SubCLObjsMap::iterator iterCL = iterGroup->second.begin() ;
         while( iterCL != iterGroup->second.end() )
         {
            netIOVec &iovec = iterCL->second ;
            UINT32 size = iovec.size() ;

            for ( UINT32 i = 0 ; i < size ; ++i )
            {
               netIOV &ioItem = iovec[ i ] ;
               rc = shardAnObj( (const CHAR*)ioItem.iovBase, cataInfo,
                                cb, groupSubCLMapNew ) ;
               if ( rc )
               {
                  PD_LOG( PDWARNING, "Failed to reshard the object, rc: %d",
                          rc ) ;
                  goto error ;
               }
            }
            ++iterCL ;
         }
         ++iterGroup ;
      }
      groupSubCLMap = groupSubCLMapNew ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _coordInsertOperator::buildInsertMsg( const netIOV &fixed,
                                               GroupSubCLMap &groupSubCLMap,
                                               vector< BSONObj > &subClInfoLst,
                                               GROUP_2_IOVEC &datas )
   {
      INT32 rc = SDB_OK ;
      static CHAR _fillData[ 8 ] = { 0 } ;

      GroupSubCLMap::iterator iterGroup = groupSubCLMap.begin() ;
      while ( iterGroup != groupSubCLMap.end() )
      {
         UINT32 groupID = iterGroup->first ;
         netIOVec &iovec = datas[ groupID ] ;
         iovec.push_back( fixed ) ;

         SubCLObjsMap &subCLDataMap = iterGroup->second ;
         SubCLObjsMap::iterator iterCL = subCLDataMap.begin() ;
         while ( iterCL != iterGroup->second.end() )
         {
            netIOVec &subCLIOVec = iterCL->second ;
            UINT32 dataLen = netCalcIOVecSize( subCLIOVec ) ;
            UINT32 objNum = subCLIOVec.size() ;

            // first for sub cl info
            BSONObjBuilder subCLInfoBuild ;
            subCLInfoBuild.append( FIELD_NAME_SUBOBJSNUM, (INT32)objNum ) ;
            subCLInfoBuild.append( FIELD_NAME_SUBOBJSSIZE, (INT32)dataLen ) ;
            subCLInfoBuild.append( FIELD_NAME_SUBCLNAME, iterCL->first ) ;
            BSONObj subCLInfoObj = subCLInfoBuild.obj() ;
            subClInfoLst.push_back( subCLInfoObj ) ;
            netIOV ioCLInfo ;
            ioCLInfo.iovBase = (const void*)subCLInfoObj.objdata() ;
            ioCLInfo.iovLen = subCLInfoObj.objsize() ;
            iovec.push_back( ioCLInfo ) ;

            // need fill
            UINT32 infoRoundSize = ossRoundUpToMultipleX( ioCLInfo.iovLen,
                                                          4 ) ;
            if ( infoRoundSize > ioCLInfo.iovLen )
            {
               iovec.push_back( netIOV( (const void*)_fillData,
                                infoRoundSize - ioCLInfo.iovLen ) ) ;
            }

            for ( UINT32 i = 0 ; i < objNum ; ++i )
            {
               iovec.push_back( subCLIOVec[ i ] ) ;
            }
            ++iterCL ;
         }
         ++iterGroup ;
      }

      return rc ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__ADD_AUTOINC_TO_DATA, "_coordInsertOperator::_addAutoIncToData" )
   INT32 _coordInsertOperator::_addAutoIncToData( const AUTOINC_ITEM_MAP &autoIncMap,
                                                  CHAR *pInsertor,
                                                  INT32 count,
                                                  pmdEDUCB *cb,
                                                  CHAR **ppBuff,
                                                  INT32 &buffSize,
                                                  INT32 &buffLen )
   {
      PD_TRACE_ENTRY( SDB__ADD_AUTOINC_TO_DATA ) ;

      INT32       rc = SDB_OK ;
      INT32       i = 0 ;
      INT32       newBuffLen = 0 ;
      BufBuilder  bufBuilder ;

      buffLen = 0 ;

      for ( i = 0 ; i < count ; ++i )
      {
         // add field to obj
         const BSONObj objIn( (const CHAR*)pInsertor ) ;
         rc = _addAutoIncToObj( objIn, autoIncMap, cb, bufBuilder ) ;
         PD_RC_CHECK( rc, PDERROR,
                      "Failed to add autoIncrement field to obj[%s], rc: %d",
                      objIn.toString().c_str(), rc ) ;

         // append obj to buff
         newBuffLen = buffLen + ossRoundUpToMultipleX( bufBuilder.len(), 4 ) ;
         rc = msgCheckBuffer( ppBuff, &buffSize, newBuffLen, cb ) ;
         PD_RC_CHECK( rc, PDERROR, "Failed to malloc buffer, rc: %d", rc ) ;
         ossMemcpy( *ppBuff + buffLen, bufBuilder.buf(), bufBuilder.len() ) ;
         buffLen = newBuffLen ;

         pInsertor += ossRoundUpToMultipleX( objIn.objsize(), 4 ) ;
      }

   done:
      PD_TRACE_EXIT( SDB__ADD_AUTOINC_TO_DATA ) ;
      return rc ;
   error:
      buffLen = 0 ;
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__ADD_AUTOINC_TO_OBJ, "_coordInsertOperator::_addAutoIncToObj" )
   INT32 _coordInsertOperator::_addAutoIncToObj( const BSONObj &objIn,
                                                 const AUTOINC_ITEM_MAP &autoIncMap,
                                                 pmdEDUCB *cb,
                                                 BufBuilder &bufBuilder )
   {
      PD_TRACE_ENTRY( SDB__ADD_AUTOINC_TO_OBJ ) ;

      INT32                      rc = SDB_OK ;
      BSONElement                ele ;
      const CHAR*                eleField = NULL;
      BSONObj                    subObjIn ;
      BufBuilder                 subBuilder ;

      AUTOINC_ITEM_MAP_CONST_IT  autoIncIt ;
      const coordAutoIncItem*    pItem = NULL ;
      _utilArray<const CHAR*>    doneArray ;
      BOOLEAN                    isDone = FALSE ;
      const CHAR*                doneField = NULL ;

      coordSequenceAgent*        pSequenceAgent = NULL ;
      INT64                      nextValue = 0 ;

      try
      {
         bufBuilder.reset() ;
         bufBuilder.appendNum( (UINT32) 0 ) ;// bson length
         bufBuilder.reserveBytes(1); // reserve for EOO

         // 1. Handle autoIncrement fields inputted by user.

         BSONObjIterator boIt( objIn ) ;

         while( boIt.more() )
         {
            ele = boIt.next() ;
            eleField = ele.fieldName() ;
            autoIncIt = autoIncMap.find( eleField ) ;
            if ( autoIncIt == autoIncMap.end() )
            {
               bufBuilder.appendBuf( (void*)ele.rawdata(), ele.size() ) ;
               continue ;
            }

            if ( !autoIncIt->second->hasSubField() )
            {
               switch( autoIncIt->second->generatedType() )
               {
               case AUTOINC_GEN_ALWAYS:
                  break ;
               case AUTOINC_GEN_STRICT:
                  PD_CHECK( NumberInt == ele.type() || NumberLong == ele.type(),
                            SDB_INVALIDARG, error, PDERROR,
                            "Wrong type[%d] of autoIncrement field[%s]",
                            ele.type(), eleField ) ;
               case AUTOINC_GEN_DEFAULT:
                  bufBuilder.appendBuf( (void*)ele.rawdata(), ele.size() ) ;
                  doneArray.append( autoIncIt->first ) ;
                  break ;
               }
            }
            else
            {
               if ( Object == ele.type() )
               {
                  subObjIn = ele.Obj() ;
                  rc = _addAutoIncToObj( subObjIn, *(autoIncIt->second->subFieldMap()),
                                         cb, subBuilder ) ;
                  PD_RC_CHECK( rc, PDERROR, "Failed to add autoIncrement field[%s], rc: %d",
                               eleField, rc ) ;

                  bufBuilder.appendNum( (CHAR) Object ) ;
                  bufBuilder.appendStr( eleField ) ;
                  bufBuilder.appendBuf( subBuilder.buf(), subBuilder.len() ) ;

                  doneArray.append( autoIncIt->first ) ;
               }
               else
               {
                  // autoIncrement field is "a.b", and user just input field "a".
                  PD_RC_CHECK( SDB_INVALIDARG, PDERROR,
                               "Field[%s] conflicted with autoIncrement field",
                               eleField ) ;
               }
            }
         }

         // 2. Complete the rest of autoIncrement fields.

         pSequenceAgent = _pResource->getSequenceAgent() ;

         for ( autoIncIt = autoIncMap.begin() ;
               autoIncIt != autoIncMap.end() ;
               ++autoIncIt )
         {
            _utilArray<const CHAR*>::iterator doneIt( doneArray ) ;
            isDone = FALSE ;
            while ( doneIt.next( doneField ) )
            {
               if ( 0 == ossStrcmp( doneField, autoIncIt->first ) )
               {
                  isDone = TRUE ;
                  break ;
               }
            }
            if ( isDone ) continue ;

            pItem = autoIncIt->second ;
            if ( !pItem->hasSubField() )
            {
               rc = pSequenceAgent->getNextValue( pItem->sequenceName(), nextValue, cb ) ;
               PD_RC_CHECK( rc, PDERROR,
                            "Failed to get sequence[%s] next value, rc: %d",
                            pItem->sequenceName(), rc ) ;

               bufBuilder.appendNum( (CHAR) NumberLong ) ;
               bufBuilder.appendStr( pItem->fieldName() ) ;
               bufBuilder.appendNum( nextValue ) ;
            }
            else
            {
               subObjIn = BSONObjBuilder().obj() ;
               rc = _addAutoIncToObj( subObjIn, *(pItem->subFieldMap()),
                                      cb, subBuilder ) ;
               PD_RC_CHECK( rc, PDERROR, "Failed to add autoIncrement field[%s], rc: %d",
                            pItem->fieldName(), rc ) ;

               bufBuilder.appendNum( (CHAR) Object ) ;
               bufBuilder.appendStr( pItem->fieldName() ) ;
               bufBuilder.appendBuf( subBuilder.buf(), subBuilder.len() ) ;
            }

            doneArray.append( autoIncIt->first ) ;
         }

         bufBuilder.claimReservedBytes(1) ;  // Prevents adding EOO from failing.
         bufBuilder.appendNum( (CHAR) EOO ) ;
         *((INT32*)bufBuilder.buf()) = bufBuilder.len() ; // set bson length

      }
      catch ( std::exception &e )
      {
         PD_RC_CHECK( SDB_SYS, PDERROR, "Unexpected exception happened[%s]", e.what() ) ;
      }

   done:
      PD_TRACE_EXIT( SDB__ADD_AUTOINC_TO_OBJ ) ;
      return rc ;
   error:
      goto done ;
   }

}

