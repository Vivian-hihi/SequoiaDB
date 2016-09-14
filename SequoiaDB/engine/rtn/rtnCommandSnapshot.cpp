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

   Source File Name = rtnCommandSnapshot.cpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          23/06/2016  Xu Jianhui  Initial Draft

   Last Changed =

*******************************************************************************/

#include "rtnCommandSnapshot.hpp"
#include "pd.hpp"
#include "pmdEDU.hpp"
#include "rtn.hpp"
#include "dms.hpp"
#include "pmd.hpp"
#include "monDump.hpp"
#include "aggrDef.hpp"
#include "pdTrace.hpp"
#include "rtnTrace.hpp"

using namespace bson ;
using namespace std ;

namespace engine
{

   _rtnSnapshotInner::_rtnSnapshotInner()
   :_matcherBuff ( NULL ), _selectBuff ( NULL ), _orderByBuff ( NULL ),
    _hintBuff( NULL )
   {
      _flags = 0 ;
      _numToReturn = -1 ;
      _numToSkip = 0 ;
   }

   _rtnSnapshotInner::~_rtnSnapshotInner()
   {
   }

   INT32 _rtnSnapshotInner::init ( INT32 flags, INT64 numToSkip,
                                   INT64 numToReturn,
                                   const CHAR * pMatcherBuff,
                                   const CHAR * pSelectBuff,
                                   const CHAR * pOrderByBuff,
                                   const CHAR * pHintBuff )
   {
      _flags = flags ;
      _numToReturn = numToReturn ;
      _numToSkip = numToSkip ;
      _matcherBuff = pMatcherBuff ;
      _selectBuff = pSelectBuff ;
      _orderByBuff = pOrderByBuff ;
      _hintBuff = pHintBuff ;

      return SDB_OK ;
   }

   UINT32 _rtnSnapshotInner::_addInfoMask() const
   {
      return MON_MASK_FETCH_DEFAULT ;
   }

   BSONObj _rtnSnapshotInner::_getOptObj() const
   {
      return BSONObj() ;
   }

   INT32 _rtnSnapshotInner::_createFetch( _pmdEDUCB *cb,
                                          rtnFetchBase **ppFetch )
   {
      INT32 rc = SDB_OK ;
      _rtnFetchBuilder *pFetchBuilder = getRtnFetchBuilder() ;
      rtnFetchBase *pNewFetch = NULL ;

      pNewFetch = pFetchBuilder->create( _getFetchType() ) ;
      if ( !pNewFetch )
      {
         rc = SDB_OOM ;
         PD_LOG( PDERROR, "Failed to alloc fetch[type:%d]",
                 _getFetchType() ) ;
         goto error ;
      }

      /// init
      rc = pNewFetch->init( cb, _isCurrent(), TRUE,
                            _addInfoMask(), _getOptObj() ) ;
      if ( rc )
      {
         PD_LOG( PDERROR, "Failed to init fetch, rc: %d", rc ) ;
         goto error ;
      }

      *ppFetch = pNewFetch ;
      pNewFetch = NULL ;

   done:
      if ( pNewFetch )
      {
         pFetchBuilder->release( pNewFetch ) ;
      }
      return rc ;
   error:
      goto done ;
   }

   INT32 _rtnSnapshotInner::doit( _pmdEDUCB *cb, _SDB_DMSCB *dmsCB,
                                  _SDB_RTNCB *rtnCB, _dpsLogWrapper *dpsCB,
                                  INT16 w, INT64 *pContextID )
   {
      INT32 rc = SDB_OK ;
      SDB_ASSERT ( cb, "educb can't be NULL" ) ;
      SDB_ASSERT ( pContextID, "context id can't be NULL" ) ;

      rtnContextDump *context = NULL ;
      rtnFetchBase *pFetch = NULL ;

      BSONObj matcher ;
      BSONObj selector ;
      BSONObj orderBy ;
      BSONObj hint ;

      try
      {
         BSONObj tmpMatcher ( _matcherBuff ) ;
         BSONObj nodeMatcher ;
         selector = BSONObj( _selectBuff ) ;
         orderBy = BSONObj( _orderByBuff ) ;
         hint = BSONObj( _hintBuff ) ;

         rc = parseMatcher( tmpMatcher, nodeMatcher, matcher, TRUE ) ;
         PD_RC_CHECK( rc, PDERROR, "Parse matcher failed, rc: %d", rc ) ;
      }
      catch( std::exception &e )
      {
         PD_LOG( PDERROR, "Parse param occur exception: %s", e.what() ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }

      // create cursors
      rc = rtnCB->contextNew ( RTN_CONTEXT_DUMP, (rtnContext**)&context,
                               *pContextID, cb ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to create new context, rc: %d", rc ) ;
         goto error ;
      }

      rc = context->open( selector,
                          matcher,
                          orderBy.isEmpty() ? _numToReturn : -1,
                          orderBy.isEmpty() ? _numToSkip : 0 ) ;
      PD_RC_CHECK( rc, PDERROR, "Open context failed, rc: %d", rc ) ;

      // sample timetamp
      if ( cb->getMonConfigCB()->timestampON )
      {
         context->getMonCB()->recordStartTimestamp() ;
      }

      /// create and init fetch
      rc = _createFetch( cb, &pFetch ) ;
      PD_RC_CHECK( rc, PDERROR, "Failed to create fetch, rc: %d", rc ) ;

      context->setMonFetch( pFetch, TRUE ) ;
      pFetch = NULL ;

      /// when has orderby
      if ( !orderBy.isEmpty() )
      {
         rc = rtnSort( (rtnContext**)&context, orderBy, cb,
                       _numToSkip, _numToReturn, *pContextID ) ;
         PD_RC_CHECK( rc, PDERROR, "Failed to sort, rc: %d", rc ) ;
      }

   done:
      return rc ;
   error:
      if ( -1 != *pContextID )
      {
         rtnCB->contextDelete( *pContextID, cb ) ;
         *pContextID = -1 ;
         context = NULL ;
      }
      if ( pFetch )
      {
         SDB_OSS_DEL pFetch ;
         pFetch = NULL ;
      }
      goto done ;
   }

   _rtnSnapshot::_rtnSnapshot ()
   {
   }

   _rtnSnapshot::~_rtnSnapshot ()
   {
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__RTNSNAPSHOT_DOIT, "_rtnSnapshot::doit" )
   INT32 _rtnSnapshot::doit( _pmdEDUCB *cb, SDB_DMSCB *dmsCB,
                             SDB_RTNCB *rtnCB, SDB_DPSCB *dpsCB,
                             INT16 w , INT64 *pContextID )
   {
      PD_TRACE_ENTRY ( SDB__RTNSNAPSHOT_DOIT ) ;
      SDB_ASSERT ( cb, "educb can't be NULL" ) ;
      SDB_ASSERT ( pContextID, "context id can't be NULL" ) ;

      INT32 rc = SDB_OK ;
      CHAR *pOutBuff = NULL ;
      INT32 buffSize = 0 ;
      INT32 buffUsedSize = 0 ;
      INT32 buffObjNum = 0 ;

      vector< BSONObj > vecUserAggr ;
      BSONObj matcher ;
      BSONObj selector ;
      BSONObj orderBy ;
      BSONObj hint ;

      try
      {
         hint = BSONObj( _hintBuff ) ;

         rc = parseUserAggr( hint, vecUserAggr ) ;
         PD_RC_CHECK( rc, PDERROR, "Parse user define aggr failed, rc: %d",
                      rc ) ;

         if ( vecUserAggr.size() > 0 )
         {
            BSONObj tmpMatcher ( _matcherBuff ) ;
            BSONObj nodeMatcher ;
            selector = BSONObj( _selectBuff ) ;
            orderBy = BSONObj( _orderByBuff ) ;

            rc = parseMatcher( tmpMatcher, nodeMatcher, matcher, TRUE ) ;
            PD_RC_CHECK( rc, PDERROR, "Parse matcher failed, rc: %d", rc ) ;
         }
      }
      catch( std::exception &e )
      {
         PD_LOG( PDERROR, "Parse param occur exception: %s", e.what() ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }

      if ( vecUserAggr.size() > 0 )
      {
         /// add new matcher
         if ( !matcher.isEmpty() )
         {
            rc = appendObj( BSON( AGGR_MATCH_PARSER_NAME << matcher ),
                            pOutBuff, buffSize, buffUsedSize, buffObjNum ) ;
            PD_RC_CHECK( rc, PDERROR, "Append new matcher failed, rc: %d",
                         rc ) ;
         }

         /// order by
         if ( orderBy.isEmpty() )
         {
            rc = appendObj( BSON( AGGR_SORT_PARSER_NAME << orderBy ),
                            pOutBuff, buffSize, buffUsedSize, buffObjNum ) ;
            PD_RC_CHECK( rc, PDERROR, "Append order by failed, rc: %d",
                         rc ) ;
         }

         for ( UINT32 i = 0 ; i < vecUserAggr.size() ; ++i )
         {
            rc = appendObj( vecUserAggr[ i ], pOutBuff, buffSize,
                            buffUsedSize, buffObjNum ) ;
            PD_RC_CHECK( rc, PDERROR, "Append user define aggr[%s] failed, "
                         "rc: %d", vecUserAggr[ i ].toString().c_str(),
                         rc ) ;
         }

         /// open context
         rc = openContext( pOutBuff, buffObjNum, getIntrCMDName(),
                           selector, cb, *pContextID ) ;
         PD_RC_CHECK( rc, PDERROR, "Open context failed, rc: %d", rc ) ;
      }
      else
      {
         rc = _rtnSnapshotInner::doit( cb, dmsCB, rtnCB,
                                       dpsCB, w, pContextID ) ;
         if ( rc )
         {
            goto error ;
         }
      }

   done:
      if ( pOutBuff )
      {
         SDB_OSS_FREE( pOutBuff ) ;
         pOutBuff = NULL ;
         buffSize = 0 ;
         buffUsedSize = 0 ;
      }
      PD_TRACE_EXITRC ( SDB__RTNSNAPSHOT_DOIT, rc ) ;
      return rc ;
   error:
      if ( -1 != *pContextID )
      {
         rtnCB->contextDelete ( *pContextID, cb ) ;
         *pContextID = -1 ;
      }
      goto done ;
   }

   IMPLEMENT_CMD_AUTO_REGISTER(_rtnSnapshotSystem)
   _rtnSnapshotSystem::_rtnSnapshotSystem ()
   {
   }

   _rtnSnapshotSystem::~_rtnSnapshotSystem ()
   {
   }

   const CHAR *_rtnSnapshotSystem::name ()
   {
      return NAME_SNAPSHOT_SYSTEM ;
   }

   RTN_COMMAND_TYPE _rtnSnapshotSystem::type ()
   {
      return CMD_SNAPSHOT_SYSTEM ;
   }

   INT32 _rtnSnapshotSystem::_getFetchType() const
   {
      return RTN_FETCH_SYSTEM ;
   }

   BOOLEAN _rtnSnapshotSystem::_isCurrent() const
   {
      return FALSE ;
   }

   UINT32 _rtnSnapshotSystem::_addInfoMask() const
   {
      return MON_MASK_ALL ;
   }

   const CHAR* _rtnSnapshotSystem::getIntrCMDName()
   {
      return CMD_NAME_SNAPSHOT_SYSTEM_INTR ;
   }

   IMPLEMENT_CMD_AUTO_REGISTER(_rtnSnapshotSystemInner)
   const CHAR *_rtnSnapshotSystemInner::name ()
   {
      return CMD_NAME_SNAPSHOT_SYSTEM_INTR ;
   }

   RTN_COMMAND_TYPE _rtnSnapshotSystemInner::type ()
   {
      return CMD_SNAPSHOT_SYSTEM ;
   }

   INT32 _rtnSnapshotSystemInner::_getFetchType() const
   {
      return RTN_FETCH_SYSTEM ;
   }

   BOOLEAN _rtnSnapshotSystemInner::_isCurrent() const
   {
      return FALSE ;
   }

   UINT32 _rtnSnapshotSystemInner::_addInfoMask() const
   {
      return MON_MASK_ALL ;
   }

   IMPLEMENT_CMD_AUTO_REGISTER(_rtnSnapshotContexts)
   _rtnSnapshotContexts::_rtnSnapshotContexts ()
   {
   }

   _rtnSnapshotContexts::~_rtnSnapshotContexts ()
   {
   }

   const CHAR *_rtnSnapshotContexts::name ()
   {
      return NAME_SNAPSHOT_CONTEXTS ;
   }

   RTN_COMMAND_TYPE _rtnSnapshotContexts::type ()
   {
      return CMD_SNAPSHOT_CONTEXTS ;
   }

   INT32 _rtnSnapshotContexts::_getFetchType() const
   {
      return RTN_FETCH_CONTEXT ;
   }

   BOOLEAN _rtnSnapshotContexts::_isCurrent() const
   {
      return FALSE ;
   }

   UINT32 _rtnSnapshotContexts::_addInfoMask() const
   {
      return MON_MASK_NODE_NAME ;
   }

   const CHAR* _rtnSnapshotContexts::getIntrCMDName()
   {
      return CMD_NAME_SNAPSHOT_CONTEX_INTR ;
   }

   IMPLEMENT_CMD_AUTO_REGISTER(_rtnSnapshotContextsInner)
   const CHAR *_rtnSnapshotContextsInner::name ()
   {
      return CMD_NAME_SNAPSHOT_CONTEX_INTR ;
   }

   RTN_COMMAND_TYPE _rtnSnapshotContextsInner::type ()
   {
      return CMD_SNAPSHOT_CONTEXTS ;
   }

   INT32 _rtnSnapshotContextsInner::_getFetchType() const
   {
      return RTN_FETCH_CONTEXT ;
   }

   BOOLEAN _rtnSnapshotContextsInner::_isCurrent() const
   {
      return FALSE ;
   }

   UINT32 _rtnSnapshotContextsInner::_addInfoMask() const
   {
      return MON_MASK_NODE_NAME ;
   }

   IMPLEMENT_CMD_AUTO_REGISTER(_rtnSnapshotContextsCurrent)
   _rtnSnapshotContextsCurrent::_rtnSnapshotContextsCurrent ()
   {
   }

   _rtnSnapshotContextsCurrent::~_rtnSnapshotContextsCurrent ()
   {
   }

   const CHAR *_rtnSnapshotContextsCurrent::name ()
   {
      return NAME_SNAPSHOT_CONTEXTS_CURRENT ;
   }

   RTN_COMMAND_TYPE _rtnSnapshotContextsCurrent::type ()
   {
      return CMD_SNAPSHOT_CONTEXTS_CURRENT ;
   }

   INT32 _rtnSnapshotContextsCurrent::_getFetchType() const
   {
      return RTN_FETCH_CONTEXT ;
   }

   BOOLEAN _rtnSnapshotContextsCurrent::_isCurrent() const
   {
      return TRUE ;
   }

   UINT32 _rtnSnapshotContextsCurrent::_addInfoMask() const
   {
      return MON_MASK_NODE_NAME ;
   }

   const CHAR* _rtnSnapshotContextsCurrent::getIntrCMDName()
   {
      return CMD_NAME_SNAPSHOT_CONTEXCUR_INTR ;
   }

   IMPLEMENT_CMD_AUTO_REGISTER(_rtnSnapshotContextsCurrentInner)
   const CHAR *_rtnSnapshotContextsCurrentInner::name ()
   {
      return CMD_NAME_SNAPSHOT_CONTEXCUR_INTR ;
   }

   RTN_COMMAND_TYPE _rtnSnapshotContextsCurrentInner::type ()
   {
      return CMD_SNAPSHOT_CONTEXTS_CURRENT ;
   }

   INT32 _rtnSnapshotContextsCurrentInner::_getFetchType() const
   {
      return RTN_FETCH_CONTEXT ;
   }

   BOOLEAN _rtnSnapshotContextsCurrentInner::_isCurrent() const
   {
      return TRUE ;
   }

   UINT32 _rtnSnapshotContextsCurrentInner::_addInfoMask() const
   {
      return MON_MASK_NODE_NAME ;
   }

   IMPLEMENT_CMD_AUTO_REGISTER(_rtnSnapshotDatabase)
   _rtnSnapshotDatabase::_rtnSnapshotDatabase ()
   {
   }

   _rtnSnapshotDatabase::~_rtnSnapshotDatabase ()
   {
   }

   const CHAR *_rtnSnapshotDatabase::name ()
   {
      return NAME_SNAPSHOT_DATABASE ;
   }

   RTN_COMMAND_TYPE _rtnSnapshotDatabase::type ()
   {
      return CMD_SNAPSHOT_DATABASE ;
   }

   const CHAR* _rtnSnapshotDatabase::getIntrCMDName()
   {
      return CMD_NAME_SNAPSHOT_DATABASE_INTR ;
   }

   INT32 _rtnSnapshotDatabase::_getFetchType() const
   {
      return RTN_FETCH_DATABASE ;
   }

   BOOLEAN _rtnSnapshotDatabase::_isCurrent() const
   {
      return FALSE ;
   }

   UINT32 _rtnSnapshotDatabase::_addInfoMask() const
   {
      return MON_MASK_ALL ;
   }

   IMPLEMENT_CMD_AUTO_REGISTER(_rtnSnapshotDatabaseInner)
   const CHAR *_rtnSnapshotDatabaseInner::name ()
   {
      return CMD_NAME_SNAPSHOT_DATABASE_INTR ;
   }

   RTN_COMMAND_TYPE _rtnSnapshotDatabaseInner::type ()
   {
      return CMD_SNAPSHOT_DATABASE ;
   }

   INT32 _rtnSnapshotDatabaseInner::_getFetchType() const
   {
      return RTN_FETCH_DATABASE ;
   }

   BOOLEAN _rtnSnapshotDatabaseInner::_isCurrent() const
   {
      return FALSE ;
   }

   UINT32 _rtnSnapshotDatabaseInner::_addInfoMask() const
   {
      return MON_MASK_ALL ;
   }

   IMPLEMENT_CMD_AUTO_REGISTER(_rtnSnapshotReset)
   _rtnSnapshotReset::_rtnSnapshotReset ()
   {
   }

   _rtnSnapshotReset::~_rtnSnapshotReset ()
   {
   }

   const CHAR *_rtnSnapshotReset::name ()
   {
      return NAME_SNAPSHOT_RESET ;
   }

   RTN_COMMAND_TYPE _rtnSnapshotReset::type ()
   {
      return CMD_SNAPSHOT_RESET ;
   }

   INT32 _rtnSnapshotReset::init( INT32 flags, INT64 numToSkip,
                                  INT64 numToReturn,
                                  const CHAR *pMatcherBuff,
                                  const CHAR *pSelectBuff,
                                  const CHAR *pOrderByBuff,
                                  const CHAR *pHintBuff )
   {
      return SDB_OK ;
   }

   INT32 _rtnSnapshotReset::doit( _pmdEDUCB *cb, _SDB_DMSCB *dmsCB,
                                  _SDB_RTNCB *rtnCB, _dpsLogWrapper *dpsCB,
                                  INT16 w, INT64 *pContextID )
   {
      monResetMon() ;
      return SDB_OK ;
   }

   IMPLEMENT_CMD_AUTO_REGISTER(_rtnSnapshotSessions)
   _rtnSnapshotSessions::_rtnSnapshotSessions ()
   {
   }

   _rtnSnapshotSessions::~_rtnSnapshotSessions ()
   {
   }

   const CHAR *_rtnSnapshotSessions::name ()
   {
      return NAME_SNAPSHOT_SESSIONS ;
   }

   RTN_COMMAND_TYPE _rtnSnapshotSessions::type ()
   {
      return CMD_SNAPSHOT_SESSIONS ;
   }

   INT32 _rtnSnapshotSessions::_getFetchType() const
   {
      return RTN_FETCH_SESSION ;
   }

   BOOLEAN _rtnSnapshotSessions::_isCurrent() const
   {
      return FALSE ;
   }

   UINT32 _rtnSnapshotSessions::_addInfoMask() const
   {
      return MON_MASK_NODE_NAME ;
   }

   const CHAR* _rtnSnapshotSessions::getIntrCMDName()
   {
      return CMD_NAME_SNAPSHOT_SESSION_INTR ;
   }

   IMPLEMENT_CMD_AUTO_REGISTER(_rtnSnapshotSessionsInner)
   const CHAR *_rtnSnapshotSessionsInner::name ()
   {
      return CMD_NAME_SNAPSHOT_SESSION_INTR ;
   }
   RTN_COMMAND_TYPE _rtnSnapshotSessionsInner::type ()
   {
      return CMD_SNAPSHOT_SESSIONS ;
   }

   INT32 _rtnSnapshotSessionsInner::_getFetchType() const
   {
      return RTN_FETCH_SESSION ;
   }

   BOOLEAN _rtnSnapshotSessionsInner::_isCurrent() const
   {
      return FALSE ;
   }

   UINT32 _rtnSnapshotSessionsInner::_addInfoMask() const
   {
      return MON_MASK_NODE_NAME ;
   }

   IMPLEMENT_CMD_AUTO_REGISTER(_rtnSnapshotSessionsCurrent)
   _rtnSnapshotSessionsCurrent::_rtnSnapshotSessionsCurrent ()
   {
   }

   _rtnSnapshotSessionsCurrent::~_rtnSnapshotSessionsCurrent ()
   {
   }

   const CHAR *_rtnSnapshotSessionsCurrent::name ()
   {
      return NAME_SNAPSHOT_SESSIONS_CURRENT ;
   }

   RTN_COMMAND_TYPE _rtnSnapshotSessionsCurrent::type ()
   {
      return CMD_SNAPSHOT_SESSIONS_CURRENT ;
   }

   INT32 _rtnSnapshotSessionsCurrent::_getFetchType() const
   {
      return RTN_FETCH_SESSION ;
   }

   BOOLEAN _rtnSnapshotSessionsCurrent::_isCurrent() const
   {
      return TRUE ;
   }

   UINT32 _rtnSnapshotSessionsCurrent::_addInfoMask() const
   {
      return MON_MASK_NODE_NAME ;
   }

   const CHAR* _rtnSnapshotSessionsCurrent::getIntrCMDName()
   {
      return CMD_NAME_SNAPSHOT_SESSIONCUR_INTR ;
   }

   IMPLEMENT_CMD_AUTO_REGISTER(_rtnSnapshotSessionsCurrentInner)
   const CHAR *_rtnSnapshotSessionsCurrentInner::name ()
   {
      return CMD_NAME_SNAPSHOT_SESSIONCUR_INTR ;
   }
   RTN_COMMAND_TYPE _rtnSnapshotSessionsCurrentInner::type ()
   {
      return CMD_SNAPSHOT_SESSIONS_CURRENT ;
   }

   INT32 _rtnSnapshotSessionsCurrentInner::_getFetchType() const
   {
      return RTN_FETCH_SESSION ;
   }

   BOOLEAN _rtnSnapshotSessionsCurrentInner::_isCurrent() const
   {
      return TRUE ;
   }

   UINT32 _rtnSnapshotSessionsCurrentInner::_addInfoMask() const
   {
      return MON_MASK_NODE_NAME ;
   }

   IMPLEMENT_CMD_AUTO_REGISTER(_rtnSnapshotTransactionsCurrent)
   _rtnSnapshotTransactionsCurrent::_rtnSnapshotTransactionsCurrent()
   {
   }

   _rtnSnapshotTransactionsCurrent::~_rtnSnapshotTransactionsCurrent()
   {
   }

   const CHAR *_rtnSnapshotTransactionsCurrent::name ()
   {
      return NAME_SNAPSHOT_TRANSACTIONS_CUR ;
   }

   RTN_COMMAND_TYPE _rtnSnapshotTransactionsCurrent::type ()
   {
      return CMD_SNAPSHOT_TRANSACTIONS_CUR ;
   }

   INT32 _rtnSnapshotTransactionsCurrent::_getFetchType() const
   {
      return RTN_FETCH_TRANS ;
   }

   BOOLEAN _rtnSnapshotTransactionsCurrent::_isCurrent() const
   {
      return TRUE ;
   }

   UINT32 _rtnSnapshotTransactionsCurrent::_addInfoMask() const
   {
      return MON_MASK_NODE_NAME ;
   }

   const CHAR* _rtnSnapshotTransactionsCurrent::getIntrCMDName()
   {
      return CMD_NAME_SNAPSHOT_TRANSCUR_INTR ;
   }

   IMPLEMENT_CMD_AUTO_REGISTER(_rtnSnapshotTransactionsCurrentInner)
   const CHAR *_rtnSnapshotTransactionsCurrentInner::name ()
   {
      return CMD_NAME_SNAPSHOT_TRANSCUR_INTR ;
   }
   RTN_COMMAND_TYPE _rtnSnapshotTransactionsCurrentInner::type ()
   {
      return CMD_SNAPSHOT_TRANSACTIONS_CUR ;
   }

   INT32 _rtnSnapshotTransactionsCurrentInner::_getFetchType() const
   {
      return RTN_FETCH_TRANS ;
   }

   BOOLEAN _rtnSnapshotTransactionsCurrentInner::_isCurrent() const
   {
      return TRUE ;
   }

   UINT32 _rtnSnapshotTransactionsCurrentInner::_addInfoMask() const
   {
      return MON_MASK_NODE_NAME ;
   }

   IMPLEMENT_CMD_AUTO_REGISTER(_rtnSnapshotTransactions)
   _rtnSnapshotTransactions::_rtnSnapshotTransactions()
   {
   }

   _rtnSnapshotTransactions::~_rtnSnapshotTransactions()
   {
   }

   const CHAR *_rtnSnapshotTransactions::name ()
   {
      return NAME_SNAPSHOT_TRANSACTIONS ;
   }

   RTN_COMMAND_TYPE _rtnSnapshotTransactions::type ()
   {
      return CMD_SNAPSHOT_TRANSACTIONS ;
   }

   INT32 _rtnSnapshotTransactions::_getFetchType() const
   {
      return RTN_FETCH_TRANS ;
   }

   BOOLEAN _rtnSnapshotTransactions::_isCurrent() const
   {
      return FALSE ;
   }

   UINT32 _rtnSnapshotTransactions::_addInfoMask() const
   {
      return MON_MASK_NODE_NAME ;
   }

   const CHAR* _rtnSnapshotTransactions::getIntrCMDName()
   {
      return CMD_NAME_SNAPSHOT_TRANS_INTR ;
   }

   IMPLEMENT_CMD_AUTO_REGISTER(_rtnSnapshotTransactionsInner)
   const CHAR *_rtnSnapshotTransactionsInner::name ()
   {
      return CMD_NAME_SNAPSHOT_TRANS_INTR ;
   }
   RTN_COMMAND_TYPE _rtnSnapshotTransactionsInner::type ()
   {
      return CMD_SNAPSHOT_TRANSACTIONS ;
   }

   INT32 _rtnSnapshotTransactionsInner::_getFetchType() const
   {
      return RTN_FETCH_TRANS ;
   }

   BOOLEAN _rtnSnapshotTransactionsInner::_isCurrent() const
   {
      return FALSE ;
   }

   UINT32 _rtnSnapshotTransactionsInner::_addInfoMask() const
   {
      return MON_MASK_NODE_NAME ;
   }

   IMPLEMENT_CMD_AUTO_REGISTER(_rtnSnapshotCollections)
   _rtnSnapshotCollections::_rtnSnapshotCollections ()
   {
   }

   _rtnSnapshotCollections::~_rtnSnapshotCollections ()
   {
   }

   const CHAR *_rtnSnapshotCollections::name ()
   {
      return NAME_SNAPSHOT_COLLECTIONS ;
   }

   RTN_COMMAND_TYPE _rtnSnapshotCollections::type ()
   {
      return CMD_SNAPSHOT_COLLECTIONS ;
   }

   INT32 _rtnSnapshotCollections::_getFetchType() const
   {
      return RTN_FETCH_COLLECTION ;
   }

   BOOLEAN _rtnSnapshotCollections::_isCurrent() const
   {
      /// for include system
      if ( CMD_SPACE_SERVICE_LOCAL == getFromService() )
      {
         return TRUE ;
      }
      return FALSE ;
   }

   UINT32 _rtnSnapshotCollections::_addInfoMask() const
   {
      return MON_MASK_NODE_NAME | MON_MASK_GROUP_NAME ;
   }

   const CHAR* _rtnSnapshotCollections::getIntrCMDName()
   {
      return CMD_NAME_SNAPSHOT_COLLECTION_INTR ;
   }

   IMPLEMENT_CMD_AUTO_REGISTER(_rtnSnapshotCollectionsInner)
   const CHAR *_rtnSnapshotCollectionsInner::name ()
   {
      return CMD_NAME_SNAPSHOT_COLLECTION_INTR ;
   }
   RTN_COMMAND_TYPE _rtnSnapshotCollectionsInner::type ()
   {
      return CMD_SNAPSHOT_COLLECTIONS ;
   }

   INT32 _rtnSnapshotCollectionsInner::_getFetchType() const
   {
      return RTN_FETCH_COLLECTION ;
   }

   BOOLEAN _rtnSnapshotCollectionsInner::_isCurrent() const
   {
      /// for include system
      if ( CMD_SPACE_SERVICE_LOCAL == getFromService() )
      {
         return TRUE ;
      }
      return FALSE ;
   }

   UINT32 _rtnSnapshotCollectionsInner::_addInfoMask() const
   {
      return MON_MASK_NODE_NAME | MON_MASK_GROUP_NAME ;
   }

   IMPLEMENT_CMD_AUTO_REGISTER(_rtnSnapshotCollectionSpaces)
   _rtnSnapshotCollectionSpaces::_rtnSnapshotCollectionSpaces ()
   {
   }

   _rtnSnapshotCollectionSpaces::~_rtnSnapshotCollectionSpaces ()
   {
   }

   const CHAR *_rtnSnapshotCollectionSpaces::name ()
   {
      return NAME_SNAPSHOT_COLLECTIONSPACES ;
   }

   RTN_COMMAND_TYPE _rtnSnapshotCollectionSpaces::type ()
   {
      return CMD_SNAPSHOT_COLLECTIONSPACES ;
   }

   INT32 _rtnSnapshotCollectionSpaces::_getFetchType() const
   {
      return RTN_FETCH_COLLECTIONSPACE ;
   }

   BOOLEAN _rtnSnapshotCollectionSpaces::_isCurrent() const
   {
      /// for include system
      if ( CMD_SPACE_SERVICE_LOCAL == getFromService() )
      {
         return TRUE ;
      }
      return FALSE ;
   }

   UINT32 _rtnSnapshotCollectionSpaces::_addInfoMask() const
   {
      return MON_MASK_GROUP_NAME ;
   }

   const CHAR* _rtnSnapshotCollectionSpaces::getIntrCMDName()
   {
      return CMD_NAME_SNAPSHOT_SPACE_INTR ;
   }

   IMPLEMENT_CMD_AUTO_REGISTER(_rtnSnapshotCollectionSpacesInner)
   const CHAR *_rtnSnapshotCollectionSpacesInner::name ()
   {
      return CMD_NAME_SNAPSHOT_SPACE_INTR ;
   }
   RTN_COMMAND_TYPE _rtnSnapshotCollectionSpacesInner::type ()
   {
      return CMD_SNAPSHOT_COLLECTIONSPACES ;
   }

   INT32 _rtnSnapshotCollectionSpacesInner::_getFetchType() const
   {
      return RTN_FETCH_COLLECTIONSPACE ;
   }

   BOOLEAN _rtnSnapshotCollectionSpacesInner::_isCurrent() const
   {
      /// for include system
      if ( CMD_SPACE_SERVICE_LOCAL == getFromService() )
      {
         return TRUE ;
      }
      return FALSE ;
   }

   UINT32 _rtnSnapshotCollectionSpacesInner::_addInfoMask() const
   {
      return MON_MASK_GROUP_NAME ;
   }

}


