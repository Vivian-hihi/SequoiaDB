/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = qgmReturn.cpp

   Descriptive Name =

   When/how to use: this program may be used on binary and text-formatted
   versions of PMD component. This file contains implementation for qgm return
   operator

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          04/09/2013  TW  Initial Draft

   Last Changed =

*******************************************************************************/
#include "core.hpp"
#include "qgm.hpp"
#include "pd.hpp"
#include "pmd.hpp"
#include "rtn.hpp"
namespace engine
{
   _qgmOperatorReturn::_qgmOperatorReturn ( INT64 numSkip,
                                            INT64 numReturn )
   {
      _opType        = QGM_OPTYPE_RETURN ;
      _input         = NULL ;
      _numSkip       = numSkip ;
      _numReturn     = numReturn ;
      _currentSkip   = 0 ;
      _currentReturn = 0 ;
      _isConstructed = TRUE ;
   }

   INT32 _qgmOperatorReturn::execute ( _pmdEDUCB *eduCB )
   {
      INT32 rc       = SDB_OK ;
      _eduCB         = eduCB ;
      // get role for coord or data
      SDB_ASSERT ( _inputLegs.size() == 1, "only takes 1 leg" )
      _currentSkip   = 0 ;
      _currentReturn = 0 ;
      _input = _inputLegs[0] ;
      rc = _input->execute(_eduCB) ;
      PD_RC_CHECK ( rc, PDERROR, "Failed to execute subop, rc = %d", rc ) ;
      _hasExecuted = TRUE ;
   done :
      return rc ;
   error :
      goto done ;
   }

   INT32 _qgmOperatorReturn::fetchNext ( BSONObj &obj )
   {
      INT32 rc = SDB_OK ;
      SDB_ASSERT ( _input, "input can't be NULL" )
      // if user specified numreturn, and if the current returned is same or
      // more than expected, let's return EOC
      if ( _numReturn >= 0 && _currentReturn >= _numReturn )
      {
         close () ;
         rc = SDB_DMS_EOC ;
         goto error ;
      }
      // skip whatever users want
      while ( _currentSkip < _numSkip )
      {
         rc = _input->fetchNext ( obj ) ;
         if ( rc )
         {
            PD_CHECK ( SDB_DMS_EOC != rc, rc, error, PDERROR,
                       "Failed to fetch next, rc = %d", rc ) ;
            goto error ;
         }
         _currentSkip ++ ;
      }
      // get one record
      rc = _input->fetchNext ( obj ) ;
      _currentReturn ++ ;
      if ( rc )
      {
         PD_CHECK ( SDB_DMS_EOC == rc, rc, error, PDERROR,
                    "Failed to fetch next, rc = %d", rc ) ;
         goto error ;
      }
   done :
      return rc ;
   error :
      goto done ;
   }

   INT32 _qgmOperatorReturn::fromBson ( const BSONObj &plan,
                                        qgmOperatorBase **output )
   {
      INT32 rc = SDB_OK ;
      SDB_ASSERT ( output, "output can't be NULL" )
      *output = NULL ;
      qgmOperatorBase *dataSource = NULL ;
      // required for sniaty check
      BSONElement eleType       = plan.getField ( FIELD_NAME_TYPE ) ;
      // optional
      BSONElement eleSkip       = plan.getField ( FIELD_NAME_SKIP ) ;
      // optional
      BSONElement eleReturn     = plan.getField ( FIELD_NAME_RETURN ) ;
      // required for source
      BSONElement eleSource     = plan.getField ( FIELD_NAME_SOURCE ) ;
      // sanity check for type and source
      PD_CHECK ( eleType.type() == NumberInt &&
                 eleSource.type() == Object,
                 SDB_INVALIDARG, error, PDERROR,
                 "Invalid type or source: %s",
                 plan.toString().c_str() ) ;
      // sanity check for type
      PD_CHECK ( eleType.numberInt() == QGM_OPTYPE_RETURN,
                 SDB_INVALIDARG, error, PDERROR,
                 "Invalid operator type, expects: %d, real %d, from %s",
                 QGM_OPTYPE_RETURN, eleType.numberInt(),
                 plan.toString().c_str() ) ;
      // generate data source
      rc = _qgmOperatorBase::fromBson ( eleSource.embeddedObject(),
                                        &dataSource ) ;
      PD_RC_CHECK ( rc, PDERROR, "Failed to generate qgm operator from %s",
                    eleSource.embeddedObject().toString().c_str() ) ;
      SDB_ASSERT ( dataSource, "data source can't be NULL here" )
      // memory will be freed by caller or in qgmOperatorBase destructor
      *output = SDB_OSS_NEW qgmOperatorReturn ( eleSkip.isNumber()?
                                                 eleSkip.numberLong():
                                                 0,
                                                eleReturn.isNumber()?
                                                 eleReturn.numberLong():
                                                 -1 ) ;
      PD_CHECK ( *output, SDB_OOM, error, PDERROR,
                 "Failed to allocate return operator" ) ;
      // assign data source
      rc = (*output)->addInput ( dataSource ) ;
      PD_RC_CHECK ( rc, PDERROR, "Failed to add input, rc = %d", rc ) ;
      dataSource = NULL ;
   done :
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
   std::string _qgmOperatorReturn::toString ( INT32 &id )
   {
      std::stringstream out ;
      out << id << ") " << FIELD_NAME_RETURN << endl ;
      ++id ;
      out << "\t" << FIELD_NAME_SKIP << "\t\t: " <<
              _numSkip << endl ;
      out << "\t" << FIELD_NAME_RETURN << "\t\t: " <<
              _numReturn << endl ;
      if ( _inputLegs.size() == 1 )
      {
         out << endl ;
         out << _inputLegs[0]->toString ( id ) ;
      }
      return out.str() ;
   }

   BSONObj _qgmOperatorReturn::toBson ()
   {
      BSONObjBuilder ob ;
      ob.append ( FIELD_NAME_TYPE, (INT32)_opType ) ;
      ob.append ( FIELD_NAME_SKIP, _numSkip ) ;
      ob.append ( FIELD_NAME_RETURN, _numReturn ) ;
      if ( _inputLegs.size() == 1 )
         ob.append ( FIELD_NAME_SOURCE, _inputLegs[0]->toBson() ) ;
      return ob.obj () ;
   }
}
