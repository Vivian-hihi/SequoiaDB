/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = qgmScan.cpp

   Descriptive Name =

   When/how to use: this program may be used on binary and text-formatted
   versions of PMD component. This file contains implementation for qgm
   collection or index scan

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          04/10/2013  TW  Initial Draft

   Last Changed =

*******************************************************************************/
#include "core.hpp"
#include "qgm.hpp"
#include "pd.hpp"
#include "pmd.hpp"
#include "rtn.hpp"
#include "msgMessage.hpp"

namespace engine
{
   _qgmOperatorScan::_qgmOperatorScan ( const CHAR *pAlias,
                                        const CHAR *pCollectionFullName,
                                        const BSONObj &condition,
                                        const BSONObj &selector,
                                        const BSONObj &orderBy,
                                        const BSONObj &hint,
                                        INT64 numSkip,
                                        INT64 numReturn )
   {
      INT32 rc          = SDB_OK ;
      pmdKRCB *krcb     = pmdGetKRCB () ;
      _hasExecuted      = FALSE ;
      _opType           = QGM_OPTYPE_SCAN ;
      _condition        = condition ;
      _selector         = selector ;
      _orderBy          = orderBy ;
      _hint             = hint ;
      _collectionName   = pCollectionFullName ;
      _numSkip          = numSkip ;
      _numReturn        = numReturn ;
      _currentSkip      = 0 ;
      _currentReturn    = 0 ;
      _finalCondition   = _condition ;
      _contextID        = -1 ;
      _dmsCB            = krcb->getDMSCB () ;
      _rtnCB            = krcb->getRTNCB () ;
      _dbRole           = krcb->getDBRole() ;
      _getMoreRequest   = NULL ;
      if ( SDB_ROLE_COORD == _dbRole )
      {
         INT32 rc = SDB_OK ;
         INT32 getMoreSize = 0 ;
         _pRouteAgent   = krcb->getCoordCB()->getRouteAgent() ;
         // memory free in destructor
         rc = msgBuildGetMoreMsg ( (CHAR**)&_getMoreRequest, &getMoreSize, 1,
                                   -1, 0 ) ;
         if ( rc )
         {
            PD_LOG ( PDERROR, "Failed to build get more message, rc = %d",
                     rc ) ;
            return ;
         }
         _getMoreRequest->contextID = -1 ;
      }
      else
      {
         _pRouteAgent   = NULL ;
      }
      _alias            = ossStrdup ( pAlias ) ;
      PD_CHECK ( _alias, SDB_OOM, error, PDERROR,
                 "Failed to allocate memory for alias" ) ;
      _isConstructed    = TRUE ;
   done :
      return ;
   error :
      goto done ;
   }

   _qgmOperatorScan::~_qgmOperatorScan ()
   {
      _killContext() ;
      if ( _getMoreRequest )
      {
         SDB_OSS_FREE ( _getMoreRequest ) ;
      }
   }

   void _qgmOperatorScan::close ()
   {
      _killContext() ;
   }

   INT32 _qgmOperatorScan::pushDownPredicates ( const BSONObj &pred )
   {
      INT32 rc = SDB_OK ;
      BSONObjBuilder ob ;
      // local predicate
      try
      {
         BSONObjIterator it ( _condition ) ;
         while ( it.more() )
         {
            BSONElement ele = it.next() ;
            ob.append ( ele ) ;
         }
      }
      catch ( std::exception &e )
      {
         PD_RC_CHECK ( SDB_SYS, PDERROR,
                       "Failed to extract from local predicates: %s: %s",
                       _condition.toString().c_str(),
                       e.what() ) ;
      }
      // join predicate
      try
      {
         BSONObjIterator it ( pred ) ;
         while ( it.more() )
         {
            BSONElement ele = it.next() ;
            ob.append ( ele ) ;
         }
      }
      catch ( std::exception &e )
      {
         PD_RC_CHECK ( SDB_SYS, PDERROR,
                       "Failed to extract from join predicates: %s: %s",
                       pred.toString().c_str(),
                       e.what() ) ;
      }
      _finalCondition = ob.obj() ;
   done :
      return rc ;
   error :
      goto done ;
   }
   INT32 _qgmOperatorScan::execute ( _pmdEDUCB *eduCB )
   {
      INT32 rc   = SDB_OK ;
      _eduCB     = eduCB ;
      CHAR *pMsg = NULL ;
      // get role for coord or data
      SDB_ASSERT ( _inputLegs.size() == 0, "no leg is expected" )
      // for scan type, basically we have to construct a buffer to simulate user
      // query request and then call rtn components. Because in coordinator it
      // simply bypass the request to the required partitions
      PD_CHECK ( _isConstructed, SDB_SYS, error, PDERROR,
                 "Scan was not properly constructed" ) ;
      _currentSkip = 0 ;
      _currentReturn = 0 ;
      if ( SDB_ROLE_COORD == _dbRole )
      {
         INT32 bufSize                    = 0 ;
         SDB_ASSERT ( _pRouteAgent, "route agent can't be NULL" )
         SDB_ASSERT ( _getMoreRequest, "getmore request can't be NULL" )
         UINT64 reqID                     = _pRouteAgent->reqIDNew();
         MsgOpReply dummyReply ;
         // memory is free by end of the function
         rc = msgBuildQueryMsg ( &pMsg, &bufSize, _collectionName, 0,
                                  reqID, _numSkip, _numReturn,
                                  &_finalCondition, &_selector,
                                  &_orderBy, &_hint ) ;
         PD_RC_CHECK ( rc, PDERROR,
                       "Failed to query on coord, rc = %d", rc ) ;
         rc = _coordQuery.execute ( pMsg, *(SINT32*)pMsg, NULL, _eduCB,
                                    dummyReply ) ;
         PD_RC_CHECK ( rc, PDERROR,
                       "Failed to execute coordQuery, rc = %d", rc ) ;
         // copy request and and the returned context id to getmore request
         _getMoreRequest->contextID = dummyReply.contextID ;
         _getMoreRequest->header.requestID = reqID ;

      }
      else
      {
         rc = rtnQuery ( _collectionName, _selector, _finalCondition,
                         _orderBy, _hint, 0, _eduCB, _numSkip, _numReturn,
                         _dmsCB, _rtnCB, _contextID ) ;
         PD_RC_CHECK ( rc, PDERROR,
                       "Failed to execute rtnQuery, rc = %d", rc ) ;
      }
      // inner side will be executed in fetchNext
      _hasExecuted = TRUE ;
   done :
      if ( pMsg )
      {
         SDB_OSS_FREE ( pMsg ) ;
      }
      return rc ;
   error :
      close () ;
      goto done ;
   }

   void _qgmOperatorScan::_killContext ()
   {
      INT32 rc = SDB_OK ;
      CHAR *pBuffer = NULL ;
      if ( SDB_ROLE_COORD == _dbRole )
      {
         SDB_ASSERT ( _getMoreRequest, "getmore request can't be NULL" )
         if ( -1 != _getMoreRequest->contextID )
         {
            MsgOpReply dummyReply ;
            CHAR *pBuffer = NULL ;
            INT32 bufferSize = NULL ;
            UINT64 reqID = _pRouteAgent->reqIDNew();
            rc = msgBuildKillContextsMsg ( &pBuffer, &bufferSize, reqID,
                                          1, &_getMoreRequest->contextID ) ;
            PD_RC_CHECK ( rc, PDERROR,
                          "Failed to allocate memory for kill context "
                          "requests, rc = %d", rc ) ;

            rc = _coordKillContext.execute ( pBuffer, *(SINT32*)pBuffer,
                                             NULL, _eduCB, dummyReply ) ;
            PD_RC_CHECK ( rc, PDERROR,
                          "Failed to kill contexts, rc = %d", rc ) ;
            _getMoreRequest->contextID = -1 ;
         }
      }
      else
      {
         if ( -1 != _contextID )
         {
            SDB_ASSERT ( _eduCB, "eduCB can't be NULL" )
            _eduCB->contextDelete ( _contextID ) ;
            _rtnCB->contextDelete ( _contextID ) ;
            _contextID = -1 ;
         }
      }
   done :
      if ( pBuffer )
      {
         SDB_OSS_FREE ( pBuffer ) ;
      }
      return ;
   error :
      goto done ;
   }

   // fetch next record, non-block operation
   INT32 _qgmOperatorScan::fetchNext ( BSONObj &obj )
   {
      INT32 rc = SDB_OK ;
      CHAR *pResultPointer = NULL ;
      PD_CHECK  ( _hasExecuted, SDB_SYS, error, PDERROR,
                  "Scan was not properly executed" ) ;
      if ( SDB_ROLE_COORD == _dbRole )
      {
         SDB_ASSERT ( _getMoreRequest, "getmore request can't be NULL" )
         MsgOpReply dummyReply ;
         rc = _coordGetMore.execute ( (CHAR*)_getMoreRequest,
                                      *(INT32*)_getMoreRequest,
                                      &pResultPointer, _eduCB, dummyReply ) ;
         PD_RC_CHECK ( rc, PDERROR, "Failed to getmore from coord, rc = %d",
                       rc ) ;
      }
      else
      {
         INT32 bufferLength ;
         INT32 numRecords ;
         INT64 startingPos ;
         SDB_ASSERT ( _contextID != -1,
                      "context id must be initialized" )
         rc = rtnGetMore ( _contextID, 1, &pResultPointer,
                           bufferLength, numRecords, startingPos,
                           _eduCB, _rtnCB ) ;
         if ( rc )
         {
            PD_CHECK ( SDB_DMS_EOC == rc, rc, error, PDERROR,
                       "Failed to getmore from non-coord, rc = %d", rc ) ;
            goto error ;
         }
      }
      try
      {
         BSONObj tempObj ( pResultPointer ) ;
         obj = tempObj ;
      }
      catch ( std::exception &e )
      {
         PD_RC_CHECK ( SDB_SYS, PDERROR,
                       "Exception when fetching from coord: %s",
                       e.what() ) ;
      }
   done :
      return rc ;
   error :
      // if anything goes wrong here, let's simply mark _contextID to -1, since
      // the real context should be closed in engine already
      _contextID = -1 ;
      if ( _getMoreRequest )
         _getMoreRequest->contextID = -1 ;
      goto done ;
   }

   INT32 _qgmOperatorScan::fromBson ( const BSONObj &plan,
                                      _qgmOperatorBase **output )
   {
      INT32 rc = SDB_OK ;
      BSONObj dummy ;
      SDB_ASSERT ( output, "output can't be NULL" )
      *output = NULL ;
      // required for sanity check
      BSONElement eleType       = plan.getField ( FIELD_NAME_TYPE ) ;
      // required for collection name
      BSONElement eleCollection = plan.getField ( FIELD_NAME_COLLECTION ) ;
      // required for alias
      BSONElement eleAlias      = plan.getField ( FIELD_NAME_ALIAS ) ;
      // optional condition
      BSONElement eleCondition  = plan.getField ( FIELD_NAME_CONDITION ) ;
      // optional selector
      BSONElement eleSelector   = plan.getField ( FIELD_NAME_SELECTOR ) ;
      // optional sort
      BSONElement eleSort       = plan.getField ( FIELD_NAME_SORT ) ;
      // optional hint
      BSONElement eleHint       = plan.getField ( FIELD_NAME_HINT ) ;
      // optional skip
      BSONElement eleSkip       = plan.getField ( FIELD_NAME_SKIP ) ;
      // optional return
      BSONElement eleReturn     = plan.getField ( FIELD_NAME_RETURN ) ;

      // sanity check for required operators
      PD_CHECK ( eleType.type()       == NumberInt &&
                 eleCollection.type() == String &&
                 eleAlias.type()      == String,
                 SDB_INVALIDARG, error, PDERROR,
                 "Invalid type, alias or collection: %s",
                 plan.toString().c_str() ) ;
      PD_CHECK ( eleType.numberInt() == QGM_OPTYPE_SCAN,
                 SDB_INVALIDARG, error, PDERROR,
                 "Invalid operator type, expects: %d, real %d, from %s",
                 QGM_OPTYPE_SCAN, eleType.numberInt(),
                 plan.toString().c_str() ) ;
      // allocate scan operator
      *output = SDB_OSS_NEW _qgmOperatorScan ( eleAlias.valuestr(),
                                               eleCollection.valuestr(),
                                               eleCondition.type() == Object ?
                                                eleCondition.embeddedObject():
                                                dummy,
                                               eleSelector.type() == Object ?
                                                eleSelector.embeddedObject():
                                                dummy,
                                               eleSort.type() == Object ?
                                                eleSort.embeddedObject():
                                                dummy,
                                               eleHint.type() == Object ?
                                                eleHint.embeddedObject():
                                                dummy,
                                               eleSkip.isNumber()?
                                                eleSkip.numberLong():0,
                                               eleReturn.isNumber()?
                                                eleReturn.numberLong():-1 ) ;
      PD_CHECK ( *output, SDB_OOM, error, PDERROR,
                 "Failed to create scan operator" ) ;
   done :
      return rc ;
   error :
      if ( *output )
      {
         SDB_OSS_DEL ( *output ) ;
         *output = NULL ;
      }
      goto done ;

   }

   std::string _qgmOperatorScan::toString ( INT32 &id )
   {
      std::stringstream out ;
      out << id << ") " << FIELD_NAME_SCAN << endl ;
      ++id ;
      out << "\t" << FIELD_NAME_ALIAS << "\t: " << _alias << endl ;
      out << "\t" << FIELD_NAME_COLLECTION << "\t: " <<
              _collectionName<< endl ;
      out << "\t" << FIELD_NAME_CONDITION << "\t: " <<
              _condition.toString() << endl ;
      out << "\t" << FIELD_NAME_SELECTOR << "\t: " <<
              _selector.toString() << endl ;
      out << "\t" << FIELD_NAME_SORT << "\t\t: " <<
              _orderBy.toString() << endl ;
      out << "\t" << FIELD_NAME_HINT << "\t\t: " <<
              _hint.toString() << endl ;
      out << "\t" << FIELD_NAME_SKIP << "\t\t: " <<
              _numSkip << endl ;
      out << "\t" << FIELD_NAME_RETURN << "\t\t: " <<
              _numReturn << endl ;
      return out.str() ;
   }

   BSONObj _qgmOperatorScan::toBson ()
   {
      BSONObjBuilder ob ;
      ob.append ( FIELD_NAME_TYPE, (INT32)_opType ) ;
      ob.append ( FIELD_NAME_ALIAS, _alias ) ;
      ob.append ( FIELD_NAME_COLLECTION, _collectionName ) ;
      ob.append ( FIELD_NAME_CONDITION, _condition ) ;
      ob.append ( FIELD_NAME_SELECTOR, _selector ) ;
      ob.append ( FIELD_NAME_SORT, _orderBy ) ;
      ob.append ( FIELD_NAME_HINT, _hint ) ;
      ob.append ( FIELD_NAME_SKIP, _numSkip ) ;
      ob.append ( FIELD_NAME_RETURN, _numReturn ) ;
      return ob.obj() ;
   }
}
