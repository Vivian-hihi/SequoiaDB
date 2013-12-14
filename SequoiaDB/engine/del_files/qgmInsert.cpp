/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = qgmInsert.cpp

   Descriptive Name =

   When/how to use: this program may be used on binary and text-formatted
   versions of PMD component. This file contains implementation for qgm insert
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
#include "rtnCoordInsert.hpp"
#include "pmd.hpp"
#include "rtn.hpp"
#include "msgMessage.hpp"
namespace engine
{
   _qgmOperatorInsert::_qgmOperatorInsert ( const CHAR *pCollectionName )
   {
      ossStrncpy ( _collectionName, pCollectionName,
                   sizeof(_collectionName) ) ;
      _opType   = QGM_OPTYPE_INSERT ;
      _isConstructed = TRUE ;
   }

   INT32 _qgmOperatorInsert::execute ( _pmdEDUCB *eduCB )
   {
      INT32 rc      = SDB_OK ;
      _eduCB        = eduCB ;
      CHAR *pMsg    = NULL ;
      UINT64 reqID  = 0 ;
      BSONObj obj ;
      // get role for coord or data
      SDB_ROLE role = pmdGetKRCB()->getDBRole() ;
      SDB_ASSERT ( _inputLegs.size() == 1, "only takes 1 leg" )
      qgmOperatorBase *input = _inputLegs[0] ;
      MsgOpReply dummyReply ;
      rtnCoordInsert insert ;
      pmdKRCB *pKrcb                   = pmdGetKRCB();
      CoordCB *pCoordcb                = pKrcb->getCoordCB();
      netMultiRouteAgent *pRouteAgent  = pCoordcb->getRouteAgent();
      INT32 bufSize                    = 0 ;
      // execute data provider
      rc = input->execute( _eduCB ) ;
      PD_RC_CHECK ( rc, PDERROR, "Failed to execute subop, rc = %d", rc ) ;
      while ( TRUE )
      {
         // get each record from input
         rc = input->fetchNext ( obj ) ;
         // if input is EOC, let's jump out
         if ( SDB_DMS_EOC == rc )
         {
            rc = SDB_OK ;
            break ;
         }
         // rc sanity check
         PD_RC_CHECK ( rc, PDERROR, "Failed to fetch next, rc = %d", rc ) ;
         // call different API based on role
         if ( SDB_ROLE_COORD == role )
         {
            // build new request
            reqID = pRouteAgent->reqIDNew() ;
            // TODO: we should be smarter later. Instead of keep allocate + free
            // memory, we should attempt to allocate a buffer and hold as much
            // data as possible for one shot
            rc = msgBuildInsertMsg ( &pMsg, &bufSize, _collectionName, 0, reqID,
                                     &obj ) ;
            PD_RC_CHECK ( rc, PDERROR, "Failed to build insert message, "
                          "rc = %d", rc ) ;
            rc = insert.execute ( pMsg, *(SINT32*)pMsg, NULL, _eduCB,
                                  dummyReply ) ;
            PD_RC_CHECK ( rc, PDERROR, "Failed to execute insert on coord, "
                          "rc = %d", rc ) ;
            SDB_OSS_FREE ( pMsg ) ;
            pMsg = NULL ;
         }
         else
         {
            rc = rtnInsert ( _collectionName, obj, 1, 0, _eduCB ) ;
            PD_RC_CHECK ( rc, PDERROR,
                          "Failed to insert on non-coord, rc = %d", rc ) ;
         }
      }
      _hasExecuted = TRUE ;
   done :
      if ( pMsg )
      {
         SDB_OSS_FREE ( pMsg ) ;
      }
      return rc ;
   error :
      goto done ;
   }
   INT32 _qgmOperatorInsert::fromBson ( const BSONObj &plan,
                                        _qgmOperatorBase **output )
   {
      INT32 rc = SDB_OK ;
      SDB_ASSERT ( output, "output can't be NULL" ) ;
      *output                   = NULL ;
      qgmOperatorBase *dataSource = NULL ;
      // required for sanity check
      BSONElement eleType       = plan.getField ( FIELD_NAME_TYPE ) ;
      // required for full collection name
      BSONElement eleCollection = plan.getField ( FIELD_NAME_COLLECTION ) ;
      // required for insert input data source
      BSONElement eleSource     = plan.getField ( FIELD_NAME_SOURCE ) ;
      // sanity check for type, collection and source
      PD_CHECK ( eleType.type()       == NumberInt &&
                 eleCollection.type() == String &&
                 eleSource.type()     == Object,
                 SDB_INVALIDARG, error, PDERROR,
                 "Invalid type, collection or source: %s",
                 plan.toString().c_str() ) ;
      // sanity check for type
      PD_CHECK ( eleType.numberInt() == QGM_OPTYPE_INSERT,
                 SDB_INVALIDARG, error, PDERROR,
                 "Invalid operator type, expects: %d, real %d, from %s",
                 QGM_OPTYPE_INSERT, eleType.numberInt(),
                 plan.toString().c_str() ) ;
      // generate data source
      rc = _qgmOperatorBase::fromBson ( eleSource.embeddedObject(),
                                        &dataSource ) ;
      PD_RC_CHECK ( rc, PDERROR, "Failed to generate qgm operator from %s",
                    eleSource.embeddedObject().toString().c_str() ) ;
      SDB_ASSERT ( dataSource, "data source can't be NULL here" )
      // allocate insert operator
      *output = SDB_OSS_NEW _qgmOperatorInsert ( eleCollection.valuestr() ) ;
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
   std::string _qgmOperatorInsert::toString ( INT32 &id )
   {
      std::stringstream out ;
      out << id << ") " << FIELD_NAME_INSERT << endl ;
      ++id ;
      out << "\t" << FIELD_NAME_COLLECTION << "\t: " <<
              _collectionName<< endl ;
      if ( _inputLegs.size() == 1 )
      {
         out << "\n" ;
         out << _inputLegs[0]->toString ( id ) ;
      }
      return out.str() ;
   }
   BSONObj _qgmOperatorInsert::toBson ()
   {
      BSONObjBuilder ob ;
      ob.append ( FIELD_NAME_TYPE, (INT32)_opType ) ;
      ob.append ( FIELD_NAME_COLLECTION, _collectionName ) ;
      if ( _inputLegs.size() == 1 )
         ob.append ( FIELD_NAME_SOURCE, _inputLegs[0]->toBson() ) ;
      return ob.obj () ;
   }
}
