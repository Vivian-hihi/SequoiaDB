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
   extern BSONObj _qgmExistsPredicate ;
   extern BSONObj _qgmNotExistsPredicate ;

   _qgmOperatorNLJoin::_qgmOperatorNLJoin ( const CHAR *pAlias,
                                            const BSONObj &selector,
                                            const BSONObj &hint,
                                            _qgmJoinType joinType,
                                            BOOLEAN earlyOut,
                                            INT64 numSkip,
                                            INT64 numReturn )
   {
      INT32 rc          = SDB_OK ;
      _opType           = QGM_OPTYPE_NLJOIN ;
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
      _alias            = ossStrdup ( pAlias ) ;
      PD_CHECK ( _alias, SDB_OOM, error, PDERROR,
                 "Failed to allocate memory for alias" ) ;
      SDB_ASSERT ( _joinType != QGM_JNTYPE_RIGHT &&
                   _joinType != QGM_JNTYPE_FULL,
                   "NLJoin does not support right or full join" )
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
      _outerAlias       = NULL ;
      _innerAlias       = NULL ;
      _isConstructed    = TRUE ;
   done :
      return ;
   error :
      goto done ;
   }
   _qgmOperatorNLJoin::~_qgmOperatorNLJoin()
   {
   }

   INT32 _qgmOperatorNLJoin::execute ( _pmdEDUCB *eduCB )
   {
      INT32 rc      = SDB_OK ;
      _eduCB        = eduCB ;
      PD_CHECK ( _isConstructed, SDB_SYS, error, PDERROR,
                 "NLJoin was not properly constructed" ) ;
      // get role for coord or data
      SDB_ASSERT ( _inputLegs.size() == 2, "2 legs are expected" )
      _outerSide = _inputLegs[0] ;
      _innerSide = _inputLegs[1] ;
      _currentSkip = 0 ;
      _currentReturn = 0 ;
      _outerAlias = _outerSide->getAlias () ;
      _innerAlias = _innerSide->getAlias () ;
      SDB_ASSERT ( _outerAlias && _innerAlias,
                   "outer and inner alias can't be NULL" )
      // build merge selector
      if ( !_selectorObj.isEmpty () )
      {
         rc = _selector.loadPattern ( _selectorObj, _outerAlias,
                                      _innerAlias ) ;
         PD_RC_CHECK ( rc, PDERROR,
                       "Failed to load pattern for inner, rc = %d", rc ) ;
         _existSelector = TRUE ;
      }
      // execute outerSide
      rc = _outerSide->execute( _eduCB ) ;
      PD_RC_CHECK ( rc, PDERROR, "Failed to execute outer, rc = %d", rc ) ;
      // inner side will be executed in fetchNext
      _hasExecuted = TRUE ;
   done :
      return rc ;
   error :
      goto done ;
   }

   // based on the join predicate list, let's build the search condition that we
   // are going to push to inner
   INT32 _qgmOperatorNLJoin::buildInnerSearchCondition (
         BSONObj &outerRecord,
         BSONObjBuilder &searchCondition )
   {
      INT32 rc = SDB_OK ;
      qgmJoinPredicateList::iterator it ;
      try
      {
         for ( it = _joinPredicates.begin () ;
               it != _joinPredicates.end () ;
               ++it )
         {
            // first let's find the value from outer record
            const CHAR *pFieldName = (*it)->_pFieldOuter ;
            BSONElement ele = outerRecord.getFieldDotted ( pFieldName ) ;
            // if the outer side does not have the predicate
            if ( ele.eoo() )
            {
               switch ( (*it)->_type )
               {
               case QGM_PDTYPE_NE :
                  // when it's not equal condition, let's match any records that
                  // got the field exists
                  searchCondition.append ( (*it)->_pFieldInner,
                                            _qgmExistsPredicate ) ;
                  break ;
               case QGM_PDTYPE_EQ :
                  // when it's equal condition, let's match any records that
                  // got the field not exists
                  searchCondition.append ( (*it)->_pFieldInner,
                                            _qgmNotExistsPredicate ) ;
                  break ;
               default :
                  rc = SDB_QGM_MATCH_NONE ;
                  goto error ;
               }
            } // if ( ele.eoo() )
            else
            {
               switch ( (*it)->_type )
               {
               case QGM_PDTYPE_NE :
               {
                  BSONObjBuilder ob ;
                  ob.appendAs ( ele, "$ne" ) ;
                  // when it's not equal condition, let's append the element
                  searchCondition.append ( (*it)->_pFieldInner,
                                           ob.obj() ) ;
                  break ;
               }
               case QGM_PDTYPE_EQ :
               {
                  // when it's equal condition, let's append the element
                  searchCondition.appendAs ( ele, (*it)->_pFieldInner ) ;
                  break ;
               }
               case QGM_PDTYPE_GT :
               {
                  BSONObjBuilder ob ;
                  ob.appendAs ( ele, "$lt" ) ;
                  // when it's > condition, it means outer supposed to greater
                  // than inner, so that means all inner must be lesser than the
                  // specified outer, so we buildd < condition
                  searchCondition.append ( (*it)->_pFieldInner,
                                           ob.obj() ) ;
                  break ;
               }
               case QGM_PDTYPE_GTE :
               {
                  BSONObjBuilder ob ;
                  ob.appendAs ( ele, "$lte" ) ;
                  // for >= condition, we should revert to <= for inner
                  // comparison
                  searchCondition.append ( (*it)->_pFieldInner,
                                           ob.obj() ) ;
                  break ;
               }
               case QGM_PDTYPE_LT :
               {
                  BSONObjBuilder ob ;
                  ob.appendAs ( ele, "$gt" ) ;
                  searchCondition.append ( (*it)->_pFieldInner,
                                           ob.obj() ) ;
                  break ;
               }
               case QGM_PDTYPE_LTE :
               {
                  BSONObjBuilder ob ;
                  ob.appendAs ( ele, "$gte" ) ;
                  searchCondition.append ( (*it)->_pFieldInner,
                                           ob.obj()  ) ;
                  break ;
               }
               }
            }
         } // for ( it = _joinPredicate.begin ()
      }
      catch ( std::exception &e )
      {
         PD_RC_CHECK ( SDB_SYS, PDERROR,
                       "Exception when building search cond: %s", e.what() ) ;
      }
   done :
      return rc ;
   error :
      goto done ;
   }

   // fetch next record, non-block operation
   INT32 _qgmOperatorNLJoin::fetchNext ( BSONObj &obj )
   {
      INT32 rc = SDB_OK ;
      SDB_ASSERT ( _outerSide, "outer can't be NULL" )
      SDB_ASSERT ( _innerSide, "inner can't be NULL" )
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
      // we get here if we the current inner leg got EOC in previous round, so
      // that we have to take the next record from outer and perform the inner
      // scan again
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
      // required for alias
      BSONElement eleAlias = plan.getField ( FIELD_NAME_ALIAS ) ;
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
                 eleAlias.type()         == String &&
                 eleOuter.type()         == Object &&
                 eleInner.type()         == Object,
                 SDB_INVALIDARG, error, PDERROR,
                 "Invalid type, jointype, alias, "
                 "outer or inner: %s",
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
      *output = SDB_OSS_NEW _qgmOperatorNLJoin ( eleAlias.valuestr(),
                                                 eleSelector.type() == Object?
                                                  eleSelector.embeddedObject():
                                                  dummy,
                                                 eleHint.type() == Object?
                                                  eleHint.embeddedObject():
                                                  dummy,
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

   std::string _qgmOperatorNLJoin::toString ( INT32 &id )
   {
      std::stringstream out ;
      out << id << ") " << FIELD_NAME_NLJOIN << endl ;
      ++id ;
      out << "\t" << FIELD_NAME_JOINTYPE << "\t: " ;
      switch ( _joinType )
      {
      case QGM_JNTYPE_INNER :
         out << "Inner" << endl ;
         break ;
      case QGM_JNTYPE_LEFT :
         out << "Left Outer" << endl ;
         break ;
      case QGM_JNTYPE_RIGHT :
         out << "Right Outer" << endl ;
         break ;
      case QGM_JNTYPE_FULL :
         out << "Full Outer" << endl ;
         break ;
      default :
         out << "Unknown" << endl ;
         break ;
      }
      out << "\t" << FIELD_NAME_ALIAS << "\t\t: " <<
             _alias << endl ;
      out << "\t" << FIELD_NAME_HINT << "\t\t: " <<
             _hint.toString() << endl ;
      out << "\t" << FIELD_NAME_SELECTOR << "\t: " <<
             _selectorObj.toString() << endl ;
      out << "\t" << FIELD_NAME_SKIP << "\t\t: " <<
             _numSkip << endl ;
      out << "\t" << FIELD_NAME_RETURN << "\t\t: " <<
             _numReturn << endl ;
      out << "\t" << FIELD_NAME_EARLYOUT << "\t: " <<
             _earlyOut << endl ;
      out << "\t" << FIELD_NAME_JOINPRED << endl ;
      qgmJoinPredicateList::iterator it ;
      INT32 count = 0 ;
      for ( it = _joinPredicates.begin() ;
            it != _joinPredicates.end() ;
            ++it )
      {
         out << "\t\t" << count << ")" << endl ;
         out << "\t\t" << (*it)->toBson().toString() << endl ;
      }
      out << endl ;
      if ( _outerSide )
         out << _outerSide->toString ( id ) ;
      if ( _innerSide )
         out << _innerSide->toString ( id ) ;
      return out.str() ;
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
      ob.append ( FIELD_NAME_ALIAS, _alias ) ;
      ob.append ( FIELD_NAME_HINT, _hint ) ;
      ob.append ( FIELD_NAME_SELECTOR, _selectorObj ) ;
      ob.append ( FIELD_NAME_SKIP, _numSkip ) ;
      ob.append ( FIELD_NAME_RETURN, _numReturn ) ;
      ob.appendBool ( FIELD_NAME_EARLYOUT, _earlyOut ) ;
      if ( _outerSide )
         ob.append ( FIELD_NAME_OUTER, _outerSide->toBson() ) ;
      if ( _innerSide )
         ob.append ( FIELD_NAME_INNER, _innerSide->toBson() ) ;
      return ob.obj() ;
   }
}
