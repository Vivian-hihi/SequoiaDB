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

   Source File Name = seAdptIndexState.hpp

   Descriptive Name = Index session state.

   When/how to use: this program may be used on binary and text-formatted
   versions of PMD component. This file contains main function for sdbcm,
   which is used to do cluster managing.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          10/24/2018  YSD  Initial Draft

   Last Changed =

*******************************************************************************/
#ifndef SEADPT_INDEX_STATE_HPP__
#define SEADPT_INDEX_STATE_HPP__

#include "netDef.hpp"
#include "msg.hpp"
#include "rtnExtOprDef.hpp"
#include "../bson/bson.hpp"

using engine::NET_HANDLE ;
using engine::_rtnExtOprType ;
using bson::BSONObj ;

namespace seadapter
{
   class _seAdptIndexSession ;

   /*
    * Note: Any change of this enum, please change the function
    * seAdptGetIndexerStateDesp at the same time.
    */
   enum SEADPT_INDEXER_STATE
   {
      CONSULT = 1,      // Consult, find where to start.
      FULL_INDEX,       // Full indexing, fetch data from the original cl.
      INCREMENT_INDEX   // Increment indexing, fetch data from capped cl.
   } ;

   const CHAR* seAdptGetIndexerStateDesp( SEADPT_INDEXER_STATE state ) ;

   //  Indexer state interface, defining processing of all the states.
   class _seAdptIndexerState : public SDBObject
   {
   public:
      _seAdptIndexerState() ;
      virtual ~_seAdptIndexerState() {}

      virtual SEADPT_INDEXER_STATE type() const = 0 ;

      // onTimer() will be called by the async session framework if no new
      // message received within more than one second.
      virtual INT32 onTimer( _seAdptIndexSession *session,
                             UINT32 interval ) = 0 ;

      // Process the reply of query from db.
      virtual INT32 processQueryRes( _seAdptIndexSession *session,
                                     NET_HANDLE handle, MsgHeader *msg ) = 0 ;

      // Process the reply of getMore from db.
      virtual INT32 processGetMoreRes( _seAdptIndexSession *session,
                                       NET_HANDLE handle, MsgHeader *msg ) = 0 ;

      /**
       * @brief Update progress document on search engine(Record with _id of
       * SDBCOMMIT).
       */
      INT32 updateProgress( _seAdptIndexSession *session,
                            BOOLEAN initial = FALSE ) ;

   protected:
      INT32 _getProgressInfo( _seAdptIndexSession *session,
                              BSONObj &progressInfo ) ;

      INT32 _validateProgress( _seAdptIndexSession *session,
                               const BSONObj &progressInfo,
                               BOOLEAN &valid ) ;

      INT32 _cleanSearchEngine( _seAdptIndexSession *session ) ;

   protected:
      UINT32 _timeout ;
      UINT16 _retryTimes ;
   } ;
   typedef _seAdptIndexerState seAdptIndexerState ;


   // Concrete state definitions.

   /**
    * Consultation state is the first state when an indexer starts.
    * Its main task is to check the indexing progress of the target text index,
    * and decide what to do next, full indexing or incremental indexing.
    */
   class _seAdptConsultState : public seAdptIndexerState
   {
   public:
      _seAdptConsultState() ;
      ~_seAdptConsultState() ;

      SEADPT_INDEXER_STATE type() const ;

      INT32 processQueryRes( _seAdptIndexSession *session,
                             NET_HANDLE handle, MsgHeader *msg ) ;
      INT32 processGetMoreRes( _seAdptIndexSession *session,
                               NET_HANDLE handle, MsgHeader *msg ) ;

      INT32 onTimer( _seAdptIndexSession *session, UINT32 interval ) ;

   private:
      INT32 _consult( _seAdptIndexSession *session ) ;
      INT32 _queryMinIDOnDB( _seAdptIndexSession *session ) ;
      BOOLEAN _progressMatch( INT64 minLID ) ;

   private:
      BOOLEAN _checkLID ;
      INT64 _expectLID ;
   } ;
   typedef _seAdptConsultState seAdptConsultState ;

   // When consultation state decides to do full indexing, it will be done by
   // this state. The steps are as follows:
   // 1. Create index on ES(drop at first if it exists).
   // 2. Pop all the data in the capped collection.
   // 3. Index all data in the original collection.
   // 4. Mark the progress in the SDBCOMMIT record.
   // Actions will be triggered by the timer, starting from cleaning ES.
   class _seAdptFullIndexState: public seAdptIndexerState
   {
      enum _STEP
      {
         CLEAN_ES,
         CRT_ES_IDX,
         CLEAN_DB_P1,
         CLEAN_DB_P2,
         QUERY_CL_VERSION,    // Version is required when query collection data.
         QUERY_DATA
      } ;
   public:
      _seAdptFullIndexState() ;
      ~_seAdptFullIndexState() ;

      SEADPT_INDEXER_STATE type() const ;

      /**
       * @brief Process query reply. In full indexing state, there are 3 kinds
       * of queries:
       * (1) Query version of original collection from catalogue node.
       * (2) Query data of original collection.
       * (3) Query data of capped collection.
       */
      INT32 processQueryRes( _seAdptIndexSession *session,
                             NET_HANDLE handle, MsgHeader *msg ) ;
      INT32 processGetMoreRes( _seAdptIndexSession *session,
                               NET_HANDLE handle, MsgHeader *msg ) ;

      INT32 onTimer( _seAdptIndexSession *session, UINT32 interval ) ;

   private:
      INT32 _crtESIdx( _seAdptIndexSession *session ) ;

      /**
       * @brief Clean the source capped collection before full indexing. Do that
       * with a pop operation backwards to the first record.
       */
      INT32 _prepareCleanDB( _seAdptIndexSession *session ) ;

      /**
       * @brief Send pop command to the data node.
       * @param session
       * @param targetObj The first record in the capped collection.
       */
      INT32 _doCleanDB( _seAdptIndexSession *session,
                        const BSONObj &targetObj ) ;

      INT32 _queryCLVersion( _seAdptIndexSession *session ) ;

      /**
       * @brief Send query request to the original collection.
       */
      INT32 _queryData( _seAdptIndexSession *session ) ;

      /**
       * @brief Generate options for querying the original collection.
       */
      INT32 _genQueryOptions( _seAdptIndexSession *session,
                              BSONObj &query, BSONObj &selector ) ;

      INT32 _handleCLVersionRes( _seAdptIndexSession *session,
                                 INT32 result, const BSONObj &record ) ;

      INT32 _handleQueryDataRes( _seAdptIndexSession *session, INT32 result,
                                 INT64 contextID ) ;

      INT32 _getMore( _seAdptIndexSession *session, INT64 contextID ) ;

      INT32 _parseRecord( const BSONObj &origRecord, string &finalID,
                          BSONObj &finalRecord ) ;

   private:
      _STEP _step ;
      INT32 _clVersion ;
   } ;
   typedef _seAdptFullIndexState seAdptFullIndexState ;

   // Incremental indexing steps are as follows:
   // 1. Get a batch of data from the capped collection.
   // 2. Parse and reformat these objects for indexing.
   // 3. Index these documents on ES.
   // 4. Pop the capped collection to the "last" done position.
   // 5. Repeate step 1~4.
   class _seAdptIncIndexState : public seAdptIndexerState
   {
      enum _STEP
      {
         QUERY_DATA,
         CLEAN_SRC
      } ;
   public:
      _seAdptIncIndexState() ;
      ~_seAdptIncIndexState() ;

      SEADPT_INDEXER_STATE type() const ;

      INT32 processQueryRes( _seAdptIndexSession *session,
                             NET_HANDLE handle, MsgHeader *msg ) ;
      INT32 processGetMoreRes( _seAdptIndexSession *session,
                               NET_HANDLE handle, MsgHeader *msg ) ;

      INT32 onTimer( _seAdptIndexSession *session, UINT32 interval ) ;

   private:
      INT32 _genQueryOptions( _seAdptIndexSession *session,
                              BSONObj &query, BSONObj &selector ) ;
      INT32 _queryData( _seAdptIndexSession *session ) ;
      INT32 _getMore( _seAdptIndexSession *session, INT64 contextID ) ;

      /**
       * @brief Process documents fetched from capped collection.
       * @param docs Documents to be processed.
       */
      INT32 _processDocuments( _seAdptIndexSession *session,
                               const vector<BSONObj> &docs ) ;

      /**
       * @brief Process one document.
       * @param logicalID Logical ID of the record.
       */
      INT32 _processDocument( _seAdptIndexSession *session,
                              const BSONObj &document, INT64 &logicalID ) ;

      /**
       * @brief Check if the first record is the one we expected. If not, it
       * means the incremental indexing has broken and not able to recover.
       * Index on search engine will be dropped. Full indexing will be started.
       * @param doc First record of data fetched in this round.
       */
      INT32 _consistencyCheck( _seAdptIndexSession *session,
                               const BSONObj &doc ) ;

      INT32 _parseRecord( const BSONObj &origObj, _rtnExtOprType &oprType,
                          string &finalID, INT64 &logicalID,
                          BSONObj &sourceObj, string *newFinalID ) ;

      INT32 _cleanData( _seAdptIndexSession *session ) ;

      BOOLEAN _typeSupport( INT32 type )
      {
         return ( type > engine::RTN_EXT_INVALID && type
                  < engine::RTN_EXT_DUMMY ) ;
      }

   private:
      _STEP _step ;

      // Whether new data has been fetched from data node.
      BOOLEAN _hasNewData ;
      BOOLEAN _firstBatchData ;
      UINT32 _expectRecHash ;
      INT64 _queryCtxID ;
   } ;
   typedef _seAdptIncIndexState seAdptIncIndexState ;
}

#endif /* SEADPT_INDEX_STATE_HPP__ */
