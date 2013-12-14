/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = qgmRecord.cpp

   Descriptive Name =

   When/how to use: this program may be used on binary and text-formatted
   versions of PMD component. This file contains implementation for qgm record
   operator

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          04/15/2013  TW  Initial Draft

   Last Changed =

*******************************************************************************/
#include "core.hpp"
#include "qgm.hpp"
#include "pd.hpp"
#include "pmd.hpp"
#include "rtn.hpp"
namespace engine
{
   _qgmOperatorRecord::_qgmOperatorRecord( const CHAR *pAlias )
   {
      INT32 rc          = SDB_OK ;
      _eduCB            = NULL ;
      _currentPos       = 0 ;
      _opType           = QGM_OPTYPE_RECORD ;
      _alias            = ossStrdup ( pAlias ) ;
      PD_CHECK ( _alias, SDB_OOM, error, PDERROR,
                 "Failed to allocate memory for alias" ) ;
      _isConstructed    = TRUE ;
   done :
      return ;
   error :
      goto done ;
   }
   _qgmOperatorRecord::~_qgmOperatorRecord()
   {
   }
   INT32 _qgmOperatorRecord::execute ( _pmdEDUCB *eduCB )
   {
      _eduCB = eduCB ;
      _currentPos = 0 ;
      _hasExecuted = TRUE ;
      return SDB_OK ;
   }

   INT32 _qgmOperatorRecord::fetchNext ( BSONObj &obj )
   {
      INT32 rc = SDB_OK ;
      SDB_ASSERT ( _inputLegs.size() == 0, "no input legs are expected" )
      SDB_ASSERT ( _hasExecuted, "operator must be executed first" )
      if ( _currentPos < _recordList.size() )
      {
         obj = _recordList[_currentPos] ;
         ++_currentPos ;
      }
      else
      {
         rc = SDB_DMS_EOC ;
         goto error ;
      }
   done :
      return rc ;
   error :
      goto done ;
   }

   INT32 _qgmOperatorRecord::addRecord ( const BSONObj &record )
   {
      INT32 rc = SDB_OK ;
      PD_CHECK ( _recordList.size() < QGM_OPERATOR_RECORD_MAX,
                 SDB_QGM_MAX_NUM_RECORD, error, PDERROR,
                 "There are maximum %d records to be inserted at once:",
                 QGM_OPERATOR_RECORD_MAX ) ;
      _recordList.push_back ( record ) ;
   done :
      return rc ;
   error :
      goto done ;
   }

   INT32 _qgmOperatorRecord::fromBson ( const BSONObj &plan,
                                        qgmOperatorBase **output )
   {
      INT32 rc = SDB_OK ;
      SDB_ASSERT ( output, "output can't be NULL" )
      *output = NULL ;
      // required for sniaty check
      BSONElement eleType       = plan.getField ( FIELD_NAME_TYPE ) ;
      // required for records
      BSONElement eleRecords    = plan.getField ( FIELD_NAME_RECORDS ) ;
      // required for alias
      BSONElement eleAlias      = plan.getField ( FIELD_NAME_ALIAS ) ;
      // sanity check for type and source
      PD_CHECK ( eleType.type() == NumberInt &&
                 eleRecords.type() == Array &&
                 eleAlias.type()   == String,
                 SDB_INVALIDARG, error, PDERROR,
                 "Invalid type, alias or records: %s",
                 plan.toString().c_str() ) ;
      // sanity check for type
      PD_CHECK ( eleType.numberInt() == QGM_OPTYPE_RECORD,
                 SDB_INVALIDARG, error, PDERROR,
                 "Invalid operator type, expects: %d, real %d, from %s",
                 QGM_OPTYPE_RETURN, eleType.numberInt(),
                 plan.toString().c_str() ) ;
      // memory will be freed by caller or in qgmOperatorBase destructor
      *output = SDB_OSS_NEW qgmOperatorRecord ( eleAlias.valuestr() ) ;
      PD_CHECK ( *output, SDB_OOM, error, PDERROR,
                 "Failed to allocate return operator" ) ;
      // extract records
      {
         BSONObjIterator it ( eleRecords.embeddedObject() ) ;
         while ( it.more () )
         {
            BSONElement ele = it.next() ;
            PD_CHECK ( ele.type() == Object, SDB_INVALIDARG, error, PDERROR,
                       "element in records must be object" ) ;
            rc = ((qgmOperatorRecord*)(*output))->addRecord
                  ( ele.embeddedObject() ) ;
            PD_RC_CHECK ( rc, PDERROR, "Failed to add record, rc = %d", rc ) ;
         }
      }
   done :
      return rc ;
   error :
      if ( *output )
      {
         SDB_OSS_DEL (*output) ;
         *output = NULL ;
      }
      goto done ;
   }

   std::string _qgmOperatorRecord::toString ( INT32 &id )
   {
      std::stringstream out ;
      out << id << ") " << FIELD_NAME_RECORD << endl ;
      ++id ;
      out << "\t" << FIELD_NAME_ALIAS << "\t: " << _alias << endl ;
      out << "\t" << FIELD_NAME_RECORDS << endl ;
      INT32 count = 0 ;
      vector<BSONObj>::iterator it ;
      for ( it = _recordList.begin(); it != _recordList.end(); ++it )
      {
         out << "\t\t" << count << ")" << endl ;
         out << "\t\t" << (*it).toString() << endl ;
      }
      return out.str() ;
   }

   BSONObj _qgmOperatorRecord::toBson ()
   {
      BSONObjBuilder ob ;
      BSONArrayBuilder ar ;
      vector<BSONObj>::iterator it ;
      ob.append ( FIELD_NAME_TYPE, (INT32)_opType ) ;
      ob.append ( FIELD_NAME_ALIAS, _alias ) ;
      for ( it = _recordList.begin(); it != _recordList.end(); ++it )
      {
         ar.append ( *it ) ;
      }
      ob.append ( FIELD_NAME_RECORDS, ar.arr() ) ;
      return ob.obj () ;
   }
}
