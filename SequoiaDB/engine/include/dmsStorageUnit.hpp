/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = dmsStorageUnit.hpp

   Descriptive Name = Data Management Service Storage Unit Header

   When/how to use: this program may be used on binary and text-formatted
   versions of data management component. This file contains structure for
   DMS storage unit and its methods.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          09/14/2012  TW  Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef DMSSTORAGEUNIT_HPP_
#define DMSSTORAGEUNIT_HPP_

#include "dmsStorageData.hpp"
#include "dmsStorageIndex.hpp"
#include "rtnAPM.hpp"

using namespace bson ;

namespace engine
{

   class _monCollection ;
   class _monStorageUnit ;
   class _monIndex ;
   class _ixmIndexCB ;
   class _dmsTempCB ;
   class _SDB_DMSCB ;
   class _pmdEDUCB ;
   class _mthMatcher ;
   class _mthModifier ;

   /*
      _dmsStorageUnit define
   */
   class _dmsStorageUnit : public SDBObject
   {
      friend class _dmsTempCB ;
      friend class _SDB_DMSCB ;

      public:
         _dmsStorageUnit ( const CHAR *pSUName, UINT32 sequence,
                           INT32 pageSize = DMS_PAGE_SIZE_DFT ) ;
         ~_dmsStorageUnit() ;

         INT32 open ( const CHAR *pDataPath, const CHAR *pIndexPath,
                      BOOLEAN createNew = TRUE,
                      BOOLEAN delWhenExist = FALSE ) ;
         void  close () ;
         INT32 remove () ;

         dmsStorageData    *data() { return _pDataSu ; }
         dmsStorageIndex   *index() { return _pIndexSu ; }
         rtnAccessPlanManager *getAPM () { return &_apm ; }

         INT32       getPageSize() const { return _storageInfo._pageSize ; }
         const CHAR* CSName() const { return _storageInfo._suName ; }
         UINT32      CSSequence() const { return _storageInfo._sequence ; }
         UINT32      LogicalCSID() const { return _pDataSu->logicalID() ; }
         dmsStorageUnitID CSID() const { return _pDataSu->CSID() ; }
         INT64       totalSize () const ;

      public:
         void     dumpInfo ( vector<CHAR*> &collectionList,
                             BOOLEAN sys = FALSE ) ;
         void     dumpInfo ( set<_monCollection> &collectionList,
                             BOOLEAN sys = FALSE ) ;
         void     dumpInfo ( set<_monStorageUnit> &storageUnitList,
                             BOOLEAN sys = FALSE ) ;

         INT32    getSegExtents ( const CHAR *pName,
                                  vector< dmsExtentID > &segExtents,
                                  dmsMBContext *context = NULL ) ;

         INT32    getIndexes ( const CHAR *pName,
                               vector<_monIndex> &resultIndexes,
                               dmsMBContext *context = NULL ) ;

      // only for LOAD
      public:
         OSS_INLINE void    mapExtent2DelList( dmsMB * mb, dmsExtent * extAddr,
                                           SINT32 extentID ) ;

         OSS_INLINE INT32   extentRemoveRecord( dmsMB *mb,
                                            const dmsRecordID &recordID,
                                            INT32 recordSize,
                                            _pmdEDUCB *cb ) ;

         OSS_INLINE void    addExtentRecordCount( dmsMB *mb, UINT32 count ) ;

      // for dmsCB
      protected:
         OSS_INLINE void  _setLogicalCSID( UINT32 logicalID ) ;

         OSS_INLINE void  _setCSID( dmsStorageUnitID CSID ) ;

         INT32        _resetCollection( dmsMBContext *context ) ;

      public:

         INT32    insertRecord ( const CHAR *pName,
                                 BSONObj &record,
                                 _pmdEDUCB *cb,
                                 SDB_DPSCB *dpscb,
                                 BOOLEAN mustOID = TRUE,
                                 BOOLEAN canUnLock = TRUE,
                                 dmsMBContext *context = NULL ) ;

         INT32    updateRecords ( const CHAR *pName,
                                  _pmdEDUCB *cb,
                                  SDB_DPSCB *dpscb,
                                  _mthMatcher *matcher,
                                  _mthModifier &modifier,
                                  SINT64 &numRecords,
                                  SINT64 maxUpdate = -1,
                                  dmsMBContext *context = NULL ) ;

         INT32    deleteRecords ( const CHAR *pName,
                                  _pmdEDUCB * cb,
                                  SDB_DPSCB *dpscb,
                                  _mthMatcher *matcher,
                                  SINT64 &numRecords,
                                  SINT64 maxDelete = -1,
                                  dmsMBContext *context = NULL ) ;

         INT32    rebuildIndexes ( const CHAR *pName,
                                   _pmdEDUCB * cb,
                                   dmsMBContext *context = NULL ) ;

         INT32    createIndex ( const CHAR *pName, const BSONObj &index,
                                _pmdEDUCB * cb, SDB_DPSCB *dpscb,
                                BOOLEAN isSys = FALSE,
                                dmsMBContext *context = NULL ) ;

         INT32    dropIndex( const CHAR *pName, const CHAR *indexName,
                             _pmdEDUCB * cb, SDB_DPSCB *dpscb,
                             BOOLEAN isSys = FALSE,
                             dmsMBContext *context = NULL ) ;

         INT32    dropIndex( const CHAR *pName, OID &indexOID,
                             _pmdEDUCB * cb, SDB_DPSCB *dpscb,
                             BOOLEAN isSys = FALSE,
                             dmsMBContext *context = NULL ) ;

         INT32    countCollection ( const CHAR *pName,
                                    INT64 &recordNum,
                                    _pmdEDUCB *cb,
                                    dmsMBContext *context = NULL ) ;

         INT32    getCollectionFlag ( const CHAR *pName, UINT16 &flag,
                                      dmsMBContext *context = NULL ) ;

         INT32    changeCollectionFlag ( const CHAR *pName, UINT16 flag,
                                         dmsMBContext *context = NULL ) ;

         INT32    getCollectionAttributes ( const CHAR *pName,
                                            UINT32 &attributes,
                                            dmsMBContext *context = NULL ) ;

         INT32    updateCollectionAttributes ( const CHAR *pName,
                                               UINT32 newAttributes,
                                               dmsMBContext *context = NULL ) ;

         //loadExtentA is not init extent records
         INT32    loadExtentA ( dmsMBContext *mbContext, const CHAR *pBuffer,
                                UINT16 numPages, const BOOLEAN toLoad = FALSE,
                                SINT32 *allocatedExtent = NULL,
                                dmsExtent **tExtAddr = NULL ) ;

         //loadExtent will init extent records
         INT32    loadExtent ( dmsMBContext *mbContext, const CHAR *pBuffer,
                               UINT16 numPages ) ;

      private :
         rtnAccessPlanManager                _apm ;

         dmsStorageData                      *_pDataSu ;
         dmsStorageIndex                     *_pIndexSu ;
         dmsStorageInfo                      _storageInfo ;

   } ;
   typedef _dmsStorageUnit dmsStorageUnit ;

   /*
      _dmsStorageUnit OSS_INLINE functions
   */
   OSS_INLINE void _dmsStorageUnit::mapExtent2DelList( dmsMB * mb,
                                                   dmsExtent *extAddr,
                                                   SINT32 extentID )
   {
      return _pDataSu->_mapExtent2DelList( mb, extAddr, extentID ) ;
   }
   OSS_INLINE INT32 _dmsStorageUnit::extentRemoveRecord(dmsMB * mb,
                                                    const dmsRecordID &recordID,
                                                    INT32 recordSize,
                                                    _pmdEDUCB *cb )
   {
      return _pDataSu->_extentRemoveRecord( mb, recordID, recordSize, cb ) ;
   }
   OSS_INLINE void _dmsStorageUnit::addExtentRecordCount( dmsMB * mb, UINT32 count )
   {
      _pDataSu->_mbStatInfo[ mb->_blockID ]._totalRecords += count ;
   }
   OSS_INLINE void _dmsStorageUnit::_setLogicalCSID( UINT32 logicalID )
   {
      if ( _pDataSu )
      {
         _pDataSu->_logicalCSID = logicalID ;
      }
   }
   OSS_INLINE void _dmsStorageUnit::_setCSID( dmsStorageUnitID CSID )
   {
      if ( _pDataSu )
      {
         _pDataSu->_CSID = CSID ;
      }
   }

}

#endif //DMSSTORAGEUNIT_HPP_

