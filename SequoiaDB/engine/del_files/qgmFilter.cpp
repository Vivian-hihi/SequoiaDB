/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = qgmFilter.cpp

   Descriptive Name =

   When/how to use: this program may be used on binary and text-formatted
   versions of PMD component. This file contains implementation for qgm
   filter

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
   _qgmOperatorFilter::_qgmOperatorFilter ( const CHAR *pAlias,
                                            const BSONObj &condition,
                                            const BSONObj &selector,
                                            INT64 numSkip,
                                            INT64 numReturn )
   {
      INT32 rc = SDB_OK ;
      _opType           = QGM_OPTYPE_FILTER ;
      _condition        = condition ;
      _sourceSelector   = selector ;
      _numSkip          = numSkip ;
      _numReturn        = numReturn ;
      _currentSkip      = 0 ;
      _currentReturn    = 0 ;
      _finalCondition   = _condition ;
      _existSelector    = FALSE ;
      _alias            = ossStrdup ( pAlias ) ;
      PD_CHECK ( _alias, SDB_OOM, error, PDERROR,
                 "Failed to allocate memory for alias" ) ;
      _isConstructed    = TRUE ;
   done :
      return ;
   error :
      goto done ;
   }

   _qgmOperatorFilter::~_qgmOperatorFilter ()
   {
   }

   INT32 _qgmOperatorFilter::pushDownPredicates ( const BSONObj &pred )
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
   INT32 _qgmOperatorFilter::execute ( _pmdEDUCB *eduCB )
   {
      INT32 rc   = SDB_OK ;
      _eduCB     = eduCB ;
      // get role for coord or data
      SDB_ASSERT ( _inputLegs.size() == 1, "1 leg is expected" )
      _qgmOperatorBase *op = _inputLegs[0] ;
      // for scan type, basically we have to construct a buffer to simulate user
      // query request and then call rtn components. Because in coordinator it
      // simply bypass the request to the required partitions
      PD_CHECK ( _isConstructed, SDB_SYS, error, PDERROR,
                 "Scan was not properly constructed" ) ;
      _currentSkip = 0 ;
      _currentReturn = 0 ;
      if ( !_sourceSelector.isEmpty () )
      {
         SDB_ASSERT ( op->getAlias(), "op alias can't be NULL" )
         rc = _selector.loadPattern ( _sourceSelector, op->getAlias(), "" ) ;
         PD_RC_CHECK ( rc, PDERROR,
                       "Failed to load selector pattern, rc = %d", rc ) ;
         _existSelector = TRUE ;
      }
      _matcher.clear() ;
      rc = _matcher.loadPattern ( _finalCondition ) ;
      PD_RC_CHECK ( rc, PDERROR, "Failed to load matcher pattern, rc = %d",
                    rc ) ;
      rc = op->execute ( _eduCB ) ;
      PD_RC_CHECK ( rc, PDERROR, "Failed to execute sub op, rc = %d", rc ) ;
      _hasExecuted = TRUE ;
   done :
      return rc ;
   error :
      close () ;
      goto done ;
   }
   // fetch next record, non-block operation
   INT32 _qgmOperatorFilter::fetchNext ( BSONObj &obj )
   {
      INT32 rc = SDB_OK ;
      BOOLEAN r ;
      BSONObj dummy ;
      BSONObj result ;
      SDB_ASSERT ( _inputLegs.size() == 1, "1 leg is expected" )
      _qgmOperatorBase *op = _inputLegs[0] ;

      PD_CHECK  ( _hasExecuted, SDB_SYS, error, PDERROR,
                  "Scan was not properly executed" ) ;
      // if we already read enough records, let's close it
      if ( _numReturn >= 0 && _currentReturn >= _numReturn )
      {
         close () ;
         rc = SDB_DMS_EOC ;
         goto error ;
      }
   retry :
      // get next record
      rc = op->fetchNext ( result ) ;
      if ( rc )
      {
         PD_CHECK ( SDB_DMS_EOC == rc, rc, error, PDERROR,
                    "Failed to fetch next, rc = %d", rc ) ;
         goto error ;
      }
      // make sure it matches our filter condition
      if ( !_finalCondition.isEmpty() )
      {
         rc = _matcher.matches ( result, r ) ;
         PD_RC_CHECK ( rc, PDERROR, "Failed to match record, rc = %d", rc ) ;
         if ( !r )
           goto retry ;
      }
      // skip the record if needed
      if ( _currentSkip < _numSkip )
      {
         ++_currentSkip ;
         goto retry ;
      }
      if ( _existSelector )
      {
         // pickup the fields
         rc = _selector.select ( result, dummy, obj ) ;
         PD_RC_CHECK ( rc, PDERROR, "Failed to select, rc = %d", rc  ) ;
      }
      else
      {
         obj = result ;
      }
      // mark the num return
      ++_currentReturn ;
   done :
      return rc ;
   error :
      close () ;
      goto done ;
   }

   INT32 _qgmOperatorFilter::fromBson ( const BSONObj &plan,
                                        _qgmOperatorBase **output )
   {
      INT32 rc = SDB_OK ;
      BSONObj dummy ;
      qgmOperatorBase *dataSource = NULL ;
      SDB_ASSERT ( output, "output can't be NULL" )
      *output = NULL ;
      // required for sanity check
      BSONElement eleType       = plan.getField ( FIELD_NAME_TYPE ) ;
      // required for alias
      BSONElement eleAlias      = plan.getField ( FIELD_NAME_ALIAS ) ;
      // optional condition
      BSONElement eleCondition  = plan.getField ( FIELD_NAME_CONDITION ) ;
      // optional selector
      BSONElement eleSelector   = plan.getField ( FIELD_NAME_SELECTOR ) ;
      // optional skip
      BSONElement eleSkip       = plan.getField ( FIELD_NAME_SKIP ) ;
      // optional return
      BSONElement eleReturn     = plan.getField ( FIELD_NAME_RETURN ) ;
      // required source
      BSONElement eleSource     = plan.getField ( FIELD_NAME_SOURCE ) ;

      // sanity check for required operators
      PD_CHECK ( eleType.type()       == NumberInt &&
                 eleAlias.type()      == String &&
                 eleSource.type()     == Object,
                 SDB_INVALIDARG, error, PDERROR,
                 "Invalid type or alias: %s",
                 plan.toString().c_str() ) ;
      PD_CHECK ( eleType.numberInt() == QGM_OPTYPE_FILTER,
                 SDB_INVALIDARG, error, PDERROR,
                 "Invalid operator type, expects: %d, real %d, from %s",
                 QGM_OPTYPE_SCAN, eleType.numberInt(),
                 plan.toString().c_str() ) ;
      // generate data source
      rc = _qgmOperatorBase::fromBson ( eleSource.embeddedObject(),
                                        &dataSource ) ;
      PD_RC_CHECK ( rc, PDERROR, "Failed to generate qgm operator from %s",
                    eleSource.embeddedObject().toString().c_str() ) ;
      SDB_ASSERT ( dataSource, "data source can't be NULL here" )
      // allocate scan operator
      *output = SDB_OSS_NEW _qgmOperatorFilter ( eleAlias.valuestr(),
                                                 eleCondition.type() == Object ?
                                                  eleCondition.embeddedObject():
                                                  dummy,
                                                 eleSelector.type() == Object ?
                                                  eleSelector.embeddedObject():
                                                  dummy,
                                               eleSkip.isNumber()?
                                                eleSkip.numberLong():0,
                                               eleReturn.isNumber()?
                                                eleReturn.numberLong():-1 ) ;

      PD_CHECK ( *output, SDB_OOM, error, PDERROR,
                 "Failed to create scan operator" ) ;
      // assign data source
      rc = (*output)->addInput ( dataSource ) ;
      PD_RC_CHECK ( rc, PDERROR, "Failed to add input, rc = %d", rc ) ;
      dataSource = NULL ;
   done :
      return rc ;
   error :
      if ( *output )
      {
         SDB_OSS_DEL ( *output ) ;
         *output = NULL ;
      }
      if ( dataSource )
      {
         SDB_OSS_DEL ( dataSource ) ;
         dataSource = NULL ;
      }
      goto done ;

   }
   std::string _qgmOperatorFilter::toString ( INT32 &id )
   {
      std::stringstream out ;
      out << id << ") " << FIELD_NAME_FILTER << endl ;
      ++id ;
      out << "\t" << FIELD_NAME_ALIAS << "\t\t: " <<
              _alias << endl ;
      out << "\t" << FIELD_NAME_CONDITION << "\t: " <<
             _condition.toString() << endl ;
      out << "\t" << FIELD_NAME_SELECTOR << "\t: " <<
             _sourceSelector.toString() << endl ;
      out << "\t" << FIELD_NAME_SKIP << "\t\t: " <<
             _numSkip << endl ;
      out << "\t" << FIELD_NAME_RETURN << "\t\t: " <<
             _numReturn << endl ;
      if ( _inputLegs.size() == 1 )
      {
         out << "\n" ;
         out << _inputLegs[0]->toString ( id ) ;
      }
      return out.str() ;
   }
   BSONObj _qgmOperatorFilter::toBson ()
   {
      BSONObjBuilder ob ;
      ob.append ( FIELD_NAME_TYPE, (INT32)_opType ) ;
      ob.append ( FIELD_NAME_ALIAS, _alias ) ;
      ob.append ( FIELD_NAME_CONDITION, _condition ) ;
      ob.append ( FIELD_NAME_SELECTOR, _sourceSelector ) ;
      ob.append ( FIELD_NAME_SKIP, _numSkip ) ;
      ob.append ( FIELD_NAME_RETURN, _numReturn ) ;
      if ( _inputLegs.size() == 1 )
         ob.append ( FIELD_NAME_SOURCE, _inputLegs[0]->toBson() ) ;
      return ob.obj() ;
   }
}
