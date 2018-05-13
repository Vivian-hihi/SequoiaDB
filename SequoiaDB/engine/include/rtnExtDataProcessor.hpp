/*******************************************************************************


   Copyright (C) 2011-2018 SequoiaDB Ltd.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

   Source File Name = rtnExtDataProcessor.hpp

   Descriptive Name = External data processor for rtn.

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
#ifndef RTN_EXTDATAPROCESSOR_HPP__
#define RTN_EXTDATAPROCESSOR_HPP__

#include "core.hpp"
#include "oss.hpp"
#include "dms.hpp"
#include "ixm.hpp"
#include "dpsLogWrapper.hpp"
#include "pmdEDU.hpp"
#include "utilConcurrentMap.hpp"
#include "rtnExtOprDef.hpp"
#include "dmsStorageDataCommon.hpp"

#define RTN_EXT_PROCESSOR_NAME_SZ   255
namespace engine
{
   struct _rtnExtProcessorMeta
   {
      string  _csName ;
      string  _clName ;
      string  _idxName ;
      BSONObj _idxKeyDef ;

      INT32 init( const CHAR *csName, const CHAR *clName, const CHAR *idxName,
                  const BSONObj &idxKeyDef );
   } ;
   typedef _rtnExtProcessorMeta rtnExtProcessorMeta ;

   /*
    * One processor is according to one text index. It handles operations on the
    * relative capped collection. All processors are created and  managed by
    * the processor manager.
    * Processor will be created when a text index is created, or when open a
    * collection space, which contains text indices.
    */
   class _rtnExtDataProcessor : public SDBObject
   {
   public:
      _rtnExtDataProcessor() ;
      ~_rtnExtDataProcessor() ;

      INT32 init( const CHAR *csName, const CHAR *clName,
                  const CHAR *idxName, const BSONObj &idxKeyDef ) ;

      // User can set a name for the processor. If not set explicitly, it will
      // be set to the same with the full name of the capped cl by default.
      INT32 setName( const CHAR *name ) ;
      const CHAR* getName() const ;

      INT32 check() ;
      INT32 processInsert( const BSONObj &inputObj, pmdEDUCB *cb,
                           SDB_DPSCB *dpsCB = NULL ) ;
      INT32 processDelete( const BSONObj &inputObj, pmdEDUCB *cb,
                           SDB_DPSCB *dpsCB = NULL ) ;
      INT32 processUpdate( const BSONObj &originalObj, const BSONObj &newObj,
                           pmdEDUCB *cb, SDB_DPSCB *dpsCB = NULL ) ;

      INT32 doWrite( pmdEDUCB *cb, BSONObj &record, SDB_DPSCB *dpsCB = NULL ) ;
      INT32 doDropP1( pmdEDUCB *cb, SDB_DPSCB *dpsCB = NULL ) ;
      INT32 doDropP1Cancel( pmdEDUCB *cb, SDB_DPSCB *dpsCB = NULL ) ;
      INT32 doDropP2( pmdEDUCB *cb, SDB_DPSCB *dpsCB = NULL ) ;
      INT32 doLoad() ;
      INT32 doUnload( _pmdEDUCB *cb, SDB_DPSCB *dpsCB = NULL ) ;
      INT32 doRebuild( pmdEDUCB *cb, SDB_DPSCB *dpsCB = NULL ) ;

      INT32 done( pmdEDUCB *cb ) ;
      INT32 abort() ;

      const rtnExtProcessorMeta& getMeta() const { return _meta ; }
      INT32 updateMeta( const rtnExtProcessorMeta& meta ) ;
      BOOLEAN isOwnedBy( const CHAR *csName, const CHAR *clName = NULL,
                         const CHAR *idxName = NULL ) ;

      static void genExtDataNames( const CHAR *csName,
                                   const CHAR *clName,
                                   const CHAR *idxName,
                                   CHAR *extCSName,
                                   UINT32 csNameBufSize,
                                   CHAR *extCLName,
                                   UINT32 clNameBufSize ) ;

   private:
      INT32 _prepareCSAndCL( const CHAR *csName, const CHAR *clName,
                             _pmdEDUCB *cb, SDB_DPSCB *dpsCB ) ;

      /*
       * Why prepare and done? The commit LSN should be the same on primary and
       * slaves. If we write before the log DPS, the commit LSN will be newer
       * than that on the slave.
       */
      INT32 _prepareInsert( const BSONObj &inputObj, BSONObj &recordObj ) ;
      INT32 _prepareDelete( const BSONObj &inputObj, BSONObj &recordObj ) ;
      INT32 _prepareUpdate( const BSONObj &originalObj, const BSONObj &newObj,
                            BSONObj &recordObj ) ;

      INT32 _prepareRecord( const CHAR *name, _rtnExtOprType oprType,
                            const bson::OID *dataOID,
                            const BSONObj *dataObj,
                            BSONObj &recordObj ) ;

      const CHAR* _getExtCLShortName() const ;

   private:
      dmsMBContext         *_mbContext ;
      CHAR                 _name[ RTN_EXT_PROCESSOR_NAME_SZ + 1 ] ;
      rtnExtProcessorMeta  _meta ;
      CHAR                 _cappedCSName[ DMS_COLLECTION_SPACE_NAME_SZ + 1 ] ;
      CHAR                 _cappedCLName[ DMS_COLLECTION_FULL_NAME_SZ + 1 ] ;
      BOOLEAN              _needUpdateLSN ;
   } ;
   typedef _rtnExtDataProcessor rtnExtDataProcessor ;

   class _rtnExtDataProcessorMgr : public SDBObject
   {
      typedef std::map<const CHAR*, rtnExtDataProcessor *>  PROCESSOR_MAP ;
      typedef PROCESSOR_MAP::iterator                       PROCESSOR_MAP_ITR ;
      typedef std::map<const CHAR*, ossRWMutex *>           PROCESSOR_LATCH_MAP ;
      typedef PROCESSOR_LATCH_MAP::iterator                 PROCESSOR_LATCH_MAP_ITR ;
      typedef std::map<std::string, ossRWMutex *>           CS_LATCH_MAP ;
      typedef CS_LATCH_MAP::iterator                        CS_LATCH_MAP_ITR ;
   public:
      _rtnExtDataProcessorMgr() ;
      ~_rtnExtDataProcessorMgr () ;

      INT32 createProcessor( const CHAR *csName, const CHAR *clName,
                             const CHAR *idxName, const BSONObj &idxKeyDef,
                             rtnExtDataProcessor *&processor ) ;
      void destroyProcessor( rtnExtDataProcessor *&processor ) ;

      INT32 addProcessor( rtnExtDataProcessor *processor ) ;

      INT32 number()  ;
      INT32 getProcessorsAndLock( const CHAR *csName, const CHAR *clName,
                                  const CHAR *idxName, OSS_LATCH_MODE lockType,
                                  std::vector<rtnExtDataProcessor *> &processors ) ;
      void unlockProcessor( const CHAR *name, OSS_LATCH_MODE lockType ) ;
      void unlockProcessors( std::vector<rtnExtDataProcessor *> &processors,
                             OSS_LATCH_MODE lockType ) ;

      void delProcessor( rtnExtDataProcessor **processor ) ;

   private:
      UINT32 _genProcessorKey( const CHAR *csName, const CHAR *clName,
                               const CHAR *idxName ) ;
   private:
      // Mutex to protect meta data change.
      ossSpinSLatch       _mutex ;
      // Map key is processor name, the same with the capped cl name.
      PROCESSOR_MAP       _processorMap ;
      PROCESSOR_LATCH_MAP _latchMap ;
   } ;
   typedef _rtnExtDataProcessorMgr rtnExtDataProcessorMgr ;

  rtnExtDataProcessorMgr* rtnGetExtDataProcessorMgr() ;
}

#endif /* RTN_EXTDATAPROCESSOR_HPP__ */

