/*******************************************************************************


   Copyright (C) 2011-2018 SequoiaDB Ltd.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU Affero General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Affero General Public License for more details.

   You should have received a copy of the GNU Affero General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.

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

#define RTN_EXT_PROCESSOR_NAME_SZ      255
#define RTN_EXT_PROCESSOR_INVALID_ID   -1
namespace engine
{
   // Metadata of the compressor.
   struct _rtnExtProcessorMeta
   {
      utilCLUniqueID _clUniqID ;
      string         _idxName ;
      BSONObj        _idxKeyDef ;

      _rtnExtProcessorMeta()
      : _clUniqID( UTIL_INVALID_UNIQUEID )
      {
      }

      INT32 init( utilCLUniqueID clUniqID, const CHAR *idxName,
                  const BSONObj &idxKeyDef )
      {
         _clUniqID = clUniqID ;
         _idxName = idxName ;
         _idxKeyDef = idxKeyDef.copy() ;   // need to copy or not ???
         return SDB_OK ;
      }
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

      INT32 init( utilCLUniqueID clUniqID, const CHAR *idxName,
                  const BSONObj &idxKeyDef ) ;

      // User can set a name for the processor. If not set explicitly, it will
      // be set to the same with the full name of the capped cl by default.
      // INT32 setName( const CHAR *name ) ;
      // const CHAR* getName() const ;

      INT64 getID() ;
      INT32 setTargetNames( const CHAR *csName, const CHAR *clName ) ;

      INT32 check( DMS_EXTOPR_TYPE type, const BSONObj *object,
                   const BSONObj *objectNew ) ;

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

      BOOLEAN isOwnedBy( utilCSUniqueID csUniqID,
                         utilCLUniqueID clUniqID = UTIL_INVALID_UNIQUEID,
                         const CHAR *idxName = NULL ) ;

      static void genExtDataNames( utilCLUniqueID clUniqID, const CHAR *idxName,
                                   string &extCSName, string &extCLName ) ;

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
                            const BSONElement &idEle,
                            const BSONObj *dataObj,
                            BSONObj &recordObj ) ;

      const CHAR* _getExtCLShortName() const ;

   private:
      dmsMBContext         *_mbContext ;  // TODO: YSD 在哪里释放的 ?
      INT64                _id ;
      // CHAR                 _name[ RTN_EXT_PROCESSOR_NAME_SZ + 1 ] ;
      rtnExtProcessorMeta  _meta ;
      CHAR                 _cappedCSName[ DMS_COLLECTION_SPACE_NAME_SZ + 1 ] ;
      CHAR                 _cappedCLName[ DMS_COLLECTION_FULL_NAME_SZ + 1 ] ;
      BOOLEAN              _needUpdateLSN ;
      BSONObjSet           _keySet ;
      BSONObjSet           _keySetNew ;
      BOOLEAN              _needOprRec ;
   } ;
   typedef _rtnExtDataProcessor rtnExtDataProcessor ;

   class _rtnExtDataProcessorMgr : public SDBObject
   {
      typedef std::map<INT64, rtnExtDataProcessor *> PROCESSOR_MAP ;
      typedef std::map<INT64, rtnExtDataProcessor *>::iterator PROCESSOR_MAP_ITR ;
      typedef std::map<INT64, ossRWMutex *>           PROCESSOR_LATCH_MAP ;
      typedef std::map<INT64, ossRWMutex *>::iterator PROCESSOR_LATCH_MAP_ITR ;
   public:
      _rtnExtDataProcessorMgr() ;
      ~_rtnExtDataProcessorMgr () ;

      // activate: Once activated, the processor can be used by other threads.
      INT32 createProcessor( utilCLUniqueID clUniqID, const CHAR *idxName,
                             const BSONObj &idxKeyDef,
                             rtnExtDataProcessor *&processor,
                             BOOLEAN activate = TRUE ) ;

      INT32 activateProcessor( rtnExtDataProcessor *processor ) ;

      void destroyProcessor( rtnExtDataProcessor *&processor ) ;

      INT32 number()  ;

      INT32 getProcessorsByCS( utilCSUniqueID csUniqID, OSS_LATCH_MODE lockType,
                               std::vector<rtnExtDataProcessor *> &processors ) ;

      INT32 getProcessorsByCL( utilCLUniqueID clUniqID, OSS_LATCH_MODE lockType,
                               std::vector<rtnExtDataProcessor *> &processors ) ;

      INT32 getProcessorByIdx( utilCLUniqueID clUniqID, const CHAR *idxName,
                               OSS_LATCH_MODE lockType,
                               rtnExtDataProcessor *&processor ) ;

      void unlockProcessor( INT64 processorID, OSS_LATCH_MODE lockType ) ;

      void unlockProcessors( std::vector<rtnExtDataProcessor *> &processors,
                             OSS_LATCH_MODE lockType ) ;

      void delProcessor( rtnExtDataProcessor **processor ) ;

   private:
      UINT32 _genProcessorKey( const CHAR *csName, const CHAR *clName,
                               const CHAR *idxName ) ;
      INT32 _activateProcessor( rtnExtDataProcessor *processor ) ;
   private:
      // Mutex to protect meta data change.
      ossSpinSLatch        _mutex ;
      // Map key is processor name, the same with the capped cl name.
      PROCESSOR_MAP        _processorMap ;
      PROCESSOR_LATCH_MAP  _latchMap ;
   } ;
   typedef _rtnExtDataProcessorMgr rtnExtDataProcessorMgr ;

  rtnExtDataProcessorMgr* rtnGetExtDataProcessorMgr() ;
}

#endif /* RTN_EXTDATAPROCESSOR_HPP__ */

