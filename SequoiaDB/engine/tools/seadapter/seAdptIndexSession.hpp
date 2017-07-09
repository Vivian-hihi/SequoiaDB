#ifndef SEADPT_INDEX_SESSION_HPP_
#define SEADPT_INDEX_SESSION_HPP_

#include "utilESClt.hpp"
#include "pmdAsyncSession.hpp"
#include "dmsExtDataHandler.hpp"
#include "seAdptMgr.hpp"

using namespace bson ;

#define SEADPT_LOGICAL_ID            "_lid"

namespace engine
{
   enum SEADPT_SESSION_STATUS
   {
      SEADPT_SESSION_STAT_BEGIN = 1,
      SEADPT_SESSION_STAT_CONSULT,
      SEADPT_SESSION_STAT_CONSULT_CQ_RES,
      SEADPT_SESSION_STAT_QUERY_NORMAL_TBL,
      SEADPT_SESSION_STAT_WAIT_NQ_RES,
      SEADPT_SESSION_STAT_GETMORE_NORMAL,
      SEADPT_SESSION_STAT_QUERY_CAP_TBL,
      SEADPT_SESSION_STAT_WAIT_CQ_RES,
      SEADPT_SESSION_STAT_GETMORE_CAP,
      SEADPT_SESSION_STAT_POP_CAP
   } ;

   // Indexer sessions on search engine adapter.
   class _seAdptIndexSession : public _pmdAsyncSession
   {
      DECLARE_OBJ_MSG_MAP()
   public:
      _seAdptIndexSession( UINT64 sessionID, netRouteAgent *rtAgent,
                           const seIndexTask *task ) ;
      virtual ~_seAdptIndexSession() ;

      virtual EDU_TYPES eduType() const ;
      virtual SDB_SESSION_TYPE sessionType() const ;
      virtual const CHAR* className() const { return "Indexer" ; }

      INT32 handleQueryRes( NET_HANDLE handle, MsgHeader* msg ) ;
      INT32 handleGetMoreRes( NET_HANDLE handle, MsgHeader *msg ) ;

      // Called by net io thread.
      virtual void onRecieve( const NET_HANDLE netHandle, MsgHeader * msg ) ;
      virtual BOOLEAN timeout( UINT32 interval ) ;

      // Called by self thread, in the main loop of the thread.
      virtual void onTimer( UINT64 timerID, UINT32 interval ) ;
   protected:
      virtual void _onAttach() ;
      virtual void _onDetach() ;

   private:
      // Consule the current progress of the indexing.
      INT32 _progressConsult() ;
      void  _switchStatus( SEADPT_SESSION_STATUS newStatus ) ;

      INT32 _sendQueryReq() ;
      INT32 _sendQueryReq( BOOLEAN queryCappedCL ) ;
      INT32 _getExpectRLID( INT64 &expectRLID ) ;
      INT32 _cleanData( INT64 recLID ) ;
      INT32 _parseSrcData( const BSONObj &origObj, _dmsExtOprType &oprType,
                           OID &oid, BSONObj &sourceObj ) ;
      INT32 _processNormalCLRecords( NET_HANDLE handle, MsgHeader *msg ) ;

      INT32 _processCappedCLRecords( NET_HANDLE handle, MsgHeader *msg ) ;
      INT32 _getLastIndexedLID( NET_HANDLE handle, MsgHeader *msg ) ;
      INT32 _markNormalCLDone() ;

      // Check if the mark of normal collection end has been written in ES.
      INT32 _chkDoneMark( BOOLEAN &found ) ;
      INT32 _consult() ;
      INT32 _ensureESClt() ;
      void _onSDBEOC() ;

      void _begin() ;
      INT32 _getMoreSdbRecords( INT64 contextID, UINT64 requestID ) ;
      INT32 _onResMsg( NET_HANDLE handle, MsgHeader *msg ) ;

      OSS_INLINE INT32 _findRecWithLID( INT64 logicalID, BOOLEAN &found ) ;

   private:
      string                  _origCLFullName ;
      string                  _cappedCLFullName ;
      string                  _origIdxName ;
      string                  _indexName ;
      string                  _typeName ;
      BSONObj                 _indexDef ;
      BSONObj                 _selector ; // Should contain _id and index fields.
      utilESClt               *_esClt ;
      netRouteAgent           *_rtAgent ;
      SEADPT_SESSION_STATUS   _status ;
      INT64                   _startLID ;
      INT64                   _lastPopLID ;
      // Whether the session should quit. If the targe collection dosn't exist
      // any more, it means the index has been dropped. In this case, this
      // session should quit.
      BOOLEAN                 _quit ;
      UINT64                  _requestID ;
      BOOLEAN                 _indexCappedData ;
      BOOLEAN                 _indexInProgress ;
      ossRWMutex              _progressLatch ;
   } ;
   typedef _seAdptIndexSession seAdptIndexSession ;

   OSS_INLINE INT32 _seAdptIndexSession::_findRecWithLID( INT64 logicalID,
                                                          BOOLEAN &found )
   {
      INT32 rc = SDB_OK ;
      std::ostringstream lidStr ;
      lidStr << logicalID ;

      if ( !_esClt )
      {
         rc = sdbGetSeAdapterCB()->getSeCltMgr()->getSeClt( &_esClt ) ;
         if ( rc )
         {
            PD_LOG( PDERROR, "Failed to get search engine client, rc: %d",
                    rc ) ;
            goto error ;
         }
      }

      rc = _esClt->documentExist( _indexName.c_str(), _typeName.c_str(),
                                  SEADPT_LOGICAL_ID, lidStr.str().c_str(),
                                  found ) ;
      PD_RC_CHECK( rc, PDERROR, "Document existence check failed[ %d ]", rc ) ;
   done:
      return rc ;
   error:
      goto done ;
   }
}

#endif /* SEADPT_INDEX_SESSION_HPP_ */

