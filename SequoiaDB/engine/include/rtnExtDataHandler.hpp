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

   Source File Name = rtnExtDataHandler.hpp

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
#ifndef RTN_EXTDATAHANDLER_HPP__
#define RTN_EXTDATAHANDLER_HPP__

#include "pmdEDU.hpp"
#include "dpsLogWrapper.hpp"
#include "dmsExtDataHandler.hpp"
#include "rtnExtDataProcessor.hpp"

namespace engine
{
   // The life circle of this context is in each operation. It holds all the
   // text index processors of one collection.
   class _rtnExtContextBase : public SDBObject
   {
   public:
      _rtnExtContextBase( DMS_EXTOPR_TYPE type ) ;
      virtual ~_rtnExtContextBase() ;
      void setID( UINT32 id )
      {
         _id = id ;
      }

      UINT32 getID() const
      {
         return _id ;
      }

      DMS_EXTOPR_TYPE getType() const { return _type ; }
      virtual INT32 done( _pmdEDUCB *cb, SDB_DPSCB *dpscb = NULL ) ;
      virtual INT32 abort( _pmdEDUCB *cb, SDB_DPSCB *dpscb = NULL ) ;
   protected:
      void _appendProcessors( const vector< rtnExtDataProcessor * >& processorVec ) ;
      virtual INT32 _onDone( _pmdEDUCB *cb, SDB_DPSCB *dpscb = NULL ) { return SDB_OK ; }
      virtual INT32 _onAbort( _pmdEDUCB *cb, SDB_DPSCB *dpscb = NULL ) { return SDB_OK ; }

   private:
      void _cleanup() ;

   protected:
      typedef vector< rtnExtDataProcessor * > EDP_VEC ;
      typedef EDP_VEC::iterator EDP_VEC_ITR ;
      typedef vector< BSONObj > OBJ_VEC ;
      typedef OBJ_VEC::iterator OBJ_VEC_ITR ;

      DMS_EXTOPR_TYPE         _type ;
      UINT32                  _id ;
      rtnExtDataProcessorMgr  *_processorMgr ;
      EDP_VEC                 _processors ;
      BOOLEAN                 _processorLocked ;
      OSS_LATCH_MODE          _lockType ;
   } ;
   typedef _rtnExtContextBase rtnExtContextBase ;

   class _rtnExtRebuildIdxCtx : public _rtnExtContextBase
   {
   public:
      _rtnExtRebuildIdxCtx() ;
      virtual ~_rtnExtRebuildIdxCtx() ;

      INT32 open( rtnExtDataProcessorMgr *processorMgr,
                  const CHAR *csName, const CHAR *clName,
                  const CHAR *idxName,
                  const BSONObj &idxKeyDef, pmdEDUCB *cb,
                  SDB_DPSCB *dpscb = NULL ) ;
   } ;
   typedef _rtnExtRebuildIdxCtx rtnExtRebuildIdxCtx ;

   // Base class for insert/delete/update operations.
   class _rtnExtDataOprCtx : public _rtnExtContextBase
   {
   public:
      _rtnExtDataOprCtx( DMS_EXTOPR_TYPE type ) ;
      virtual ~_rtnExtDataOprCtx() ;

   private:
      virtual INT32 _onDone( _pmdEDUCB *cb, SDB_DPSCB *dpscb = NULL ) ;
      // virtual INT32 _onAbort( _pmdEDUCB *cb, SDB_DPSCB *dpscb = NULL ) ;
   } ;
   typedef _rtnExtDataOprCtx rtnExtDataOprCtx ;

   class _rtnExtInsertCtx : public _rtnExtDataOprCtx
   {
   public:
      _rtnExtInsertCtx() ;
      ~_rtnExtInsertCtx() ;

      virtual INT32 open( rtnExtDataProcessorMgr *processorMgr,
                          const CHAR *csName, const CHAR *clName,
                          const CHAR *idxName, const BSONObj &object,
                          _pmdEDUCB *cb, SDB_DPSCB *dpscb = NULL ) ;
   } ;
   typedef _rtnExtInsertCtx rtnExtInsertCtx ;

   class _rtnExtDeleteCtx : public _rtnExtDataOprCtx
   {
   public:
      _rtnExtDeleteCtx() ;
      ~_rtnExtDeleteCtx() ;

      virtual INT32 open( rtnExtDataProcessorMgr *processorMgr,
                          const CHAR *csName, const CHAR *clName,
                          const CHAR *idxName, const BSONObj &object,
                          _pmdEDUCB *cb, SDB_DPSCB *dpscb = NULL ) ;
   } ;
   typedef _rtnExtDeleteCtx rtnExtDeleteCtx ;

   class _rtnExtUpdateCtx : public _rtnExtDataOprCtx
   {
   public:
      _rtnExtUpdateCtx() ;
      ~_rtnExtUpdateCtx() ;

      virtual INT32 open( rtnExtDataProcessorMgr *processorMgr,
                          const CHAR *csName, const CHAR *clName,
                          const CHAR *idxName, const BSONObj &oldObj,
                          const BSONObj &newObj, pmdEDUCB *cb,
                          SDB_DPSCB *dpscb = NULL ) ;
   } ;
   typedef _rtnExtUpdateCtx rtnExtUpdateCtx ;

   class _rtnExtDropOprCtx : public _rtnExtContextBase
   {
   public:
      _rtnExtDropOprCtx( DMS_EXTOPR_TYPE type ) ;
      virtual ~_rtnExtDropOprCtx() ;

      virtual INT32 open( rtnExtDataProcessorMgr *processorMgr,
                          const CHAR *csName, const CHAR *clName,
                          const CHAR *idxName, pmdEDUCB *cb,
                          BOOLEAN removeFiles, SDB_DPSCB *dpscb = NULL ) ;

   private:
      virtual INT32 _onDone( _pmdEDUCB *cb, SDB_DPSCB *dpscb = NULL ) ;
      virtual INT32 _onAbort( _pmdEDUCB *cb, SDB_DPSCB *dpscb = NULL ) ;
   private:
      BOOLEAN _removeFiles ;
   } ;
   typedef _rtnExtDropOprCtx rtnExtDropOprCtx ;

   class _rtnExtDropCSCtx : public _rtnExtDropOprCtx
   {
   public:
      _rtnExtDropCSCtx()
      : _rtnExtDropOprCtx( DMS_EXTOPR_TYPE_DROPCS )
      {
      }

      ~_rtnExtDropCSCtx() {}
   } ;
   typedef _rtnExtDropCSCtx rtnExtDropCSCtx ;

   class _rtnExtDropCLCtx : public _rtnExtDropOprCtx
   {
   public:
      _rtnExtDropCLCtx()
      : _rtnExtDropOprCtx( DMS_EXTOPR_TYPE_DROPCL )
      {
      }
      ~_rtnExtDropCLCtx() {}
   } ;
   typedef _rtnExtDropCLCtx rtnExtDropCLCtx ;

   class _rtnExtDropIdxCtx : public _rtnExtDropOprCtx
   {
   public:
      _rtnExtDropIdxCtx()
      : _rtnExtDropOprCtx( DMS_EXTOPR_TYPE_DROPIDX )
      {
      }
      ~_rtnExtDropIdxCtx() {}
   } ;
   typedef _rtnExtDropIdxCtx rtnExtDropIdxCtx ;

   class _rtnExtTruncateCtx : public _rtnExtContextBase
   {
   public:
      _rtnExtTruncateCtx() ;
      ~_rtnExtTruncateCtx() ;

      virtual INT32 open( rtnExtDataProcessorMgr *processorMgr,
                          const CHAR *csName, const CHAR *clName,
                          _pmdEDUCB *cb, SDB_DPSCB *dpscb = NULL ) ;
      virtual INT32 _onDone( _pmdEDUCB *cb, SDB_DPSCB *dpscb = NULL ) ;
      virtual INT32 _onAbort( _pmdEDUCB *cb, SDB_DPSCB *dpscb = NULL ) ;
   } ;
   typedef _rtnExtTruncateCtx rtnExtTruncateCtx ;

   class _rtnExtContextMgr : public SDBObject
   {
      typedef utilConcurrentMap<UINT32, rtnExtContextBase*> RTN_CTX_MAP ;
   public:
      _rtnExtContextMgr() ;
      ~_rtnExtContextMgr() ;

      rtnExtContextBase* findContext( UINT32 contextID ) ;
      INT32 createContext( DMS_EXTOPR_TYPE type, _pmdEDUCB *cb,
                           rtnExtContextBase** context ) ;
      INT32 delContext( UINT32 contextID, _pmdEDUCB *cb ) ;

   private:
      ossRWMutex        _mutex ;
      RTN_CTX_MAP       _contextMap ;
   } ;
   typedef _rtnExtContextMgr rtnExtContextMgr ;

   class _rtnExtDataHandler : public _IDmsExtDataHandler
   {
   public:
      _rtnExtDataHandler( rtnExtDataProcessorMgr *edpMgr ) ;
      virtual ~_rtnExtDataHandler() ;

   public:
      virtual INT32 getExtDataName( const CHAR *csName, const CHAR *clName,
                                    const CHAR *idxName, CHAR *buff,
                                    UINT32 buffSize ) ;

      virtual INT32 check( DMS_EXTOPR_TYPE type, const CHAR *csName,
                           const CHAR *clName, const CHAR *idxName,
                           pmdEDUCB *cb ) ;

      virtual INT32 onOpenTextIdx( const CHAR *csName, const CHAR *clName,
                                   const CHAR *idxName,
                                   const BSONObj &idxKeyDef ) ;

      virtual INT32 onDelCS( const CHAR *csName, pmdEDUCB *cb,
                             BOOLEAN removeFiles, SDB_DPSCB *dpscb = NULL ) ;

      virtual INT32 onDelCL( const CHAR *csName, const CHAR *clName,
                             pmdEDUCB *cb, SDB_DPSCB *dpscb = NULL ) ;

      virtual INT32 onCrtTextIdx( const CHAR *csName, const CHAR *clName,
                                  const CHAR *idxName, _pmdEDUCB *cb,
                                  SDB_DPSCB *dpscb = NULL ) ;

      virtual INT32 onDropTextIdx( const CHAR *csName, const CHAR *clName,
                                   const CHAR *idxName, _pmdEDUCB *cb,
                                   SDB_DPSCB *dpscb = NULL ) ;

      virtual INT32 onRebuildTextIdx( const CHAR *csName, const CHAR *clName,
                                      const CHAR *idxName,
                                      const BSONObj &idxKeyDef, _pmdEDUCB *cb,
                                      SDB_DPSCB *dpscb = NULL ) ;

      virtual INT32 onInsert( const CHAR *csName, const CHAR *clName,
                              const CHAR *idxName,  const ixmIndexCB &indexCB,
                              const BSONObj &object, _pmdEDUCB* cb,
                              SDB_DPSCB *dpscb = NULL ) ;

      virtual INT32 onDelete( const CHAR *csName, const CHAR *clName,
                              const CHAR *idxName, const ixmIndexCB &indexCB,
                              const BSONObj &object, _pmdEDUCB* cb,
                              SDB_DPSCB *dpscb = NULL ) ;

      virtual INT32 onUpdate( const CHAR *csName, const CHAR *clName,
                              const CHAR *idxName, const ixmIndexCB &indexCB,
                              const BSONObj &orignalObj, const BSONObj &newObj,
                              _pmdEDUCB* cb, SDB_DPSCB *dpscb = NULL ) ;

      INT32 onTruncateCL( const CHAR *csName, const CHAR *clName,
                           _pmdEDUCB *cb, SDB_DPSCB *dpsCB = NULL ) ;

      virtual INT32 done( DMS_EXTOPR_TYPE type, _pmdEDUCB *cb,
                          SDB_DPSCB *dpscb = NULL ) ;

      virtual INT32 abortOperation( DMS_EXTOPR_TYPE type, _pmdEDUCB *cb ) ;

   private:
      INT32 _prepareCSAndCL( const CHAR *csName, const CHAR *clName,
                             pmdEDUCB *cb, SDB_DPSCB *dpsCB ) ;

   private:
      rtnExtDataProcessorMgr  *_edpMgr ;
      rtnExtContextMgr        _contextMgr ;
   } ;
   typedef _rtnExtDataHandler rtnExtDataHandler ;

   rtnExtDataHandler* rtnGetExtDataHandler() ;
}

#endif /* RTN_EXTDATAHANDLER_HPP__ */

