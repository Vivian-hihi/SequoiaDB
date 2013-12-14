/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = qgmMSJoin.cpp

   Descriptive Name =

   When/how to use: this program may be used on binary and text-formatted
   versions of PMD component. This file contains implementation for qgm
   merge/sort join operator

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
   extern BSONObj _qgmExistsPredicate ;
   extern BSONObj _qgmNotExistsPredicate ;

   _qgmOperatorMSJoin::_qgmOperatorMSJoin ( const BSONObj &selector,
                                            const BSONObj &hint,
                                            const CHAR *pOuterAlias,
                                            const CHAR *pInnerAlias,
                                            _qgmJoinType joinType,
                                            BOOLEAN earlyOut,
                                            INT64 numSkip,
                                            INT64 numReturn )
   {
      INT32 rc          = SDB_OK ;
      _hasExecuted      = FALSE ;
      _opType           = QGM_OPTYPE_MSJOIN ;
      _outerSide        = NULL ;
      _innerSide        = NULL ;
      _earlyOut         = earlyOut ;
      _innerEnd         = TRUE ;
      _joinType         = joinType ;
      _numSkip          = numSkip ;
      _numReturn        = numReturn ;
      _currentSkip      = 0 ;
      _currentReturn    = 0 ;
      _existSelector    = FALSE ;
      _contextID        = -1 ;
      _tempID           = 0 ;
      _clLID            = 0 ;
      _tempCreated      = FALSE ;
      _tempSU           = NULL ;
      _pContext         = NULL ;
      _scanStarts       = FALSE ;
      _isDoingCrossProduct = FALSE ;
      INT32 len         = 0 ;
      SDB_ASSERT ( pOuterAlias, "outer alias can't be NULL" )
      SDB_ASSERT ( pInnerAlias, "inner alias can't be NULL" )
      _innerJoinNum     = 0 ;
      try
      {
         _selectorObj   = selector.copy () ;
         _hint          = hint.copy () ;
      }
      catch ( std::exception &e )
      {
         PD_RC_CHECK ( SDB_SYS, PDERROR,
                       "Exception when init NLJOIN: %s", e.what() ) ;
      }
      // memory free in qgmOperatorJoin destructor ( in qgmCommon.cpp )
      len               = ossStrlen ( pOuterAlias ) ;
      _outerAlias       = (CHAR*)SDB_OSS_MALLOC ( len + 1 ) ;
      PD_CHECK ( _outerAlias, SDB_OOM, error, PDERROR,
                 "Failed to allocate memory for outer alias" ) ;
      ossStrncpy ( _outerAlias, pOuterAlias, len + 1 ) ;
      len               = ossStrlen ( pInnerAlias ) ;
      _innerAlias       = (CHAR*) SDB_OSS_MALLOC ( len + 1 ) ;
      PD_CHECK ( _innerAlias, SDB_OOM, error, PDERROR,
                 "Failed to allocate memory for inner alias" ) ;
      ossStrncpy ( _innerAlias, pInnerAlias, len + 1 ) ;
      _rtnCB            = pmdGetKRCB()->getRTNCB () ;
      _tempCB           = pmdGetKRCB()->getDMSCB()->getTempCB() ;
      ossMemset ( _tempTableName, 0, sizeof(_tempTableName) ) ;
      _outerKeyGen      = NULL ;
      _innerKeyGen      = NULL ;
      _isConstructed    = TRUE ;
   done :
      return ;
   error :
      goto done ;
   }

   _qgmOperatorMSJoin::~_qgmOperatorMSJoin()
   {
      close () ;
   }

   void _qgmOperatorMSJoin::close ()
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
      if ( _outerKeyGen )
      {
         SDB_OSS_DEL ( _outerKeyGen ) ;
         _outerKeyGen = NULL ;
      }
      if ( _innerKeyGen )
      {
         SDB_OSS_DEL ( _innerKeyGen ) ;
         _innerKeyGen = NULL ;
      }
   }

   // MSJOIN always assume the both input legs are properly sorted by join key
   INT32 _qgmOperatorMSJoin::execute ( _pmdEDUCB *eduCB )
   {
      INT32 rc      = SDB_OK ;
      BSONObjBuilder obOuter, obInner ;
      qgmJoinPredicateList::iterator it ;
      _eduCB        = eduCB ;
      PD_CHECK ( _isConstructed, SDB_SYS, error, PDERROR,
                 "MSJoin was not properly constructed" ) ;
      // get role for coord or data
      SDB_ASSERT ( _inputLegs.size() == 2, "2 legs are expected" )
      _outerSide     = _inputLegs[0] ;
      _innerSide     = _inputLegs[1] ;
      _currentSkip   = 0 ;
      _currentReturn = 0 ;
      _isDoingCrossProduct = FALSE ;
      _compareResult = QGM_OPERATOR_COMPARE_RESULT_BEGIN ;
      _innerUsed = FALSE ;
      _outerUsed = FALSE ;
      // build merge selector
      if ( !_selectorObj.isEmpty () )
      {
         rc = _selector.loadPattern ( _selectorObj, _outerAlias,
                                      _innerAlias ) ;
         PD_RC_CHECK ( rc, PDERROR,
                       "Failed to load pattern for inner, rc = %d", rc ) ;
         _existSelector = TRUE ;
      }
      // extract join predicates
      for ( it = _joinPredicates.begin(); it != _joinPredicates.end(); ++it )
      {
         qgmJoinPredicate* pred = (*it) ;
         PD_CHECK ( pred->_type == QGM_PDTYPE_EQ,
                    SDB_QGM_MERGE_JOIN_EQONLY, error, PDERROR,
                    "Unsupported predicate for merge join: %d", pred->_type ) ;
         obOuter.append ( pred->_pFieldOuter, 1 ) ;
         obInner.append ( pred->_pFieldInner, 1 ) ;
      }
      _outerOrderBy = obOuter.obj () ;
      _innerOrderBy = obInner.obj () ;
      _outerKeyGen = (ixmIndexKeyGen*)SDB_OSS_NEW ixmIndexKeyGen
            ( _outerOrderBy ) ;
      PD_CHECK ( _outerKeyGen, SDB_OOM, error, PDERROR,
                 "Failed to allocate outerKeyGen" ) ;
      _innerKeyGen = (ixmIndexKeyGen*)SDB_OSS_NEW ixmIndexKeyGen
            ( _innerOrderBy ) ;
      PD_CHECK ( _innerKeyGen, SDB_OOM, error, PDERROR,
                 "Failed to allocate innerKeyGen" ) ;
      // execute outerSide
      rc = _outerSide->execute( _eduCB ) ;
      PD_RC_CHECK ( rc, PDERROR, "Failed to execute outer, rc = %d", rc ) ;
      // execute innerSide
      rc = _innerSide->execute ( _eduCB ) ;
      PD_RC_CHECK ( rc, PDERROR, "Failed to execute inner, rc = %d", rc ) ;
      // inner side will be executed in fetchNext
      _hasExecuted = TRUE ;
   done :
      return rc ;
   error :
      close () ;
      goto done ;
   }

   INT32 _extractOuter ( BSONObj &outer )
   {
   }

   // fetch next record, non-block operation
   INT32 _qgmOperatorMSJoin::fetchNext ( BSONObj &obj )
   {
      INT32 rc = SDB_OK ;
      SDB_ASSERT ( _outerSide, "outer can't be NULL" )
      SDB_ASSERT ( _innerSide, "inner can't be NULL" )
      SDB_ASSERT ( _outerKeyGen, "outer key gen can't be NULL" )
      SDB_ASSERT ( _innerKeyGen, "inner key gen can't be NULL" )
      BSONObj innerObj ;
      PD_CHECK  ( _hasExecuted, SDB_SYS, error, PDERROR,
                  "NLJoin was not properly executed" ) ;
      // if there's nothing else to return, simply return EOC
      if ( _numReturn >= 0 && _currentReturn >= _numReturn )
      {
         close () ;
         rc = SDB_DMS_EOC ;
         goto error ;
      }

      // begin
      // case not cross_product:
      //    0) case compare_result ( from previous run )
      //    1)   begin: read one record from inner to _innerRecord and read one
      //                record from outer to _outerRecord ( this only happen at
      //                begin phase ), _innerUsed/_outerUsed = FALSE
      //    2)   equal: read one record from inner to _innerRecord,
      //                _innerUsed=FALSE
      //    3)   smaller: read one record from outer to _outerRecord,
      //                  _outerUsed=FALSE
      //    4)   greater: read one record from inner to _innerRecord
      //                  _innerUsed=FALSE
      //    5) Compare _outerRecord and _innerRecord
      //    6) case compare_result ( from current run )
      //    7)   equal: push inner record to temp store, and
      //                build return result, _innerUsed=_outerUsed=TRUE
      //         smaller: if temp store is not empty, mark cross_product and
      //                  read next from outer to _outer, and goto begin
      //                  else if louterjoin or full join and _outerUsed = FALSE,
      //                  build result from outer,
      //                  else goto begin
      //         greater: if routjoin or full join and _innerUsed = FALSE,
      //                  build result from inner
      //                  else goto begin
      // case in cross_product:
      //    1) if hit end of temp store, read next from outer, read first from
      //       temp store
      //    1.1) case compare_result ( from current run )
      //    1.1.1)   equal: build return result, _outerUsed = TRUE
      //    1.1.2)   smaller: WRONG
      //    1.1.3)   greater: unset cross_product, clear temp store, goto
      //    not_cross_product.(5)
      //    2) otherwise, read next from temp store and build return record
      if ( !_isDoingCrossProduct )
      {
         // not cross product condition
         switch ( _compareResult )
         {
         case QGM_OPERATOR_COMPARE_RESULT_BEGIN:
            // this case only happen once at very first run
            rc = _outerSide->fetchNext ( _outerRecord ) ;
            if ( rc )
            {
               PD_CHECK ( SDB_DMS_EOC != rc, rc, error, PDERROR,
                          "Failed to fetch next from outerside, rc = %d", rc ) ;
               goto error ;
            }
            rc = _innerSide->fetchNext ( _innerRecord ) ;
            if ( rc )
            {
               PD_CHECK ( SDB_DMS_EOC != rc, rc, error, PDERROR,
                          "Failed to fetch next from innerside, rc = %d", rc ) ;
               goto error ;
            }
            rc = _outerKeyGen->getKeys ( _outerRecord, outerKeys ) ;
            _innerUsed = FALSE ;
            _outerUsed = FALSE ;
            break ;
         case QGM_OPERATOR_COMPARE_RESULT_EQUAL :
         case QGM_OPERATOR_COMPARE_RESULT_INNERSMALL:
            // this case happen when records are same in previous run
            // or the inner record is smaller than outer side
            rc = _innerSide->fetchNext ( _innerRecord ) ;
            if ( rc )
            {
               PD_CHECK ( SDB_DMS_EOC != rc, rc, error, PDERROR,
                          "Failed to fetch next from innerside, rc = %d", rc ) ;
               goto error ;
            }
            _innerUsed = FALSE ;
            break ;
         case QGM_OPERATOR_COMPARE_RESULT_OUTERSMALL:
            // this case when outer record is smaller than inner in previous run
            rc = _outerSide->fetchNext ( _outerRecord ) ;
            if ( rc )
            {
               PD_CHECK ( SDB_DMS_EOC != rc, rc, error, PDERROR,
                          "Failed to fetch next from outerside, rc = %d", rc ) ;
               goto error ;
            }
            _outerUsed = FALSE ;
            break ;
         }
         // now we have read records, let's compare

      }
      else
      {
      }
   retry_outer_fetch :
      if ( _innerEnd )
      {
         BSONObjBuilder ob ;
         rc = _outerSide->fetchNext ( _outerObj ) ;
         if ( rc )
         {
            // if we get any error other than EOC, let's dump error code
            PD_CHECK ( SDB_DMS_EOC == rc, rc, error, PDERROR,
                       "Failed to fetch from outer, rc = %d", rc ) ;
            goto error ;
         }
         // if user defined any join predicates
         // once we get the record, let's extract the join keys
         rc = buildInnerSearchCondition ( _outerObj, ob ) ;
         if ( rc )
         {
            if ( SDB_QGM_MATCH_NONE == rc )
            {
               // if the current record is not possible to match anything,
               // let's fetch next outer
               if ( QGM_JNTYPE_LEFT == _joinType )
               {
                  // for left outer join, we have to build result based on
                  // outerObj
                  goto build_result ;
               }
               goto retry_outer_fetch ;
            }
            PD_RC_CHECK ( rc, PDERROR,
                          "Failed to build inner search condition, "
                          "rc = %d", rc ) ;
         }
         try
         {
            rc = pushDownPredicates ( ob.obj() ) ;
            PD_RC_CHECK ( rc, PDERROR,
                          "Failed to push down predicate to inner, rc = %d",
                          rc ) ;
         }
         catch ( std::exception &e )
         {
            PD_RC_CHECK ( SDB_SYS, PDERROR,
                          "Exception when building search condition: %s",
                          e.what() ) ;
         }
         // mark inner loop starts
         _innerEnd = FALSE ;
         // mark there's no records from inner read yet,
         // this is for left outer join
         _innerJoinNum = 0 ;
         // restart execution with new predicate
         rc = _innerSide->execute ( _eduCB ) ;
         PD_RC_CHECK ( rc, PDERROR,
                       "Failed to reexecute inner side, rc = %d", rc ) ;
      }
      // fetch one record from inner side
      rc = _innerSide->fetchNext ( innerObj ) ;
      if ( rc )
      {
         if ( SDB_DMS_EOC == rc )
         {
            // if there's no matching from inner join and user want left join,
            // we have to build output from the inner join
            if ( 0 == _innerJoinNum && QGM_JNTYPE_LEFT == _joinType )
            {
               innerObj = BSONObj() ;
               goto build_result ;
            }
            else
            {
               // otherwise if we already scaned record from inner, let's ignore
               // join type and loop to next outer
               _innerEnd = TRUE ;
               goto retry_outer_fetch ;
            }
         }
         PD_RC_CHECK ( rc, PDERROR,
                       "Failed to fetch next from inner, rc = %d", rc ) ;
      }
   build_result :
      // if we still want to skip something
      if ( _currentSkip < _numSkip )
      {
         _currentSkip ++ ;
         goto retry_outer_fetch ;
      }
      // when we get here, it means we have both outer and inner, let's first
      // attempt to make outer result. Since we may have 1 outer matches 100000
      // inner result, we only need to get outer result once
      // why we build outerResult here instead of earlier? because we may have
      // 10000 outer result but no one matches anything from inner. In this case
      // we don't have to waste time to build result for the ones without any
      // return. Thus we build outer result only when needed
      if ( _existSelector )
      {
         rc = _selector.select ( _outerObj, innerObj, obj ) ;
         PD_RC_CHECK ( rc, PDERROR,
                       "Failed to select for inner, rc = %d", rc ) ;
      }
      else
      {
         // if there's no selector defined in Join, we need to merge all fields
         // from both side
         rc = _mergeResult ( _outerObj, innerObj, obj ) ;
         PD_RC_CHECK ( rc, PDERROR,
                       "Failed to merge result, rc = %d", rc ) ;
      }
   done :
      return rc ;
   error :
      goto done ;
   }
   INT32 _qgmOperatorNLJoin::pushDownPredicates ( const BSONObj &pred )
   {
      SDB_ASSERT ( _innerSide, "inner side can't be NULL" )
      return _innerSide->pushDownPredicates ( pred ) ;
   }

   INT32 _qgmOperatorNLJoin::addPredicate ( qgmJoinPredicate *pred )
   {
      INT32 rc = SDB_OK ;
      try
      {
         _joinPredicates.push_back ( pred ) ;
      }
      catch ( std::exception &e )
      {
         PD_RC_CHECK ( SDB_SYS, PDERROR, "Failed to push pred into list: %s",
                       e.what() ) ;
      }
   done :
      return rc ;
   error :
      goto done ;
   }

   INT32 _qgmOperatorNLJoin::fromBson ( const BSONObj &plan,
                                        _qgmOperatorBase **output )
   {
      INT32 rc                             = SDB_OK ;
      BSONObj dummy ;
      qgmJoinPredicate *pred               = NULL ;
      SDB_ASSERT ( output, "output can't be NULL" )
      qgmOperatorBase *outerLeg            = NULL ;
      qgmOperatorBase *innerLeg            = NULL ;
      *output                              = NULL ;
      // required for sanity check
      BSONElement eleType       = plan.getField ( FIELD_NAME_TYPE ) ;
      // required for join type
      BSONElement eleJoinType   = plan.getField ( FIELD_NAME_JOINTYPE ) ;
      // optional for join predicate
      BSONElement eleJoinPred   = plan.getField ( FIELD_NAME_JOINPRED ) ;
      // required for outer alias
      BSONElement eleOuterAlias = plan.getField ( FIELD_NAME_OUTERALIAS ) ;
      // required for inner alias
      BSONElement eleInnerAlias = plan.getField ( FIELD_NAME_INNERALIAS ) ;
      // optional selector
      BSONElement eleSelector   = plan.getField ( FIELD_NAME_SELECTOR ) ;
      // optional hint
      BSONElement eleHint       = plan.getField ( FIELD_NAME_HINT ) ;
      // optional skip
      BSONElement eleSkip       = plan.getField ( FIELD_NAME_SKIP ) ;
      // optional return
      BSONElement eleReturn     = plan.getField ( FIELD_NAME_RETURN ) ;
      // optional earlyOut
      BSONElement eleEarlyOut   = plan.getField ( FIELD_NAME_EARLYOUT ) ;
      // required outer
      BSONElement eleOuter      = plan.getField ( FIELD_NAME_OUTER ) ;
      // required inner
      BSONElement eleInner      = plan.getField ( FIELD_NAME_INNER ) ;

      // sanity check for required operators
      PD_CHECK ( eleType.type()          == NumberInt &&
                 eleJoinType.type()      == NumberInt &&
                 eleOuterAlias.type()    == String &&
                 eleInnerAlias.type()    == String &&
                 eleOuter.type()         == Object &&
                 eleInner.type()         == Object,
                 SDB_INVALIDARG, error, PDERROR,
                 "Invalid type, jointype, outeralias, "
                 "inneralias, outer or inner: %s",
                 plan.toString().c_str() ) ;
      PD_CHECK ( eleType.numberInt() == QGM_OPTYPE_NLJOIN,
                 SDB_INVALIDARG, error, PDERROR,
                 "Invalid operator type, expects: %d, real %d, from %s",
                 QGM_OPTYPE_NLJOIN, eleType.numberInt(),
                 plan.toString().c_str() ) ;
      // generate outer
      rc = _qgmOperatorBase::fromBson ( eleOuter.embeddedObject(),
                                        &outerLeg ) ;
      PD_RC_CHECK ( rc, PDERROR, "Failed to generate outer from %s",
                    eleOuter.embeddedObject().toString().c_str() ) ;
      SDB_ASSERT ( outerLeg, "outer leg can't be NULL here" )
      // generate inner
      rc = _qgmOperatorBase::fromBson ( eleInner.embeddedObject(),
                                        &innerLeg ) ;
      PD_RC_CHECK ( rc, PDERROR, "Failed to generate inner from %s",
                    eleInner.embeddedObject().toString().c_str() ) ;
      SDB_ASSERT ( innerLeg, "inner leg can't be NULL here" ) ;
      // allocate nljoin operator
      *output = SDB_OSS_NEW _qgmOperatorNLJoin ( eleSelector.type() == Object?
                                                  eleSelector.embeddedObject():
                                                  dummy,
                                                 eleHint.type() == Object?
                                                  eleHint.embeddedObject():
                                                  dummy,
                                                 eleOuterAlias.valuestr(),
                                                 eleInnerAlias.valuestr(),
                                                 (_qgmJoinType)
                                                    eleJoinType.numberInt(),
                                                 eleEarlyOut.type() == Bool?
                                                  eleEarlyOut.boolean():
                                                  FALSE,
                                                 eleSkip.isNumber()?
                                                  eleSkip.numberLong():
                                                  0,
                                                 eleReturn.isNumber()?
                                                  eleReturn.numberLong():
                                                  -1
                                                 ) ;
      PD_CHECK ( *output, SDB_OOM, error, PDERROR,
                 "Failed to create new NLJoin operator" ) ;
      // convert eleJoinPred into qgmJoinPredicateList
      if ( eleJoinPred.type() == Array )
      {
         BSONObjIterator it ( eleJoinPred.embeddedObject() ) ;
         while ( it.more () )
         {
            BSONElement ele = it.next() ;
            pred = NULL ;
            PD_CHECK ( ele.type() == Object, SDB_INVALIDARG, error, PDERROR,
                       "element in predlist must be object" ) ;
            pred = SDB_OSS_NEW qgmJoinPredicate() ;
            PD_CHECK ( pred, SDB_OOM, error, PDERROR,
                       "Failed to allocate new join predicate" ) ;
            rc = pred->fromBson ( ele.embeddedObject() ) ;
            PD_RC_CHECK ( rc, PDERROR,
                          "Failed to convert predlist element from bson, "
                          "rc = %d", rc ) ;
            rc = ((_qgmOperatorNLJoin*)(*output))->addPredicate ( pred ) ;
            PD_RC_CHECK ( rc, PDERROR,
                          "Failed to add predicate, rc = %d", rc ) ;
            // set to NULL indicating we don't delete memory when error
            // happened, memory will be removed from output's destructor
            pred = NULL ;
         }
      }
      // assign outer side
      rc = (*output)->addInput ( outerLeg ) ;
      PD_RC_CHECK ( rc, PDERROR,
                    "Failed to add outer into nljoin, rc = %d",
                    rc ) ;
      // remove the pointer here because *output is already added, so that we
      // don't want to free memory if something goes wrong ( destructor will
      // release memory )
      outerLeg = NULL ;
      // assign inner side
      rc = (*output)->addInput ( innerLeg ) ;
      PD_RC_CHECK ( rc, PDERROR,
                    "Failed to add inner into nljoin, rc = %d",
                    rc ) ;
      innerLeg = NULL ;
   done :
      SDB_ASSERT ( outerLeg == NULL && innerLeg == NULL,
                   "legs must be NULL" )
      return rc ;
   error :
      if ( *output )
      {
         SDB_OSS_DEL ( *output ) ;
         *output = NULL ;
      }
      if ( outerLeg )
      {
         SDB_OSS_DEL ( outerLeg ) ;
         outerLeg = NULL ;
      }
      if ( innerLeg )
      {
         SDB_OSS_DEL ( innerLeg ) ;
         innerLeg = NULL ;
      }
      if ( pred )
      {
         SDB_OSS_DEL ( pred ) ;
         pred = NULL ;
      }
      goto done ;
   }

   BSONObj _qgmOperatorNLJoin::toBson ()
   {
      BSONObjBuilder ob ;
      ob.append ( FIELD_NAME_TYPE, (INT32)_opType ) ;
      ob.append ( FIELD_NAME_JOINTYPE, (INT32)_joinType ) ;
      BSONArrayBuilder ab ;
      qgmJoinPredicateList::iterator it ;
      for ( it = _joinPredicates.begin() ;
            it != _joinPredicates.end() ;
            ++it )
      {
         ab.append ( (*it)->toBson() ) ;
      }
      ob.append ( FIELD_NAME_JOINPRED, ab.arr() ) ;
      ob.append ( FIELD_NAME_OUTERALIAS, _outerAlias ) ;
      ob.append ( FIELD_NAME_INNERALIAS, _innerAlias ) ;
      ob.append ( FIELD_NAME_HINT, _hint ) ;
      ob.append ( FIELD_NAME_SELECTOR, _selectorObj ) ;
      ob.append ( FIELD_NAME_SKIP, _currentSkip ) ;
      ob.append ( FIELD_NAME_RETURN, _currentSkip ) ;
      ob.appendBool ( FIELD_NAME_EARLYOUT, _earlyOut ) ;
      if ( _outerSide )
         ob.append ( FIELD_NAME_OUTER, _outerSide->toBson() ) ;
      if ( _innerSide )
         ob.append ( FIELD_NAME_INNER, _innerSide->toBson() ) ;
      return ob.obj() ;
   }
}
