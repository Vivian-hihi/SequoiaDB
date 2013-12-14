/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = qgmUpdate.cpp

   Descriptive Name =

   When/how to use: this program may be used on binary and text-formatted
   versions of PMD component. This file contains implementation for qgm delete
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
#include "rtnCoordDelete.hpp"
#include "pmd.hpp"
#include "rtn.hpp"
#include "msgMessage.hpp"
#include "msgDef.hpp"
#include <sstream>

namespace engine
{
   _qgmOperatorDelete::_qgmOperatorDelete ( const CHAR *pCollectionName,
                                            const BSONObj &condition,
                                            const BSONObj &hint )
   {
      ossStrncpy ( _collectionName, pCollectionName,
                   sizeof(_collectionName) ) ;
      _opType     = QGM_OPTYPE_DELETE ;
      _condition  = condition.copy () ;
      _hint       = hint.copy () ;
      _isConstructed = TRUE ;
   }

   INT32 _qgmOperatorDelete::execute ( _pmdEDUCB *eduCB )
   {
      INT32 rc      = SDB_OK ;
      _eduCB        = eduCB ;
      // get role for coord or data
      SDB_ROLE role = pmdGetKRCB()->getDBRole() ;
      CHAR *pMsg    = NULL ;
      if ( SDB_ROLE_COORD == role )
      {
         // for coordinator, we have to build update request since
         // rtnCoordUpdate is going to simply forward the request to data nodes
         INT32 bufSize                    = 0 ;
         pmdKRCB *pKrcb                   = pmdGetKRCB();
         CoordCB *pCoordcb                = pKrcb->getCoordCB();
         netMultiRouteAgent *pRouteAgent  = pCoordcb->getRouteAgent();
         UINT64 reqID                     = pRouteAgent->reqIDNew();
         rtnCoordDelete coordDelete ;
         MsgOpReply dummyReply ;
         rc = msgBuildDeleteMsg ( &pMsg, &bufSize, _collectionName, 0,
                                  reqID, &_condition, &_hint ) ;
         PD_RC_CHECK ( rc, PDERROR,
                       "Failed to delete on coord, rc = %d", rc ) ;
         rc = coordDelete.execute ( pMsg, *(SINT32*)pMsg, NULL, _eduCB,
                                    dummyReply ) ;
         PD_RC_CHECK ( rc, PDERROR,
                       "Failed to execute coordUpdate, rc = %d", rc ) ;
      }
      else
      {
         rc = rtnDelete ( _collectionName, _condition, _hint,
                          0, _eduCB ) ;
         PD_RC_CHECK ( rc, PDERROR,
                       "Failed to update on non-coord, rc = %d", rc ) ;
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

   INT32 _qgmOperatorDelete::fromBson ( const BSONObj &plan,
                                        qgmOperatorBase **output )
   {
      INT32 rc = SDB_OK ;
      BSONObj dummy ;
      SDB_ASSERT ( output, "output can't be NULL" )
      *output = NULL ;
      // required for sanity check
      BSONElement eleType       = plan.getField ( FIELD_NAME_TYPE ) ;
      // required for full collection name
      BSONElement eleCollection = plan.getField ( FIELD_NAME_COLLECTION ) ;
      // optional
      BSONElement eleCondition  = plan.getField ( FIELD_NAME_CONDITION ) ;
      // optional
      BSONElement eleHint       = plan.getField ( FIELD_NAME_HINT ) ;
      // sanity check for type and collection
      PD_CHECK ( eleType.type() == NumberInt &&
                 eleCollection.type() == String,
                 SDB_INVALIDARG, error, PDERROR,
                 "Invalid type or collection name: %s",
                 plan.toString().c_str() ) ;
      // sanity check for type
      PD_CHECK ( eleType.numberInt() == QGM_OPTYPE_DELETE,
                 SDB_INVALIDARG, error, PDERROR,
                 "Invalid operator type, expects: %d, real %d, from %s",
                 QGM_OPTYPE_DELETE, eleType.numberInt(),
                 plan.toString().c_str() ) ;
      // memory will be freed by caller or in qgmOperatorBase destructor
      *output = SDB_OSS_NEW qgmOperatorDelete ( eleCollection.valuestr(),
                                                eleCondition.type() == Object?
                                                   eleCondition.embeddedObject():
                                                   dummy,
                                                eleHint.type() == Object?
                                                   eleHint.embeddedObject():
                                                   dummy ) ;
      PD_CHECK ( *output, SDB_OOM, error, PDERROR,
                 "Failed to allocate delete operator" ) ;
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

   std::string _qgmOperatorDelete::toString ( INT32 &id )
   {
      std::stringstream out ;
      out << id << ") " << FIELD_NAME_DELETE << endl ;
      ++id ;
      out << "\t" << FIELD_NAME_COLLECTION << "\t: " <<
             _collectionName << endl ;
      out << "\t" << FIELD_NAME_CONDITION << "\t: " <<
             _condition.toString() << endl ;
      out << "\t" << FIELD_NAME_HINT << "\t\t: " <<
             _hint.toString() << endl ;
      return out.str() ;
   }

   BSONObj _qgmOperatorDelete::toBson ()
   {
      BSONObjBuilder ob ;
      ob.append ( FIELD_NAME_TYPE, (INT32)_opType ) ;
      ob.append ( FIELD_NAME_COLLECTION, _collectionName ) ;
      ob.append ( FIELD_NAME_CONDITION, _condition ) ;
      ob.append ( FIELD_NAME_HINT, _hint ) ;
      return ob.obj () ;
   }
}
