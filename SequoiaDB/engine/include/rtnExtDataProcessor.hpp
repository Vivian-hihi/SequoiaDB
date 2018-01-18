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

namespace engine
{
   struct _rtnExtProcessorMeta
   {
      UINT32 _csLogicalID ;
      UINT32 _clLogicalID ;
      dmsExtentID _idxLogicalID ;

      _rtnExtProcessorMeta()
      : _csLogicalID( DMS_INVALID_LOGICCSID ),
        _clLogicalID( DMS_INVALID_LOGICCLID ),
        _idxLogicalID( DMS_INVALID_EXTENT )
      {
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

      INT32 init( UINT32 csLogicalID, UINT32 clLogicalID,
                  dmsExtentID idxLogicalID ) ;

      /*
       * Why prepare and done? The commit LSN should be the same on primary and
       * slaves. If we write before the log DPS, the commit LSN will be newer
       * than that on the slave.
       */
      INT32 prepareInsert( const ixmIndexCB &indexCB, const BSONObj &inputObj,
                           BSONObj &recordObj ) ;
      INT32 prepareDelete( const ixmIndexCB &indexCB, const BSONObj &inputObj,
                           BSONObj &recordObj ) ;
      INT32 prepareUpdate( const ixmIndexCB &indexCB,
                           const BSONObj &originalObj,
                           const BSONObj &newObj, BSONObj &recordObj ) ;

      INT32 doWrite( pmdEDUCB *cb, BSONObj &record, SDB_DPSCB *dpsCB = NULL ) ;
      INT32 doDropP1( pmdEDUCB *cb, SDB_DPSCB *dpsCB = NULL ) ;
      INT32 doDropP1Cancel( pmdEDUCB *cb, SDB_DPSCB *dpsCB = NULL ) ;
      INT32 doDropP2( pmdEDUCB *cb, SDB_DPSCB *dpsCB = NULL ) ;
      INT32 doLoad() ;
      INT32 doUnload( _pmdEDUCB *cb, SDB_DPSCB *dpsCB = NULL ) ;
      INT32 doRebuild( pmdEDUCB *cb, SDB_DPSCB *dpsCB = NULL ) ;

      const rtnExtProcessorMeta& getMeta() const { return _meta ; }
      INT32 updateMeta( const rtnExtProcessorMeta& meta ) ;

   private:
      void _lock() ;
      void _unlock() ;
      INT32 _prepareCSAndCL( const CHAR *csName, const CHAR *clName,
                             _pmdEDUCB *cb, SDB_DPSCB *dpsCB ) ;

      INT32 _prepareRecord( const CHAR *name, _rtnExtOprType oprType,
                            const bson::OID *dataOID,
                            const BSONObj *dataObj,
                            BSONObj &recordObj ) ;

   private:
      rtnExtProcessorMeta _meta ;
      CHAR _cappedCSName[ DMS_COLLECTION_SPACE_NAME_SZ + 1 ] ;
      CHAR _cappedCLName[ DMS_COLLECTION_NAME_SZ + 1 ] ;
      ossSpinXLatch _latch ;
   } ;
   typedef _rtnExtDataProcessor rtnExtDataProcessor ;

   class _rtnExtDataProcessorMgr : public SDBObject
   {
      typedef std::map<string, rtnExtDataProcessor *>   PROCESSOR_MAP ;
      typedef PROCESSOR_MAP::iterator                   PROCESSOR_MAP_ITR ;
   public:
      _rtnExtDataProcessorMgr() ;
      ~_rtnExtDataProcessorMgr () ;

      INT32 addProcessor( UINT32 csLogicalID, UINT32 clLogicalID,
                          dmsExtentID idxLogicalID,
                          rtnExtDataProcessor** processor ) ;

      void getProcessors( UINT32 csLogicalID,
                          vector<rtnExtDataProcessor *> &processorVec ) ;

      void getProcessors( UINT32 csLogicalID, UINT32 clLogicalID,
                          vector<rtnExtDataProcessor *> &processorVec ) ;

      INT32 getProcessor( UINT32 csLogicalID, UINT32 clLogicalID,
                          dmsExtentID idxLogicalID, rtnExtDataProcessor **processor,
                          BOOLEAN create = FALSE ) ;

      void delProcessor( rtnExtDataProcessor **processor ) ;

   private:
      ossRWMutex     _mutex ;
      // Use a hash map to manage all the processors.
      PROCESSOR_MAP  _processorMap ;
   } ;
   typedef _rtnExtDataProcessorMgr rtnExtDataProcessorMgr ;

  rtnExtDataProcessorMgr* rtnGetExtDataProcessorMgr() ;
}

#endif /* RTN_EXTDATAPROCESSOR_HPP__ */

