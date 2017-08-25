/*******************************************************************************


   Copyright (C) 2011-2017 SequoiaDB Ltd.

   This program is free software: you can redistribute it and/or modify
   it under the term of the GNU Affero General Public License, version 3,
   as published by the Free Software Foundation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warrenty of
   MARCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU Affero General Public License for more details.

   You should have received a copy of the GNU Affero General Public License
   along with this program. If not, see <http://www.gnu.org/license/>.

   Source File Name = rtnExtDataHandler.cpp

   Descriptive Name = External data process handler for rtn.

   When/how to use: this program may be used on binary and text-formatted
   versions of data management component. This file contains structure for
   DMS storage unit and its methods.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          14/04/2017  YSD Initial Draft

   Last Changed =

*******************************************************************************/
#include "pmd.hpp"
#include "rtn.hpp"
#include "rtnExtDataHandler.hpp"

#define RTN_CAPPED_CL_MAXRECNUM     0
#define RTN_FIELD_NAME_RID          "_rid"
#define RTN_FIELD_NAME_SOURCE       "_source"

namespace engine
{
   _rtnExtDataHandler::_rtnExtDataHandler()
   {
   }

   _rtnExtDataHandler::~_rtnExtDataHandler()
   {
   }

   INT32 _rtnExtDataHandler::_addOprRecord( const CHAR *name,
                                            _dmsExtOprType oprType,
                                            pmdEDUCB *cb,
                                            const bson::OID *dataOID,
                                            const BSONObj *dataObj,
                                            SDB_DPSCB *dpsCB )
   {
      INT32 rc = SDB_OK ;
      INT32 insertNum = 0 ;
      INT32 ignoreNum = 0 ;
      BSONObj objToInsert ;
      BSONObjBuilder objBuilder ;
      BOOLEAN oidRequired = FALSE ;
      BOOLEAN dataRequired = FALSE ;
      SDB_DMSCB *dmsCB = pmdGetKRCB()->getDMSCB () ;

      switch ( oprType )
      {
         case DMS_EXT_INSERT:  // insert
         case DMS_EXT_UPDATE:  // update
            oidRequired = TRUE ;
            dataRequired = TRUE ;
            break ;
         case DMS_EXT_DELETE:  // delete
            oidRequired = TRUE ;
            dataRequired = FALSE ;
            break ;
         case DMS_EXT_TRUNCATE:  // truncate
            oidRequired = FALSE ;
            dataRequired = FALSE ;
            break ;
         default:
            PD_LOG( PDERROR, "Invalid operation type[ %d ]", oprType ) ;
            rc = SDB_SYS ;
            goto error ;
      }

      if ( oidRequired && ( !dataOID || !dataOID->isSet() ) )
      {
         PD_LOG( PDERROR, "_id should be specified for current operation" ) ;
         rc = SDB_SYS ;
         goto error ;
      }

      if ( dataRequired && !dataObj )
      {
         PD_LOG( PDERROR, "Data object is NULL for operation" ) ;
         rc = SDB_SYS ;
         goto error ;
      }

      // 1. Append operation type.
      objBuilder.append( FIELD_NAME_TYPE, oprType ) ;
      // 2. Append the _id as _rid.
      if ( oidRequired )
      {
         objBuilder.append( RTN_FIELD_NAME_RID, dataOID->str().c_str() ) ;
      }

      // 3. Append data if necessarry.
      if ( dataRequired )
      {
         objBuilder.append( RTN_FIELD_NAME_SOURCE, *dataObj ) ;
      }

      objToInsert = objBuilder.done() ;

      PD_LOG( PDDEBUG, "Operation record to insert: %s",
              objToInsert.toString( FALSE, TRUE ).c_str() ) ;

      // Pass dpsCB as NULL as we don't want to write dps log. The replication
      // of external data totally relies on the original data.
      rc = rtnInsert( name, objToInsert, 1, 0,
                      cb, dmsCB, NULL, 1, &insertNum, &ignoreNum ) ;
      PD_RC_CHECK( rc, PDERROR, "Insert record failed[ %d ]", rc ) ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _rtnExtDataHandler::onDropCS( const monCSSimple &csInfo, pmdEDUCB *cb )
   {
      INT32 rc = SDB_OK ;

      SDB_DMSCB *dmsCB = pmdGetKRCB()->getDMSCB() ;
      MON_CS_SIM_LIST csList ;
      vector< string > cappedCSs ;

      // Traverse all the indices of all collections in the cs, to find out all
      // the text indexes.
      for ( MON_CL_SIM_VEC::const_iterator clItr = csInfo._clList.begin();
            clItr != csInfo._clList.end(); ++clItr )
      {
         for ( MON_IDX_LIST::const_iterator idxItr = clItr->_idxList.begin();
               idxItr != clItr->_idxList.end(); ++idxItr )
         {
            UINT16 idxType = IXM_EXTENT_TYPE_NONE ;
            rc = idxItr->getIndexType( idxType ) ;
            if ( rc )
            {
               // Only warning, and process the remaining ones.
               PD_LOG( PDWARNING, "Get type of index failed[ %d ], cs[ %s ],"
                       " cl[ %s ], index[ %s ]", rc, csInfo._name,
                       clItr->_name, idxItr->getIndexName() ) ;
               continue ;
            }
            if ( IXM_EXTENT_TYPE_TEXT == idxType )
            {
               string cappedCSName = string(SYS_PREFIX) + string(csInfo._name)
                                  + string(clItr->_clname) +
                                  idxItr->getIndexName() ;
               rtnDropCollectionSpaceCommand( cappedCSName.c_str(), cb, dmsCB,
                                              NULL, TRUE ) ;
            }
         }
      }

      return rc ;
   }

   INT32 _rtnExtDataHandler::onCreateTextIdx( const CHAR *clFullName,
                                              const CHAR *idxName,
                                              INT64 bufferSize,
                                              pmdEDUCB* cb,
                                              SDB_DPSCB *dpsCB )
   {
      INT32 rc = SDB_OK ;
      SDB_DMSCB *dmsCB = pmdGetKRCB()->getDMSCB() ;
      string cappedCSFullName = clFullName ;
      string cappedCLFullName ;
      BOOLEAN csCreated = FALSE ;
      BSONObj extOptions ;
      BSONObjBuilder builder ;
      SDB_RTNCB *rtnCB = pmdGetKRCB()->getRTNCB() ;

      cappedCSFullName.erase( cappedCSFullName.find("."), 1 ) ;
      cappedCSFullName = SYS_PREFIX + cappedCSFullName  + string(idxName) ;
      cappedCLFullName = cappedCSFullName + "." + cappedCSFullName ;

      rc = rtnCreateCollectionSpaceCommand( cappedCSFullName.c_str(), cb, dmsCB,
                                            dpsCB, DMS_PAGE_SIZE_DFT,
                                            DMS_DO_NOT_CREATE_LOB,
                                            DMS_STORAGE_CAPPED, TRUE ) ;
      PD_RC_CHECK( rc, PDERROR, "Create capped collection space failed[ %d ]",
                   rc ) ;
      csCreated = TRUE ;

      // The unit of bufferSize is MB.
      builder.append( FIELD_NAME_SIZE, ( bufferSize << 20 ) ) ;
      builder.append( FIELD_NAME_MAX, RTN_CAPPED_CL_MAXRECNUM ) ;
      // Set the OverWrite option as false.
      builder.appendBool( FIELD_NAME_OVERWRITE, FALSE ) ;
      extOptions = builder.done() ;

      rc = rtnCreateCollectionCommand( cappedCLFullName.c_str(),
                                       DMS_MB_ATTR_NOIDINDEX,
                                       cb, dmsCB, dpsCB,
                                       UTIL_COMPRESSOR_INVALID, 0,
                                       TRUE, &extOptions ) ;
      PD_RC_CHECK( rc, PDERROR, "Create capped collection failed[ %d ]", rc ) ;
      rtnCB->incTextIdxVersion() ;

   done:
      return rc ;
   error:
      if ( csCreated )
      {
         rtnDropCollectionSpaceCommand( cappedCSFullName.c_str(), cb,
                                        dmsCB, dpsCB, TRUE ) ;
      }
      goto done ;
   }

   INT32 _rtnExtDataHandler::onDropTextIdx( const CHAR *clFullName,
                                            const CHAR *idxName,
                                            pmdEDUCB* cb,
                                            SDB_DPSCB *dpscb )
   {
      INT32 rc = SDB_OK ;
      string cappedCSFullName = clFullName ;
      SDB_DMSCB *dmsCB = pmdGetKRCB()->getDMSCB() ;

      cappedCSFullName.erase( cappedCSFullName.find("."), 1 ) ;
      cappedCSFullName = SYS_PREFIX + cappedCSFullName  + string(idxName) ;

      rc = rtnDropCollectionSpaceCommand( cappedCSFullName.c_str(), cb, dmsCB,
                                          dpscb, FALSE ) ;
      PD_RC_CHECK( rc, PDERROR, "Drop capped collection space failed[ %d ]",
                   rc ) ;
   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _rtnExtDataHandler::onInsert( const CHAR *clFullName,
                                       const CHAR *idxName, BSONObj &object,
                                       bson::OID &oid, INT32 flags,
                                       pmdEDUCB* cb, SDB_DPSCB *dpscb )
   {
      INT32 rc = SDB_OK ;
      string cappedCSFullName = clFullName ;
      string cappedCLFullName ;

      cappedCSFullName.erase( cappedCSFullName.find("."), 1 ) ;
      cappedCSFullName = SYS_PREFIX + cappedCSFullName  + string(idxName) ;
      cappedCLFullName = cappedCSFullName + "." + cappedCSFullName ;

      rc = _addOprRecord( cappedCLFullName.c_str(), DMS_EXT_INSERT, cb,
                          &oid, &object, NULL ) ;
      PD_RC_CHECK( rc, PDERROR, "Insert operation record failed[ %d ]", rc ) ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _rtnExtDataHandler::onDelete( const CHAR *clFullName,
                                       const CHAR *idxName, bson::OID &oid,
                                       pmdEDUCB* cb, SDB_DPSCB *dpscb )
   {
      INT32 rc = SDB_OK ;
      string cappedCSFullName = clFullName ;
      string cappedCLFullName ;

      cappedCSFullName.erase( cappedCSFullName.find("."), 1 ) ;
      cappedCSFullName = SYS_PREFIX + cappedCSFullName  + string(idxName) ;
      cappedCLFullName = cappedCSFullName + "." + cappedCSFullName ;

      rc = _addOprRecord( cappedCLFullName.c_str(), DMS_EXT_DELETE, cb,
                          &oid, NULL, NULL ) ;
      PD_RC_CHECK( rc, PDERROR, "Insert Operation Record failed[ %d ]", rc ) ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _rtnExtDataHandler::onUpdate( const CHAR *clFullName,
                                       const CHAR *idxName, BSONObj &object,
                                       bson::OID &oid, INT32 flags,
                                       pmdEDUCB* cb, SDB_DPSCB *dpscb )
   {
      INT32 rc = SDB_OK ;
      string cappedCSFullName = clFullName ;
      string cappedCLFullName ;

      cappedCSFullName.erase( cappedCSFullName.find("."), 1 ) ;
      cappedCSFullName = SYS_PREFIX + cappedCSFullName  + string(idxName) ;
      cappedCLFullName = cappedCSFullName + "." + cappedCSFullName ;

      rc = _addOprRecord( cappedCLFullName.c_str(), DMS_EXT_UPDATE, cb,
                          &oid, &object, NULL ) ;
      PD_RC_CHECK( rc, PDERROR, "Insert Operation Record failed[ %d ]", rc ) ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _rtnExtDataHandler::onTruncate( const CHAR *clFullName,
                                         const CHAR *idxName, pmdEDUCB* cb,
                                         SDB_DPSCB *dpscb )
   {
      INT32 rc = SDB_OK ;
      string cappedCSFullName = clFullName ;
      string cappedCLFullName ;

      cappedCSFullName.erase( cappedCSFullName.find("."), 1 ) ;
      cappedCSFullName = SYS_PREFIX + cappedCSFullName  + string(idxName) ;
      cappedCLFullName = cappedCSFullName + "." + cappedCSFullName ;

      rc = _addOprRecord( cappedCLFullName.c_str(), DMS_EXT_TRUNCATE, cb,
                          NULL, NULL, NULL ) ;
      PD_RC_CHECK( rc, PDERROR, "Insert Operation Record failed[ %d ]", rc ) ;

   done:
      return rc ;
   error:
      goto done ;
   }

   rtnExtDataHandler* getRtnExtDataHandler()
   {
      static rtnExtDataHandler s_rtnExtDataHandler ;
      return &s_rtnExtDataHandler ;
   }
}

