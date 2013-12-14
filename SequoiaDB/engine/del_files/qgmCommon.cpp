/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = qgmCommon.cpp

   Descriptive Name =

   When/how to use: this program may be used on binary and text-formatted
   versions of PMD component. This file contains implementation for qgm
   common variables and functions

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
#include "optQgmStrategy.hpp"

namespace engine
{
   BSONObj _qgmExistsPredicate ;
   BSONObj _qgmNotExistsPredicate ;
   INT32 qgmInit ()
   {
      INT32 rc = SDB_OK ;
      try
      {
         BSONObjBuilder boExists ;
         BSONObjBuilder boNotExists ;
         boExists.appendBool ( "$exists", TRUE ) ;
         boNotExists.appendBool ( "$exists", FALSE ) ;
         _qgmExistsPredicate = boExists.obj () ;
         _qgmNotExistsPredicate = boNotExists.obj () ;
      }
      catch ( std::exception &e )
      {
         PD_RC_CHECK ( SDB_SYS, PDERROR,
                       "Exception during qgm init: %s", e.what() ) ;
      }

      rc = getQgmStrategyTable()->init() ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "Init qgm strategy table failed, rc: %d", rc ) ;
         goto error ;
      }

   done :
      return rc ;
   error :
      goto done ;
   }

   _qgmOperatorJoin::~_qgmOperatorJoin ()
   {
      qgmJoinPredicateList::iterator it ;
      for ( it = _joinPredicates.begin() ;
            it != _joinPredicates.end() ;
            ++it )
      {
         SDB_OSS_DEL ( *it ) ;
      }
      _joinPredicates.clear() ;
   }

   INT32 _qgmOperatorBase::fromBson ( const BSONObj &plan,
                                      qgmOperatorBase **output )
   {
      INT32 rc = SDB_OK ;
      BSONObj dummy ;
      SDB_ASSERT ( output, "output can't be NULL" )
      // type is required for all type of operators
      BSONElement eleType = plan.getField ( FIELD_NAME_TYPE ) ;
      PD_CHECK ( eleType.type() == NumberInt,
                 SDB_INVALIDARG, error, PDERROR,
                 "Invalid type: %s",
                 plan.toString().c_str() ) ;
      switch ( eleType.numberInt () )
      {
      case QGM_OPTYPE_DELETE :
         return _qgmOperatorDelete::fromBson ( plan, output ) ;
      case QGM_OPTYPE_INSERT :
         return _qgmOperatorInsert::fromBson ( plan, output ) ;
      case QGM_OPTYPE_UPDATE :
         return _qgmOperatorUpdate::fromBson ( plan, output ) ;
      case QGM_OPTYPE_RETURN :
         return _qgmOperatorReturn::fromBson ( plan, output ) ;
      case QGM_OPTYPE_NLJOIN :
         return _qgmOperatorNLJoin::fromBson ( plan, output ) ;
      case QGM_OPTYPE_SORT :
         return _qgmOperatorSort::fromBson ( plan, output ) ;
      case QGM_OPTYPE_SCAN :
         return _qgmOperatorScan::fromBson ( plan, output ) ;
      case QGM_OPTYPE_FILTER :
         return _qgmOperatorFilter::fromBson ( plan, output ) ;
      case QGM_OPTYPE_RECORD :
         return _qgmOperatorRecord::fromBson ( plan, output ) ;
      default :
         PD_RC_CHECK ( SDB_INVALIDARG, PDERROR,
                       "Invalid type: %d from %s",
                       eleType.numberInt(),
                       plan.toString().c_str() ) ;
      }
   done :
      return rc ;
   error :
      goto done ;
   }
   INT32 qgmExtractPlan ( BSONObj &accessPlan , qgmOperatorBase **result )
   {
      INT32 rc = SDB_OK ;
      SDB_ASSERT ( result, "result can't be NULL" )
      rc = qgmOperatorBase::fromBson ( accessPlan, result ) ;
      PD_RC_CHECK ( rc, PDERROR, "Failed to convert from plan %s, rc = %d",
                    accessPlan.toString().c_str(), rc ) ;
   done :
      return rc ;
   error :
      goto done ;
   }
#if defined (_DEBUG)
   INT32 qgmDebugQuery ( BSONObj &queryPlan )
   {
      INT32 rc = SDB_OK ;
      BSONObj obj ;
      CHAR *pBuffer = NULL ;
      INT32 bufferSize = 0 ;
      _qgmOperatorType opType = QGM_OPTYPE_INSERT ;
      pmdEDUCB *cb = pmdGetKRCB()->getEDUMgr()->getEDU() ;

      qgmOperatorBase *operatorEntryPoint = NULL ;
      PD_LOG ( PDINFO, "Received Debug Access Plan: %s",
               queryPlan.toString().c_str() ) ;
      rc = qgmExtractPlan ( queryPlan, &operatorEntryPoint ) ;
      PD_RC_CHECK ( rc, PDERROR, "Failed to extract plan %s, rc = %d",
                    queryPlan.toString().c_str(), rc ) ;
      rc = operatorEntryPoint->execute ( cb ) ;
      PD_RC_CHECK ( rc, PDERROR, "Failed to execute, rc = %d", rc ) ;
      opType = operatorEntryPoint->getType() ;
      // skip fetch for IUD operators
      if ( opType == QGM_OPTYPE_INSERT ||
           opType == QGM_OPTYPE_UPDATE ||
           opType == QGM_OPTYPE_DELETE )
         goto done ;
      while ( TRUE )
      {
         rc = operatorEntryPoint->fetchNext ( obj ) ;
         if ( rc )
         {
            PD_CHECK ( SDB_DMS_EOC == rc, rc, error, PDERROR,
                       "Failed to fetch next record, rc = %d", rc ) ;
            break ;
         }
         PD_LOG ( PDEVENT, "Record: %s",
                  obj.toString().c_str() ) ;
      }
      bufferSize = 1024*1024*10 ;
      pBuffer = (CHAR*)SDB_OSS_MALLOC ( bufferSize ) ;
      PD_CHECK ( pBuffer, SDB_OOM, error, PDERROR,
                 "Failed to allocate buffer for 10MB" ) ;
      //rc = qgmDump ( operatorEntryPoint, pBuffer, bufferSize ) ;
      //PD_RC_CHECK ( rc, PDERROR, "Failed to dump qgm, rc = %d", rc ) ;
      //PD_LOG ( PDEVENT, "Plan:\n%s", pBuffer ) ;
   done :
      if ( operatorEntryPoint )
      {
         SDB_OSS_DEL ( operatorEntryPoint ) ;
      }
      if ( pBuffer )
      {
         SDB_OSS_FREE ( pBuffer ) ;
      }
      return rc ;
   error :
      goto done ;
   }
#endif

   INT32 _qgmOperatorJoin::_mergeResult ( const BSONObj &outer,
                                          const BSONObj &inner,
                                          BSONObj &output )
   {
      INT32 rc = SDB_OK ;
      BSONObjBuilder ob ;
      const CHAR *pFieldName = NULL ;
      std::set<const CHAR*, cmp_str> fieldNameMap ;
#if defined (_WINDOWS)
      std::set<const CHAR*, cmp_str>::iterator it ;
#elif defined (_LINUX)
      std::set<const CHAR*>::iterator it ;
#endif
      BSONObjIterator it0 ( outer ) ;
      BSONObjIterator it1 ( inner ) ;
      while ( it0.more() )
      {
         BSONElement ele = it0.next() ;
         pFieldName = ele.fieldName() ;
         it = fieldNameMap.find( pFieldName ) ;
         PD_CHECK ( it == fieldNameMap.end(),
                    SDB_RTN_DUPLICATE_FIELDNAME, error, PDERROR,
                    "Duplicate field name exists: %s\nouter: %s\ninner: %s",
                    ele.fieldName(),
                    outer.toString().c_str(),
                    inner.toString().c_str() ) ;
         fieldNameMap.insert ( pFieldName ) ;
         ob.append ( ele ) ;
      }
      while ( it1.more() )
      {
         BSONElement ele = it1.next() ;
         pFieldName = ele.fieldName() ;
         it = fieldNameMap.find( pFieldName ) ;
         PD_CHECK ( it == fieldNameMap.end(),
                    SDB_RTN_DUPLICATE_FIELDNAME, error, PDERROR,
                    "Duplicate field name exists: %s\nouter: %s\ninner: %s",
                    ele.fieldName(),
                    outer.toString().c_str(),
                    inner.toString().c_str() ) ;
         fieldNameMap.insert ( pFieldName ) ;
         ob.append ( ele ) ;
      }
      output = ob.obj() ;
   done :
      return rc ;
   error :
      goto done ;
   }
}
