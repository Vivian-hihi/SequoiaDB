/*******************************************************************************


   Copyright (C) 2011-2014 SequoiaDB Ltd.

   This program is free software: you can redistribute it and/or modify
   it under the term of the GNU Affero General Public License, version 3,
   as published by the Free Software Foundation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warrenty of
   MARCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU Affero General Public License for more details.

   You should have received a copy of the GNU Affero General Public License
   along with this program. If not, see <http://www.gnu.org/license/>.

   Source File Name = rtnContext.hpp

   Descriptive Name = RunTime Context Header

   When/how to use: this program may be used on binary and text-formatted
   versions of Runtime component. This file contains structure for Runtime
   Context.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          09/14/2012  TW  Initial Draft

   Last Changed =

*******************************************************************************/
#ifndef RTNCONTEXT_HPP_
#define RTNCONTEXT_HPP_

#include "dms.hpp"
#include "pd.hpp"
#include "ossMem.hpp"
#include "ossLatch.hpp"
#include "ossRWMutex.hpp"
#include "monCB.hpp"
#include "ixm.hpp"
#include "optAccessPlan.hpp"
#include "qgmPlanContainer.hpp"
#include "msg.h"
#include "ossAtomic.hpp"
#include "../bson/bsonobj.h"
#include "dmsCB.hpp"
#include "dmsLobDef.hpp"
#include "mthMatchTree.hpp"
#include "mthSelector.hpp"
#include "rtnQueryOptions.hpp"
#include "rtnContextBuff.hpp"
#include "utilMap.hpp"
#include <string>

using namespace bson ;

namespace engine
{

   class _pmdEDUCB ;
   class _dmsStorageUnit ;
   class _rtnIXScanner ;
   class _optAccessPlan ;
   class _SDB_DMSCB ;
   class _dmsMBContext ;
   class _rtnContextBase ;

   #define RTN_CONTEXT_GETNUM_ONCE              (1000)
   /*
      _rtnPrefWatcher define
   */
   class _rtnPrefWatcher : public SDBObject
   {
      public:
         _rtnPrefWatcher () :_prefNum(0), _needWait(FALSE) {}
         ~_rtnPrefWatcher () {}
         void     reset ()
         {
            _needWait = _prefNum > 0 ? TRUE : FALSE ;
            _prefEvent.reset() ;
         }
         void     ntyBegin ()
         { 
            ++_prefNum ;
            _needWait = TRUE ;
         }
         void     ntyEnd ()
         {
            --_prefNum ;
            _prefEvent.signalAll() ;
         }
         INT32    waitDone( INT64 millisec = -1 )
         {
            if ( !_needWait && _prefNum <= 0 )
            {
               return 0 ;
            }
            INT32 rc = _prefEvent.wait( millisec, NULL ) ;
            if ( SDB_OK == rc )
            {
               return 1 ;
            }
            return rc ;
         }

      private:
         UINT32         _prefNum ;
         BOOLEAN        _needWait ;
         ossEvent       _prefEvent ;
   } ;
   typedef _rtnPrefWatcher rtnPrefWatcher ;

   /*
      RTN_CONTEXT_TYPE define
   */
   enum RTN_CONTEXT_TYPE
   {
      RTN_CONTEXT_DATA     = 1,
      RTN_CONTEXT_DUMP,
      RTN_CONTEXT_COORD,
      RTN_CONTEXT_QGM,
      RTN_CONTEXT_TEMP,
      RTN_CONTEXT_SP,
      RTN_CONTEXT_PARADATA,
      RTN_CONTEXT_MAINCL,
      RTN_CONTEXT_SORT,
      RTN_CONTEXT_QGMSORT,
      RTN_CONTEXT_DELCS,
      RTN_CONTEXT_DELCL,
      RTN_CONTEXT_DELMAINCL,
      RTN_CONTEXT_EXPLAIN,
      RTN_CONTEXT_LOB,
      RTN_CONTEXT_LOB_FETCHER,
      RTN_CONTEXT_SHARD_OF_LOB,
      RTN_CONTEXT_LIST_LOB,
      RTN_CONTEXT_OM_TRANSFER,

      /// Catalog contexts

      RTN_CONTEXT_CAT_BEGIN,

      /// Group related
      RTN_CONTEXT_CAT_REMOVE_GROUP,
      RTN_CONTEXT_CAT_ACTIVE_GROUP,
      RTN_CONTEXT_CAT_SHUTDOWN_GROUP,
      /// Node related
      RTN_CONTEXT_CAT_CREATE_NODE,
      RTN_CONTEXT_CAT_REMOVE_NODE,
      /// CollectionSpace related
      RTN_CONTEXT_CAT_DROP_CS,
      /// Collection related
      RTN_CONTEXT_CAT_CREATE_CL,
      RTN_CONTEXT_CAT_DROP_CL,
      RTN_CONTEXT_CAT_ALTER_CL,
      RTN_CONTEXT_CAT_LINK_CL,
      RTN_CONTEXT_CAT_UNLINK_CL,
      /// Index related
      RTN_CONTEXT_CAT_CREATE_IDX,
      RTN_CONTEXT_CAT_DROP_IDX,

      RTN_CONTEXT_CAT_END,
   } ;

   const CHAR *getContextTypeDesp( RTN_CONTEXT_TYPE type ) ;

   /*
      _rtnContextBase define
   */
   class _rtnContextBase : public SDBObject
   {
      friend class _rtnContextParaData ;
      public:
         _rtnContextBase ( INT64 contextID, UINT64 eduID ) ;
         virtual ~_rtnContextBase () ;
         string   toString() ;

         INT32    newMatcher () ;

         INT64    contextID () const { return _contextID ; }
         UINT64   eduID () const { return _eduID ; }

         monContextCB*     getMonCB () { return &_monCtxCB ; }
         ossRWMutex*       dataLock () { return &_dataLock ; }
         _mthSelector&     getSelector () { return _selector ; }
         _mthMatchTree*    getMatcher () { return _matcher ; }

         INT32    append( const BSONObj &result ) ;
         INT32    appendObjs( const CHAR *pObjBuff,
                              INT32 len,
                              INT32 num,
                              BOOLEAN needAligned = TRUE ) ;

         virtual INT32    getMore( INT32 maxNumToReturn,
                                   rtnContextBuf &buffObj,
                                   _pmdEDUCB *cb ) ;

         virtual void     getErrorInfo( INT32 rc,
                                        pmdEDUCB *cb,
                                        rtnContextBuf &buffObj )
         {}

         virtual UINT32   getCachedRecordNum() ;

         OSS_INLINE BOOLEAN  isEmpty () const ;

         INT64    numRecords () const { return _bufferNumRecords ; }
         INT32    buffSize () const { return _resultBufferSize ; }
         INT64    totalRecords () const { return _totalRecords ; }
         OSS_INLINE INT32 freeSize () const ;
         INT32    buffEndOffset () const { return _bufferEndOffset ; }

         BOOLEAN  isOpened () const { return _isOpened ; }
         BOOLEAN  eof () const { return _hitEnd ; }

         INT32    getReference() const ;

         void     enableCountMode() { _countOnly = TRUE ; }
         BOOLEAN  isCountMode() const { return _countOnly ; }

         /// write info( some context will write data, drop collection, etc..)
         void           setWriteInfo( SDB_DPSCB *dpsCB, INT16 w ) ;
         SDB_DPSCB*     getDPSCB() { return _pDpsCB ; }
         INT16          getW() const { return _w ; }

      // prefetch
      public:
         void     enablePrefetch ( _pmdEDUCB *cb,
                                   rtnPrefWatcher *pWatcher = NULL ) ;
         void     disablePrefetch ()
         {
            _prefetchID = 0 ;
            _pPrefWatcher = NULL ;
            _pMonAppCB = NULL ;
         }
         INT32    prefetchResult() const { return _prefetchRet ; }
         INT32    prefetch ( _pmdEDUCB *cb, UINT32 prefetchID ) ;
         void     waitForPrefetch() ;

      public:
         virtual std::string      name() const = 0 ;
         virtual RTN_CONTEXT_TYPE getType () const = 0 ;
         virtual _dmsStorageUnit* getSU () = 0 ;
         virtual _optAccessPlan*  getPlan () { return NULL ; }
         virtual BOOLEAN          isWrite() const { return FALSE ; }

      protected:
         void              _onDataEmpty () ;
         virtual INT32     _prepareData( _pmdEDUCB *cb ) = 0 ;
         virtual BOOLEAN   _canPrefetch () const { return FALSE ; }
         virtual void      _toString( stringstream &ss ) {}
         virtual BOOLEAN   _canPrepareMoreData() const { return FALSE ;}
         INT32             _prepareMoreData( _pmdEDUCB *cb );

      protected:
         INT32    _reallocBuffer ( SINT32 requiredSize ) ;
         OSS_INLINE void _empty () ;
         OSS_INLINE void _close () { _isOpened = FALSE ; }
         UINT32   _getWaitPrefetchNum () { return _waitPrefetchNum.peek() ; }
         BOOLEAN  _isInPrefetching () const { return _isInPrefetch ; }

         void     _resetTotalRecords( INT64 totalRecords )
         {
            _totalRecords = totalRecords ;
         }

      protected:
         monContextCB            _monCtxCB ;
         _mthSelector            _selector ;
         _mthMatchTree           *_matcher ;
         BOOLEAN                 _ownedMatcher ;
         // status
         BOOLEAN                 _hitEnd ;
         BOOLEAN                 _isOpened ;

         SDB_DPSCB               *_pDpsCB ;
         INT16                   _w ;

      private:
         INT64                   _contextID ;
         UINT64                  _eduID ;
         // buffer
         CHAR                   *_pResultBuffer ;
         INT32                   _resultBufferSize ;
         INT32                   _bufferCurrentOffset ;
         INT32                   _bufferEndOffset ;
         INT64                   _bufferNumRecords ;
         // control param
         INT64                   _totalRecords ;
         // mutex
         ossRWMutex              _dataLock ;
         ossRWMutex              _prefetchLock ;
         UINT32                  _prefetchID ;
         ossAtomic32             _waitPrefetchNum ;
         BOOLEAN                 _isInPrefetch ;
         INT32                   _prefetchRet ;
         rtnPrefWatcher          *_pPrefWatcher ;
         _monAppCB               *_pMonAppCB ;

         BOOLEAN                 _countOnly ;
   } ;
   typedef _rtnContextBase rtnContextBase ;
   typedef _rtnContextBase rtnContext ;

   /*
      _rtnContextBase OSS_INLINE functions
   */
   OSS_INLINE BOOLEAN _rtnContextBase::isEmpty () const
   {
      if ( !_countOnly )
      {
         return _bufferCurrentOffset >= _bufferEndOffset ;
      }
      else
      {
         return _bufferNumRecords <= 0 ? TRUE : FALSE ;
      }
   }
   OSS_INLINE void _rtnContextBase::_empty ()
   {
      _bufferCurrentOffset = 0 ;
      _bufferEndOffset     = 0 ;
      _totalRecords        = _totalRecords - _bufferNumRecords ;
      _bufferNumRecords    = 0 ;
   }
   OSS_INLINE INT32 _rtnContextBase::freeSize () const
   {
      return _resultBufferSize - ossAlign4((UINT32)_bufferEndOffset) ;
   }

   typedef _rtnContextBase* (*RTN_CTX_NEW_FUNC)( INT64 contextId, EDUID eduId ) ;

   class _rtnContextAssit: public SDBObject
   {
   public:
      _rtnContextAssit( RTN_CONTEXT_TYPE type,
                             std::string name,
                             RTN_CTX_NEW_FUNC func ) ;
      ~_rtnContextAssit() ;
   } ;

#define DECLARE_RTN_CTX_AUTO_REGISTER() \
   public: \
      static _rtnContextBase *newThis ( INT64 contextId, EDUID eduId ) ;

#define RTN_CTX_AUTO_REGISTER(theClass, type, name ) \
   _rtnContextBase *theClass::newThis ( INT64 contextId, EDUID eduId ) \
   { \
      return SDB_OSS_NEW theClass( contextId, eduId ) ;\
   } \
   _rtnContextAssit theClass##Assit ( type, std::string( name ), theClass::newThis ) ;

   struct _rtnContextInfo: public SDBObject
   {
      RTN_CONTEXT_TYPE  type ;
      std::string       name ;
      RTN_CTX_NEW_FUNC  newFunc ;
   } ;

   class _rtnContextBuilder: public SDBObject
   {
      friend class _rtnContextAssit ;

   public:
      _rtnContextBuilder() ;
      ~_rtnContextBuilder() ;

      _rtnContextBase* create ( RTN_CONTEXT_TYPE type, INT64 contextId, EDUID eduId ) ;
      void             release ( _rtnContextBase* context ) ;
      const _rtnContextInfo* find( RTN_CONTEXT_TYPE type ) const ;

   private:
      INT32 _register( RTN_CONTEXT_TYPE type,
                        std::string name,
                        RTN_CTX_NEW_FUNC func ) ;
      INT32 _insert( _rtnContextInfo* contextInfo ) ;
      void _releaseContextInfos() ;

   private:
      std::map<RTN_CONTEXT_TYPE, _rtnContextInfo*> _contextInfoMap ;
      typedef std::pair<RTN_CONTEXT_TYPE, _rtnContextInfo*> pair_type ;
      typedef std::map<RTN_CONTEXT_TYPE, _rtnContextInfo*>::const_iterator ctx_info_iterator ;
   } ;

   _rtnContextBuilder* sdbGetRTNContextBuilder() ;

   /*
      _rtnContextQGM define
   */
   class _rtnContextQGM : public _rtnContextBase
   {
      DECLARE_RTN_CTX_AUTO_REGISTER()
      public:
         _rtnContextQGM ( INT64 contextID, UINT64 eduID ) ;
         virtual ~_rtnContextQGM () ;

      public:
         virtual std::string      name() const ;
         virtual RTN_CONTEXT_TYPE getType () const ;
         virtual _dmsStorageUnit* getSU () { return NULL ; }

         INT32 open( _qgmPlanContainer *accPlan ) ;

      protected:
         virtual INT32   _prepareData( _pmdEDUCB *cb ) ;
         virtual BOOLEAN _canPrepareMoreData() const
         {
            return TRUE ;
         }

      private:
         _qgmPlanContainer          *_accPlan ;

   } ;
   typedef _rtnContextQGM rtnContextQGM ;

   class _qgmPlan ;

   class _rtnContextQgmSort : public _rtnContextBase
   {
      DECLARE_RTN_CTX_AUTO_REGISTER()
   public:
      _rtnContextQgmSort( INT64 contextID, UINT64 eduID ) ;
      virtual ~_rtnContextQgmSort() ;

   public:
      virtual std::string      name() const ;
      virtual RTN_CONTEXT_TYPE getType () const ;
      virtual _dmsStorageUnit* getSU () { return NULL ; }

      INT32 open( _qgmPlan *qp ) ;

   protected:
      virtual INT32   _prepareData( _pmdEDUCB *cb ) ;
      virtual BOOLEAN _canPrepareMoreData() const
      {
         return TRUE ;
      }

   private:
      _qgmPlan *_qp ;
   } ;
   typedef class _rtnContextQgmSort rtnContextQgmSort ;

}

#endif //RTNCONTEXT_HPP_

