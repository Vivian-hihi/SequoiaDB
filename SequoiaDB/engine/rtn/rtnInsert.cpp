/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = rtnInsert.cpp

   Descriptive Name = Runtime Insert

   When/how to use: this program may be used on binary and text-formatted
   versions of Runtime component. This file contains Runtime code for insert
   request.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          09/14/2012  TW  Initial Draft

   Last Changed =

*******************************************************************************/
#include "rtn.hpp"
#include "dmsStorageUnit.hpp"
#include "ossTypes.hpp"
#include "msgMessage.hpp"
#include "pmd.hpp"
#include "pmdCB.hpp"
#include "pdTrace.hpp"
#include "rtnTrace.hpp"

namespace engine
{
   // PD_TRACE_DECLARE_FUNCTION ( SDB_RTNINSERT1, "rtnInsert" )
   INT32 rtnInsert ( const CHAR *pCollectionName, BSONObj &objs, INT32 objNum,
                     INT32 flags, pmdEDUCB *cb )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB_RTNINSERT1 ) ;
      pmdKRCB *krcb = pmdGetKRCB () ;
      SDB_DMSCB *dmsCB = krcb->getDMSCB () ;
      SDB_DPSCB *dpsCB = krcb->getDPSCB () ;
      if ( krcb->getDBRole() == SDB_ROLE_STANDALONE )
      {
         dpsCB = NULL ;
      }
      else if ( !dpsCB->isLogLocal() )
      {
         dpsCB = NULL ;
      }
      rc = rtnInsert ( pCollectionName, objs, objNum, flags, cb,
                         dmsCB, dpsCB ) ;
      PD_TRACE_EXITRC ( SDB_RTNINSERT1, rc ) ;
      return rc ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_RTNINSERT2, "rtnInsert" )
   INT32 rtnInsert ( const CHAR *pCollectionName, BSONObj &objs, INT32 objNum,
                     INT32 flags, pmdEDUCB *cb, SDB_DMSCB *dmsCB,
                     SDB_DPSCB *dpsCB, INT16 w )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB_RTNINSERT2 ) ;
      SDB_ASSERT ( pCollectionName, "collection name can't be NULL" )
      SDB_ASSERT ( cb, "educb can't be NULL" )
      SDB_ASSERT ( dmsCB, "dmsCB can't be NULL" )
      dmsStorageUnit *su = NULL ;
      dmsStorageUnitID suID = DMS_INVALID_CS ;
      const CHAR *pCollectionShortName = NULL ;
      BOOLEAN writable = FALSE;

      ossValuePtr pDataPos = 0 ;
      rc = dmsCB->writable( cb ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Database is not writable, rc = %d", rc ) ;
         goto error;
      }
      writable = TRUE;

      rc = rtnResolveCollectionNameAndLock ( pCollectionName, dmsCB, &su,
                                             &pCollectionShortName, suID ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to resolve collection name %s",
                  pCollectionName ) ;
         goto error ;
      }

      if ( objs.isEmpty () )
      {
         PD_LOG ( PDERROR, "Insert record can't be empty" ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }
      pDataPos = (ossValuePtr)objs.objdata() ;
      for ( INT32 i=0; i<objNum; i++ )
      {
         try
         {
            BSONObj record ( (const CHAR*)pDataPos ) ;
            rc = su->insertRecord ( pCollectionShortName, record, cb, dpsCB ) ;
            // check return code
            if ( rc )
            {
               // if we want to skip duplicate key error
               if ( ( SDB_IXM_DUP_KEY == rc ) &&
                    ( FLG_INSERT_CONTONDUP & flags ) )
               {
                  rc = SDB_OK ;
               }
               else
               {
                  PD_LOG ( PDERROR, "Failed to insert record %s into "
                           "collection: %s, rc: %d", record.toString().c_str(),
                           pCollectionName, rc ) ;
                  goto error ;
               }
            }
            pDataPos += ossAlignX ( (ossValuePtr)record.objsize(), 4 ) ;
         }
         catch ( std::exception &e )
         {
            pdLog ( PDERROR, __FUNC__, __FILE__, __LINE__,
                    "Failed to convert to BSON and insert to collection: %s",
                    e.what() ) ;
            rc = SDB_INVALIDARG ;
            goto error ;
         }

      }
   done :
      if ( DMS_INVALID_CS != suID )
         dmsCB->suUnlock ( suID ) ;

      if ( writable )
      {
         dmsCB->writeDown();
      }
      if ( cb )
      {
         if ( SDB_OK == rc && dpsCB && 0 != cb->getLsnCount () && w > 1
            && pmdGetKRCB()->getDBRole() != SDB_ROLE_STANDALONE )
         {
            rc = pmdGetKRCB()->getReplCB()->sync( cb->getEndLsn(),cb, w ) ;
         }
         cb->resetLsn () ;
      }
      PD_TRACE_EXITRC ( SDB_RTNINSERT2, rc ) ;
      return rc ;
   error :
      goto done ;
   }
}
