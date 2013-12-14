/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = qgmNLJoin.cpp

   Descriptive Name =

   When/how to use: this program may be used on binary and text-formatted
   versions of PMD component. This file contains implementation for qgm
   nested loop join operator

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
namespace engine
{
   _qgmOperatorSort::_qgmOperatorSort ( const CHAR *pAlias,
                                        const BSONObj &orderBy )
   {
      INT32 rc          = SDB_OK ;
      _pContext         = NULL ;
      _contextID        = -1 ;
      _orderBy          = orderBy ;
      _opType           = QGM_OPTYPE_SORT ;
      _tempCreated      = FALSE ;
      _tempSU           = NULL ;
      _rtnCB            = pmdGetKRCB()->getRTNCB () ;
      _tempCB           = pmdGetKRCB()->getDMSCB()->getTempCB() ;
      ossMemset ( _tempTableName, 0, sizeof(_tempTableName) ) ;
      _alias            = ossStrdup ( pAlias ) ;
      PD_CHECK ( _alias, SDB_OOM, error, PDERROR,
                 "Failed to allocate memory for alias" ) ;
      _isConstructed = TRUE ;
   done :
      return ;
   error :
      goto done ;
   }

   _qgmOperatorSort::~_qgmOperatorSort()
   {
      close () ;
   }

   INT32 _qgmOperatorSort::execute ( _pmdEDUCB *eduCB )
   {
      INT32 rc               = SDB_OK ;
      SDB_ASSERT ( eduCB, "eduCB can't be NULL" )
      _eduCB                 = eduCB ;
      _monAppCB              = _eduCB->getMonAppCB() ;
      // get role for coord or data
      SDB_ASSERT ( _inputLegs.size() == 1, "1 legs is expected" )
      _sortHint              = BSON ( ""<<RTN_SORT_INDEX_NAME ) ;
      _tempID                = 0 ;
      _clLID                 = 0 ;
      // reserve temp table first
      rc = _tempCB->reserve ( _tempID, _clLID, _eduCB->getID () ) ;
      PD_RC_CHECK ( rc, PDERROR,
                    "Failed to reserve temp collection, rc = %d", rc ) ;
      _tempCreated = TRUE ;
      // then create index using orderBy
      ossSnprintf ( _tempTableName, sizeof(_tempTableName),
                    DMS_TEMP_NAME_PATTERN,
                    SDB_DMSTEMP_NAME, _tempID ) ;
      _tempSU = _tempCB->getTempSU () ;
      // tempSU can't be NULL
      rc = _tempSU->createIndex ( _tempID, _clLID,
                       BSON("key"<<_orderBy<<"name"<<RTN_SORT_INDEX_NAME),
                       _eduCB, NULL,
                       TRUE, TRUE ) ;
      PD_RC_CHECK ( rc, PDERROR,
                    "Failed to create index on temp table, rc = %d",
                    rc ) ;
      rc = _inputLegs[0]->execute ( _eduCB ) ;
      PD_RC_CHECK ( rc, PDERROR,
                    "Failed to execute source, rc = %d", rc ) ;
      _hasExecuted = TRUE ;
   done :
      return rc ;
   error :
      close () ;
      goto done ;
   }
   void _qgmOperatorSort::close ()
   {
      if ( _tempCreated )
      {
         _tempCB->release ( _tempID ) ;
         _tempCreated = FALSE ;
      }
      if ( -1 != _contextID )
      {
         _eduCB->contextDelete ( _contextID ) ;
         _rtnCB->contextDelete ( _contextID ) ;
         _contextID = -1 ;
      }
   }

   // fetch next record, non-block operation
   INT32 _qgmOperatorSort::fetchNext ( BSONObj &obj )
   {
      INT32 rc           = SDB_OK ;
      CHAR *pBuffer      = NULL ;
      SINT32 bufLen      = 0 ;
      SINT32 recordNum   = 0 ;
      SINT64 startingPos = 0 ;

      PD_CHECK  ( _hasExecuted, SDB_SYS, error, PDERROR,
                  "NLJoin was not properly executed" ) ;
      // first let's see if the sort is already done or not
      if ( -1 == _contextID )
      {
         rc = _fillupTemp () ;
         PD_RC_CHECK ( rc, PDERROR, "Failed to fill up temp, rc = %d", rc ) ;
      }
      SDB_ASSERT ( _pContext, "context can't be NULL" )
      rc = rtnGetMore ( _contextID, 1, &pBuffer, bufLen, recordNum,
                        startingPos, _eduCB, _rtnCB ) ;
      try
      {
         if ( SDB_OK == rc && pBuffer && bufLen > 0 && recordNum > 0 )
         {
            obj = BSONObj ( pBuffer ) ;
         }
         else
         {
            if ( SDB_DMS_EOC != rc )
            {
               PD_RC_CHECK ( rc, PDERROR, "get more failed, rc = %d", rc ) ;
            }
            goto error ;
         }
      }
      catch ( std::exception &e )
      {
         PD_RC_CHECK ( SDB_SYS, PDERROR,
                       "Received exception %s", e.what() ) ;
      }
   done :
      return rc ;
   error :
      close () ;
      goto done ;
   }

   INT32 _qgmOperatorSort::_fillupTemp ()
   {
      INT32 rc = SDB_OK ;
      rtnAccessPlanManager *apm = NULL ;
      optAccessPlan *plan       = NULL ;
      BSONObj emptyObj ;
      BSONObj obj ;
      qgmOperatorBase *childOperator = _inputLegs[0] ;
      // first time calling fetchNext, we have to block operation and perform
      // full sort
      while ( TRUE )
      {
         rc = childOperator->fetchNext(obj) ;
         if ( rc )
         {
            if ( SDB_DMS_EOC != rc )
            {
               PD_LOG ( PDERROR, "Error detected during fetch, rc = %d", rc ) ;
               goto error ;
            }
            rc = SDB_OK ;
            break ;
         }
         // insert each row into the temp table
         try
         {
            rc = _tempSU->insertRecord ( _tempID, _clLID,
                                         obj, _eduCB, NULL, TRUE ) ;
            PD_RC_CHECK ( rc, PDERROR,
                          "Failed to insert record into temp table" ) ;
            DMS_MON_OP_COUNT_INC( _monAppCB, MON_TEMP_WRITE, 1 ) ;
         }
         catch ( std::exception &e )
         {
            PD_RC_CHECK ( SDB_SYS, PDERROR,
                          "Exception when insert into temp: %s",
                          e.what() ) ;
         }
      } // while ( TRUE )
      rc = _rtnCB->contextNew ( &_pContext, _contextID, _eduCB, NULL, _tempSU ) ;
      PD_RC_CHECK ( rc, PDERROR, "Failed to create new context" ) ;
      _pContext->_numToReturn = -1 ;
      _pContext->_numToSkip   = 0 ;
      _pContext->_eduID       = _eduCB->getID() ;
      // sample timestamp
      if ( _eduCB->getMonConfigCB()->timestampON )
      {
         _pContext->_monCtxCB.recordStartTimestamp() ;
      }
      _eduCB->contextInsert ( _contextID ) ;
      apm = _tempSU->getAPM () ;
      SDB_ASSERT ( apm, "apm shouldn't be NULL" )

      // for temp scan, let's simply allocate new plan instead of using APM
      // memory will be deleted in context destructor
      plan = SDB_OSS_NEW optAccessPlan ( _tempSU, _tempTableName, emptyObj,
                                         emptyObj, _sortHint ) ;
      PD_CHECK ( plan, SDB_OOM, error, PDERROR,
                 "failed to create new access plan" ) ;
      rc = plan->optimize() ;
      PD_RC_CHECK ( rc, PDERROR, "Failed to optimize plan, rc = %d", rc ) ;
      _pContext->_plan = plan ;
      _pContext->_tempCB = _tempCB ;
      rc = rtnQueryIXScan ( _tempID, _clLID, emptyObj, _tempSU,
                            _pContext, _eduCB ) ;
      PD_RC_CHECK ( rc, PDERROR,
                    "Failed to do index scan for temp, context %lld",
                    _contextID ) ;
   done :
      return rc ;
   error :
      close () ;
      goto done ;
   }
   INT32 _qgmOperatorSort::pushDownPredicates ( const BSONObj &pred )
   {
      return _inputLegs[0]->pushDownPredicates ( pred ) ;
   }
   INT32 _qgmOperatorSort::fromBson ( const BSONObj &plan,
                                      _qgmOperatorBase **output )
   {
      INT32 rc                    = SDB_OK ;
      SDB_ASSERT ( output, "output can't be NULL" )
      *output                     = NULL ;
      qgmOperatorBase *dataSource = NULL ;
      // required for sanity check
      BSONElement eleType         = plan.getField ( FIELD_NAME_TYPE ) ;
      // required for sort field
      BSONElement eleSort         = plan.getField ( FIELD_NAME_SORT ) ;
      // required for input
      BSONElement eleSource       = plan.getField ( FIELD_NAME_SOURCE ) ;
      // required for alias
      BSONElement eleAlias        = plan.getField ( FIELD_NAME_ALIAS ) ;
      // sanity check for type, sort and input
      PD_CHECK ( eleType.type()         == NumberInt &&
                 eleSort.type()         == Object &&
                 eleSource.type()       == Object &&
                 eleAlias.type()        == String,
                 SDB_INVALIDARG, error, PDERROR,
                 "Invalid type, orderBy, alias or source: %s",
                 plan.toString().c_str() ) ;
      // sanity check for type
      PD_CHECK ( eleType.numberInt() == QGM_OPTYPE_SORT,
                 SDB_INVALIDARG, error, PDERROR,
                 "Invalid operator type, expects: %d, real %d, from %s",
                 QGM_OPTYPE_SORT, eleType.numberInt(),
                 plan.toString().c_str() ) ;
      // generate data source
      rc = _qgmOperatorBase::fromBson ( eleSource.embeddedObject(),
                                        &dataSource ) ;
      PD_RC_CHECK ( rc, PDERROR, "Failed to generate qgm operator from %s",
                    eleSource.embeddedObject().toString().c_str() ) ;
      SDB_ASSERT ( dataSource, "data source can't be NULL here" )
      // alocate sort operator
      *output = SDB_OSS_NEW _qgmOperatorSort ( eleAlias.valuestr(),
                                               eleSort.embeddedObject() ) ;
      PD_CHECK ( *output, SDB_OOM, error, PDERROR,
                  "Failed to allocate insert operator" ) ;
      // assign data source
      rc = (*output)->addInput ( dataSource ) ;
      PD_RC_CHECK ( rc, PDERROR, "Failed to add input, rc = %d", rc ) ;
      dataSource = NULL ;
   done:
      return rc ;
   error :
      if ( *output )
      {
         SDB_OSS_DEL (*output) ;
         *output = NULL ;
      }
      if ( dataSource )
      {
         SDB_OSS_DEL ( dataSource ) ;
         dataSource = NULL ;
      }
      goto done ;
   }
   std::string _qgmOperatorSort::toString ( INT32 &id )
   {
      std::stringstream out ;
      out << id << ") " << FIELD_NAME_SORT << endl ;
      ++id ;
      out << "\t" << FIELD_NAME_ALIAS << "\t: " << _alias << endl ;
      out << "\t" << FIELD_NAME_SORT << "\t\t: " <<
              _orderBy.toString() << endl ;
      if ( _inputLegs.size() == 1 )
      {
         out << "\n" ;
         out << _inputLegs[0]->toString ( id ) ;
      }
      return out.str() ;
   }
   BSONObj _qgmOperatorSort::toBson ()
   {
      BSONObjBuilder ob ;
      ob.append ( FIELD_NAME_TYPE, (INT32)_opType ) ;
      ob.append ( FIELD_NAME_ALIAS, _alias ) ;
      ob.append ( FIELD_NAME_SORT, _orderBy ) ;
      if ( _inputLegs.size() == 1 )
         ob.append ( FIELD_NAME_SOURCE, _inputLegs[0]->toBson() ) ;
      return ob.obj() ;
   }
}
