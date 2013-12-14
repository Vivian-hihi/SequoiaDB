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
   versions of PMD component. This file contains implementation for qgm update
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
#include "rtnCoordUpdate.hpp"
#include "pmd.hpp"
#include "rtn.hpp"
#include "msgMessage.hpp"

namespace engine
{
   _qgmOperatorUpdate::_qgmOperatorUpdate ( const CHAR *pCollectionName,
                                            const BSONObj &condition,
                                            const BSONObj &updateRule,
                                            const BSONObj &hint )
   {
      ossStrncpy ( _collectionName, pCollectionName,
                   sizeof(_collectionName) ) ;
      _opType     = QGM_OPTYPE_UPDATE ;
      _condition  = condition.copy () ;
      _updateRule = updateRule.copy () ;
      _hint       = hint.copy () ;
      _isConstructed = TRUE ;
   }

   INT32 _qgmOperatorUpdate::execute ( _pmdEDUCB *eduCB )
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
         rtnCoordUpdate coordUpdate ;
         MsgOpReply dummyReply ;
         rc = msgBuildUpdateMsg ( &pMsg, &bufSize, _collectionName, 0,
                                  reqID, &_condition, &_updateRule,
                                  &_hint ) ;
         PD_RC_CHECK ( rc, PDERROR,
                       "Failed to update on coord, rc = %d", rc ) ;
         rc = coordUpdate.execute ( pMsg, *(SINT32*)pMsg, NULL, _eduCB,
                                    dummyReply ) ;
         PD_RC_CHECK ( rc, PDERROR,
                       "Failed to execute coordUpdate, rc = %d", rc ) ;
      }
      else
      {
         rc = rtnUpdate ( _collectionName, _condition, _updateRule, _hint,
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

   INT32 _qgmOperatorUpdate::fromBson ( const BSONObj &plan,
                                        _qgmOperatorBase **output )
   {
      INT32 rc = SDB_OK ;
      BSONObj dummy ;
      SDB_ASSERT ( output, "output can't be NULL" ) ;
      *output = NULL ;
      // required for sniaty check
      BSONElement eleType       = plan.getField ( FIELD_NAME_TYPE ) ;
      // required for full collection name
      BSONElement eleCollection = plan.getField ( FIELD_NAME_COLLECTION ) ;
      // optional
      BSONElement eleCondition  = plan.getField ( FIELD_NAME_CONDITION ) ;
      // required
      BSONElement eleRule       = plan.getField ( FIELD_NAME_RULE ) ;
      // optional
      BSONElement eleHint       = plan.getField ( FIELD_NAME_HINT ) ;
      // sanity check for type and collection
      PD_CHECK ( eleType.type()       == NumberInt &&
                 eleCollection.type() == String &&
                 eleRule.type()       == Object,
                 SDB_INVALIDARG, error, PDERROR,
                 "Invalid type or collection: %s",
                 plan.toString().c_str() ) ;
      // sanity check for type
      PD_CHECK ( eleType.numberInt() == QGM_OPTYPE_UPDATE,
                 SDB_INVALIDARG, error, PDERROR,
                 "Invalid operator type, expects: %d, real %d, from %s",
                 QGM_OPTYPE_UPDATE, eleType.numberInt(),
                 plan.toString().c_str() ) ;

      // allocate memory for new operator, free by caller or in qgmOperatorBase
      // destructor
      *output = SDB_OSS_NEW _qgmOperatorUpdate ( eleCollection.valuestr(),
                                                 eleCondition.type() == Object ?
                                                 eleCondition.embeddedObject():
                                                 dummy,
                                                 eleRule.embeddedObject(),
                                                 eleHint.type() == Object ?
                                                 eleHint.embeddedObject():
                                                 dummy ) ;
      PD_CHECK ( *output, SDB_OOM, error, PDERROR,
                 "Failed to allocate update operator" ) ;
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

   std::string _qgmOperatorUpdate::toString ( INT32 &id )
   {
      std::stringstream out ;
      out << id << ") " << FIELD_NAME_UPDATE << endl ;
      ++id ;
      out << "\t" << FIELD_NAME_COLLECTION << "\t: " <<
             _collectionName << endl ;
      out << "\t" << FIELD_NAME_CONDITION << "\t: " <<
             _condition.toString() << endl ;
      out << "\t" << FIELD_NAME_RULE << "\t\t: " <<
             _updateRule.toString() << endl ;
      out << "\t" << FIELD_NAME_HINT << "\t\t: " <<
             _hint.toString() << endl ;
      return out.str() ;
   }

   BSONObj _qgmOperatorUpdate::toBson ()
   {
      BSONObjBuilder ob ;
      ob.append ( FIELD_NAME_TYPE, (INT32)_opType ) ;
      ob.append ( FIELD_NAME_COLLECTION, _collectionName ) ;
      ob.append ( FIELD_NAME_CONDITION, _condition ) ;
      ob.append ( FIELD_NAME_RULE, _updateRule ) ;
      ob.append ( FIELD_NAME_HINT, _hint ) ;
      return ob.obj () ;
   }

}
