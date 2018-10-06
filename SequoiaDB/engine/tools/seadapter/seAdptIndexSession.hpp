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
#include "seAdptDef.hpp"
#include "seAdptMgr.hpp"
#include "utilESBulkBuilder.hpp"

using namespace bson ;

namespace seadapter
{
   enum SEADPT_SESSION_STATUS
   {
      SEADPT_SESSION_STAT_CONSULT = 1,       // To find where to start.
      SEADPT_SESSION_STAT_BEGIN,             // Start from the beginning,
                                             // query normal collection.
      SEADPT_SESSION_STAT_UPDATE_CL_VERSION, // Update collection version.
      SEADPT_SESSION_STAT_CLEAN_DATA,
      SEADPT_SESSION_STAT_COMP_LID,
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
      void  _switchStatus( SEADPT_SESSION_STATUS newStatus ) ;
      INT32 _sendGetmoreReq( INT64 contextID, UINT64 requestID ) ;
      INT32 _queryOrigCollection() ;
      INT32 _queryOneCappedRec() ;
      INT32 _truncateSrcCappedData() ;
      INT32 _queryCappedCollection( BSONObj &condition ) ;
      INT32 _cleanData( INT64 recLID ) ;
      INT32 _parseCappedRecord( const BSONObj &origObj, _rtnExtOprType &oprType,
                                string &finalID, INT64 &logicalID,
                                BSONObj &sourceObj,
                                string *newFinalID = NULL ) ;
      INT32 _parseNormalRecord( const BSONObj &origRecord, string &finalID,
                                BSONObj &finalRecord ) ;
      INT32 _processNormalCLRecords( NET_HANDLE handle, MsgHeader *msg ) ;

      INT32 _processCappedCLRecords( NET_HANDLE handle, MsgHeader *msg ) ;
      INT32 _getLastIndexedLID( NET_HANDLE handle, MsgHeader *msg ) ;
      INT32 _markProgress( BSONObj &infoObj ) ;
      INT32 _updateProgress( INT64 logicalID ) ;

      // Check if the mark of normal collection end has been written in ES.
      INT32 _validate( const BSONObj &obj, BOOLEAN &valid ) ;
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
      INT32 _index( const string &id, const BSONObj &document ) ;
      INT32 _delete( const string &id ) ;
      INT32 _replace( const string &id, const string &newId,
                      const BSONObj &document ) ;

      BOOLEAN _typeSupport( INT32 type ) ;

   private:
      INT32                   _origCLVersion ;
      BSONObj                 _queryCond ;
      BSONObj                 _selector ; // Should contain _id and index fields.
      seIndexMeta             _meta ;
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
      UINT32                  _expectRecHash ;
      BOOLEAN                 _emptyResultSet ;
      utilESBulkBuilder       _bulkBuilder ;
   } ;
   typedef _seAdptIndexSession seAdptIndexSession ;
}

#endif /* SEADPT_INDEX_SESSION_HPP_ */

