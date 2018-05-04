/*******************************************************************************


   Copyright (C) 2011-2018 SequoiaDB Ltd.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

   Source File Name = seAdptIndexSession.hpp

   Descriptive Name = Index session on search engine adapter.

   When/how to use: this program may be used on binary and text-formatted
   versions of PMD component. This file contains main function for sdbcm,
   which is used to do cluster managing.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          04/14/2017  YSD  Initial Draft

   Last Changed =

*******************************************************************************/
#ifndef SEADPT_INDEX_SESSION_HPP_
#define SEADPT_INDEX_SESSION_HPP_

#include "utilESClt.hpp"
#include "pmdAsyncSession.hpp"
#include "dmsExtDataHandler.hpp"
#include "rtnExtOprDef.hpp"
#include "seAdptMgr.hpp"
#include "utilESBulkBuilder.hpp"

using namespace bson ;

#define SEADPT_FIELD_NAME_ID         "_id"

namespace seadapter
{
   enum SEADPT_SESSION_STATUS
   {
      SEADPT_SESSION_STAT_CONSULT = 1,       // To find where to start.
      SEADPT_SESSION_STAT_BEGIN,             // Start from the beginning,
                                             // query normal collection.
      SEADPT_SESSION_STAT_UPDATE_CL_VERSION, // Update collection version.
      SEADPT_SESSION_STAT_QUERY_LAST_LID,
      SEADPT_SESSION_STAT_COMP_LAST_LID,
      SEADPT_SESSION_STAT_QUERY_NORMAL_TBL,
      SEADPT_SESSION_STAT_QUERY_CAP_TBL,
      SEADPT_SESSION_STAT_POP_CAP,
      SEADPT_SESSION_STAT_MAX
   } ;

   // Indexer sessions on search engine adapter.
   class _seAdptIndexSession : public _pmdAsyncSession
   {
      DECLARE_OBJ_MSG_MAP()
   public:
      _seAdptIndexSession( UINT64 sessionID, const seIndexMeta *idxMeta ) ;
      virtual ~_seAdptIndexSession() ;

      virtual EDU_TYPES eduType() const ;
      virtual SDB_SESSION_TYPE sessionType() const ;
      virtual const CHAR* className() const { return "Indexer" ; }

      INT32 handleQueryRes( NET_HANDLE handle, MsgHeader* msg ) ;
      INT32 handleGetMoreRes( NET_HANDLE handle, MsgHeader *msg ) ;
      INT32 handleKillCtxRes( NET_HANDLE handle, MsgHeader *msg ) ;

      // Called by net io thread.
      virtual void onRecieve( const NET_HANDLE netHandle, MsgHeader * msg ) ;
      virtual BOOLEAN timeout( UINT32 interval ) ;

      // Called by self thread, in the main loop of the thread.
      virtual void onTimer( UINT64 timerID, UINT32 interval ) ;
   protected:
      virtual void _onAttach() ;
      virtual void _onDetach() ;

   private:
      void  _updateCLVersion( INT32 version ) ;
      void  _switchStatus( SEADPT_SESSION_STATUS newStatus ) ;
      INT32 _sendGetmoreReq( INT64 contextID, UINT64 requestID ) ;
      INT32 _queryOrigCollection() ;
      INT32 _queryLastCappedRecLID( BOOLEAN reverse = FALSE ) ;
      INT32 _queryCappedCollection( BSONObj &condition ) ;
      INT32 _cleanData( INT64 recLID ) ;
      INT32 _parseSrcData( const BSONObj &origObj, _rtnExtOprType &oprType,
                           const CHAR **origOID, INT64 &logicalID,
                           BSONObj &sourceObj ) ;
      INT32 _processNormalCLRecords( NET_HANDLE handle, MsgHeader *msg ) ;

      INT32 _processCappedCLRecords( NET_HANDLE handle, MsgHeader *msg ) ;
      INT32 _getLastIndexedLID( NET_HANDLE handle, MsgHeader *msg ) ;
      INT32 _markProgress( BSONObj &infoObj ) ;
      INT32 _updateProgress( INT64 logicalID ) ;

      // Check if the mark of normal collection end has been written in ES.
      INT32 _chkDoneMark( BOOLEAN &found ) ;
      INT32 _consult() ;
      INT32 _onSDBEOC() ;
      INT32 _startOver() ;
      void  _setQueryBusyFlag( BOOLEAN busy ) { _queryBusy = busy; }
      BOOLEAN _isQueryBusy() { return _queryBusy ; }
      INT32 _bulkPrepare() ;
      INT32 _bulkProcess( const utilESBulkActionBase &actionItem ) ;
      INT32 _bulkFinish() ;
      INT32 _processBigItem( const utilESBulkActionBase &actionItem ) ;
      INT32 _createIndex( BOOLEAN force = FALSE ) ;
      INT32 _dropIndex() ;

      OSS_INLINE INT32 _findRecWithLID( INT64 logicalID, BOOLEAN &found ) ;

   private:
      string                  _origCLFullName ;
      string                  _cappedCLFullName ;
      INT32                   _origCLVersion ;
      string                  _origIdxName ;
      string                  _indexName ;
      string                  _typeName ;
      BSONObj                 _indexDef ;
      BSONObj                 _queryCond ;
      BSONObj                 _selector ; // Should contain _id and index fields.
      utilESClt               *_esClt ;
      SEADPT_SESSION_STATUS   _status ;
      INT64                   _lastPopLID ;
      // Whether the session should quit. If the targe collection dosn't exist
      // any more, it means the index has been dropped. In this case, this
      // session should quit.
      BOOLEAN                 _quit ;
      INT64                   _queryCtxID ;
      BOOLEAN                 _queryBusy ;   // Where one query is in progress.
                                             // A new query can be started only
                                             // when it's FALSE.
      INT64                   _expectLID ;
      utilESBulkBuilder       _bulkBuilder ;
   } ;
   typedef _seAdptIndexSession seAdptIndexSession ;

   OSS_INLINE INT32 _seAdptIndexSession::_findRecWithLID( INT64 logicalID,
                                                          BOOLEAN &found )
   {
      INT32 rc = SDB_OK ;
      std::ostringstream lidStr ;
      lidStr << logicalID ;

      rc = _esClt->documentExist( _indexName.c_str(), _typeName.c_str(),
                                  SEADPT_FIELD_NAME_ID, lidStr.str().c_str(),
                                  found ) ;
      PD_RC_CHECK( rc, PDERROR, "Document existence check failed[ %d ]", rc ) ;
   done:
      return rc ;
   error:
      goto done ;
   }
}

#endif /* SEADPT_INDEX_SESSION_HPP_ */

