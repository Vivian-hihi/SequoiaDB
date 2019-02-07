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

   Source File Name = dmsScanner.hpp

   Descriptive Name = Data Management Service Storage Unit Header

   When/how to use: this program may be used on binary and text-formatted
   versions of data management component. This file contains structure for
   DMS storage unit and its methods.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          22/08/2013  XJH Initial Draft

   Last Changed =

*******************************************************************************/
#ifndef DMSSCANNER_HPP__
#define DMSSCANNER_HPP__

#include "core.hpp"
#include "oss.hpp"
#include "dms.hpp"
#include "dmsExtent.hpp"
#include "ossUtil.hpp"
#include "ossMem.hpp"
#include "dmsStorageBase.hpp"
#include "dmsStorageDataCommon.hpp"
#include "dmsStorageData.hpp"
#include "dmsStorageDataCapped.hpp"
#include "../bson/bson.h"
#include "../bson/bsonobj.h"
#include "mthMatchRuntime.hpp"
#include "ossMemPool.hpp"
#include "dpsTransLockCallback.hpp"

using namespace bson ;

namespace engine
{

   class _dmsMBContext ;
   class _dmsStorageDataCommon ;
   class _dmsStorageData ;
   class _dmsStorageDataCapped ;
   class _mthMatchTreeContext ;
   class _rtnIXScanner ;
   class _pmdEDUCB ;
   class _monAppCB ;
   class dpsTransCB ;
   class _dpsTransExecutor;

   /*
      _dmsScanner define
   */
   class _dmsScanner : public SDBObject
   {
      public:
         _dmsScanner ( _dmsStorageDataCommon *su, _dmsMBContext *context,
                       mthMatchRuntime *matchRuntime,
                       DMS_ACCESS_TYPE accessType = DMS_ACCESS_TYPE_FETCH,
                       _dpsITransLockCallback * callback = NULL );
         virtual ~_dmsScanner () ;

         BOOLEAN  isReadOnly() const
         {
            return SHARED == _mbLockType ? TRUE : FALSE ;
         }

         void setupTransLockCallback( dmsRecordRW* recordRW );

         BOOLEAN isIdxChanged( dmsExtentID    lid,
                               const BSONObj* value );

         void setNeedWaitForLock( const BOOLEAN w ) { _waitLock = w; }
         const BOOLEAN needWaitForLock() const { return _waitLock ; }

      public:
         virtual INT32 advance ( dmsRecordID &recordID,
                                 _mthRecordGenerator &generator,
                                 _pmdEDUCB *cb,
                                 _mthMatchTreeContext *mthContext = NULL ) = 0 ;
         virtual void  stop () = 0 ;

      protected:
         _dmsStorageDataCommon  *_pSu ;
         _dmsMBContext          *_context ;
         mthMatchRuntime        *_matchRuntime ;
         DMS_ACCESS_TYPE         _accessType ;
         INT32                   _mbLockType ;
         _dpsITransLockCallback *_callback ;
         INT32                   _transIsolation ;
         BOOLEAN                 _waitLock ;
         BOOLEAN                 _useRollbackSegment ;

   } ;
   typedef _dmsScanner dmsScanner ;

   class _dmsTBScanner ;
   /*
      _dmsExtScanner define
   */
   class _dmsExtScannerBase : public _dmsScanner
   {
      friend class _dmsTBScanner ;
      public:
         _dmsExtScannerBase ( _dmsStorageDataCommon *su, _dmsMBContext *context,
                              mthMatchRuntime *matchRuntime,
                              dmsExtentID curExtentID,
                              DMS_ACCESS_TYPE accessType = DMS_ACCESS_TYPE_FETCH,
                              INT64 maxRecords = -1,
                              INT64 skipNum = 0,
                              INT32 flag = 0,
                              _dpsITransLockCallback * callback = NULL ) ;
         virtual ~_dmsExtScannerBase () ;

         const dmsExtent* curExtent () const { return _extent ; }
         dmsExtentID nextExtentID () const ;
         INT32 stepToNextExtent() ;
         INT64 getMaxRecords() const { return _maxRecords ; }
         INT64 getSkipNum () const { return _skipNum ; }

      public:
         virtual INT32 advance ( dmsRecordID &recordID,
                                 _mthRecordGenerator &generator,
                                 _pmdEDUCB *cb,
                                 _mthMatchTreeContext *mhtContext = NULL ) ;
         virtual void  stop () ;

      protected:
         virtual INT32 _firstInit( _pmdEDUCB *cb ) = 0 ;
         virtual INT32 _fetchNext( dmsRecordID &recordID,
                                   _mthRecordGenerator &generator,
                                   _pmdEDUCB *cb,
                                   _mthMatchTreeContext *mhtContext = NULL) = 0 ;
         void _checkMaxRecordsNum( _mthRecordGenerator &generator ) ;

      protected:
         INT64                _maxRecords ;
         INT64                _skipNum ;
         dmsExtRW             _extRW ;
         const dmsExtent      *_extent ;
         dmsRecordID          _curRID ;
         dmsRecordRW          _recordRW ;
         const dmsRecord      *_curRecordPtr ;
         dmsOffset            _next ;
         BOOLEAN              _firstRun ;
         dpsTransCB           *_pTransCB ;
         INT32                _recordLock ;
         BOOLEAN              _needUnLock ;
         BOOLEAN              _selectForUpdate ;
         // indicate if the scanner need to wait for record lock
         //BOOLEAN              _transLockwait;
         _pmdEDUCB            *_cb ;
   };
   typedef _dmsExtScannerBase dmsExtScannerBase ;

   class _dmsExtScanner : public _dmsExtScannerBase
   {
      public:
         _dmsExtScanner( dmsStorageDataCommon *su, _dmsMBContext *context,
                         mthMatchRuntime *matchRuntime,
                         dmsExtentID curExtentID,
                         DMS_ACCESS_TYPE accessType = DMS_ACCESS_TYPE_FETCH,
                         INT64 maxRecords = -1,
                         INT64 skipNum = 0,
                         INT32 flag = 0,
                         _dpsITransLockCallback * callback = NULL ) ;
         virtual ~_dmsExtScanner() ;

      private:
         virtual INT32 _firstInit( _pmdEDUCB *cb ) ;
         virtual INT32 _fetchNext( dmsRecordID &recordID,
                                   _mthRecordGenerator &generator,
                                   _pmdEDUCB *cb,
                                   _mthMatchTreeContext *mhtContext = NULL) ;
   } ;
   typedef _dmsExtScanner dmsExtScanner ;

   class _dmsCappedExtScanner : public _dmsExtScannerBase
   {
      typedef std::pair<dmsExtentID, dmsExtentID>  EXT_LID_PAIR ;
      typedef ossPoolSet<EXT_LID_PAIR>       EXT_RANGE_SET ;
      typedef EXT_RANGE_SET::iterator              EXT_RANGE_SET_ITR ;

      public:
         _dmsCappedExtScanner ( dmsStorageDataCommon *su,
                                _dmsMBContext *context,
                                mthMatchRuntime *matchRuntime,
                                dmsExtentID curExtentID,
                                DMS_ACCESS_TYPE accessType = DMS_ACCESS_TYPE_FETCH,
                                INT64 maxRecords = -1,
                                INT64 skipNum = 0,
                                INT32 flag = 0 ) ;
         virtual ~_dmsCappedExtScanner() ;
         INT64 getMaxRecords() const { return _maxRecords ; }
         INT64 getSkipNum () const { return _skipNum ; }

      public:

         const dmsExtent* curExtent () { return _extent ; }
         dmsExtentID nextExtentID () const ;

      protected:
         virtual INT32 _firstInit( _pmdEDUCB *cb ) ;
         virtual INT32 _fetchNext( dmsRecordID &recordID,
                                   _mthRecordGenerator &generator,
                                   _pmdEDUCB *cb,
                                   _mthMatchTreeContext *mhtContext = NULL) ;

         INT32 _initFastScanRange() ;
         INT32 _validateRange( BOOLEAN &inRange ) ;

         OSS_INLINE dmsExtentID _idToExtLID( INT64 id ) ;

      private:
         dmsOffset               _lastOffset ;
         const _dmsExtentInfo    *_workExtInfo ;
         BOOLEAN                 _rangeInit ;
         BOOLEAN                 _fastScanByID ;
         EXT_RANGE_SET           _rangeSet ;
   } ;
   typedef _dmsCappedExtScanner dmsCappedExtScanner ;

   /*
      _dmsTBScanner define
   */
   class _dmsTBScanner : public _dmsScanner
   {
      public:
         _dmsTBScanner ( _dmsStorageDataCommon *su, _dmsMBContext *context,
                         mthMatchRuntime *matchRuntime,
                         DMS_ACCESS_TYPE accessType = DMS_ACCESS_TYPE_FETCH,
                         INT64 maxRecords = -1,
                         INT64 skipNum = 0,
                         INT32 flag = 0,
                         _dpsITransLockCallback * callback = NULL ) ;
         ~_dmsTBScanner () ;

      public:

         virtual INT32 advance ( dmsRecordID &recordID,
                                 _mthRecordGenerator &generator,
                                 _pmdEDUCB *cb,
                                 _mthMatchTreeContext *mthContext = NULL ) ;
         virtual void  stop () ;

      protected:
         void  _resetExtScanner() ;
         INT32 _firstInit() ;

      private:
         INT32 _getExtScanner() ;

      private:
         dmsExtScannerBase         *_extScanner ;
         dmsExtentID                _curExtentID ;
         BOOLEAN                    _firstRun ;
         INT64                      _maxRecords ;
         INT64                      _skipNum ;
         INT32                      _flag ;
   };
   typedef _dmsTBScanner dmsTBScanner ;

   class _dmsIXScanner ;
   /*
      _dmsIXSecScanner define
      dms index section scanner
   */
   class _dmsIXSecScanner : public _dmsScanner
   {
      friend class _dmsIXScanner ;
      public:
         _dmsIXSecScanner ( dmsStorageDataCommon *su,
                            _dmsMBContext *context,
                            mthMatchRuntime *matchRuntime,
                            _rtnIXScanner *scanner,
                            DMS_ACCESS_TYPE accessType = DMS_ACCESS_TYPE_FETCH,
                            INT64 maxRecords = -1,
                            INT64 skipNum = 0,
                            INT32 flag = 0,
                            _dpsITransLockCallback * callback = NULL ) ;
         virtual ~_dmsIXSecScanner () ;

         void  enableIndexBlockScan( const BSONObj &startKey,
                                     const BSONObj &endKey,
                                     const dmsRecordID &startRID,
                                     const dmsRecordID &endRID,
                                     INT32 direction ) ;

         void  enableCountMode() { _countOnly = TRUE ; }

         INT64 getMaxRecords() const { return _maxRecords ; }
         INT64 getSkipNum () const { return _skipNum ; }

         BOOLEAN eof () const { return _eof ; }

      public:
         virtual INT32 advance ( dmsRecordID &recordID,
                                 _mthRecordGenerator &generator,
                                 _pmdEDUCB *cb,
                                 _mthMatchTreeContext *mhtContext = NULL ) ;
         virtual void  stop () ;

      protected:
         INT32 _firstInit( _pmdEDUCB *cb ) ;
         BSONObj* _getStartKey () ;
         BSONObj* _getEndKey () ;
         dmsRecordID* _getStartRID () ;
         dmsRecordID* _getEndRID () ;
         void _updateMaxRecordsNum( _mthRecordGenerator &generator ) ;

      private:
         INT64                _maxRecords ;
         INT64                _skipNum ;
         dmsRecordID          _curRID ;
         dmsRecordRW          _recordRW ;
         const dmsRecord      *_curRecordPtr ;
         BOOLEAN              _firstRun ;
         dpsTransCB           *_pTransCB ;
         INT8                 _recordLock ;
         BOOLEAN              _needUnLock ;
         BOOLEAN              _selectForUpdate ;
         // indicate if the scanner need to wait for record lock
         //BOOLEAN              _transLockwait;
         _pmdEDUCB            *_cb ;
         _rtnIXScanner        *_scanner ;
         INT64                _onceRestNum ;
         BOOLEAN              _eof ;

         BSONObj              _startKey ;
         BSONObj              _endKey ;
         dmsRecordID          _startRID ;
         dmsRecordID          _endRID ;
         BOOLEAN              _indexBlockScan ;
         INT32                _blockScanDir ;
         BOOLEAN              _judgeStartKey ;
         BOOLEAN              _includeStartKey ;
         BOOLEAN              _includeEndKey ;

         BOOLEAN              _countOnly ;
   } ;
   typedef _dmsIXSecScanner dmsIXSecScanner ;

   /*
      _dmsIXScanner define
   */
   class _dmsIXScanner : public _dmsScanner
   {
      public:
         _dmsIXScanner ( dmsStorageDataCommon *su,
                         _dmsMBContext *context,
                         mthMatchRuntime *matchRuntime,
                         _rtnIXScanner *scanner,
                         BOOLEAN ownedScanner = FALSE,
                         DMS_ACCESS_TYPE accessType = DMS_ACCESS_TYPE_FETCH,
                         INT64 maxRecords = -1,
                         INT64 skipNum = 0,
                         INT32 flag = 0,
                         _dpsITransLockCallback * callback = NULL ) ;
         ~_dmsIXScanner () ;

         _rtnIXScanner* getScanner () { return _scanner ; }

      public:
         virtual INT32 advance ( dmsRecordID &recordID,
                                 _mthRecordGenerator &generator,
                                 _pmdEDUCB *cb,
                                 _mthMatchTreeContext *mthContext = NULL ) ;
         virtual void  stop () ;

      protected:
         void  _resetIXSecScanner() ;

      private:
         dmsIXSecScanner            _secScanner ;
         _rtnIXScanner              *_scanner ;
         BOOLEAN                    _firstRun ;
         BOOLEAN                    _eof ;
         BOOLEAN                    _ownedScanner ;

   } ;
   typedef _dmsIXScanner dmsIXScanner ;

   /*
      _dmsExtentItr define
   */
   class _dmsExtentItr : public SDBObject
   {
      public:
         _dmsExtentItr ( _dmsStorageData *su,
                        _dmsMBContext *context,
                         DMS_ACCESS_TYPE accessType = DMS_ACCESS_TYPE_QUERY,
                         INT32 direction = 1 ) ;
         ~_dmsExtentItr () ;

         void  reset( INT32 direction ) ;

         INT32 getDirection() const { return _direction ; }

      public:
         INT32    next ( dmsExtentID &extentID, _pmdEDUCB *cb ) ;

      private:
         _dmsStorageData            *_pSu ;
         _dmsMBContext              *_context ;
         dmsExtRW                   _extRW ;
         const dmsExtent            *_curExtent ;
         DMS_ACCESS_TYPE            _accessType ;
         UINT32                     _extentCount ;
         INT32                      _direction ;

   } ;
   typedef _dmsExtentItr dmsExtentItr ;

   class _dmsExtScannerFactory : public SDBObject
   {
      public:
         _dmsExtScannerFactory() ;
         ~_dmsExtScannerFactory() ;

         dmsExtScannerBase* create( dmsStorageDataCommon *su,
                                    dmsMBContext *context,
                                    mthMatchRuntime *matchRuntime,
                                    dmsExtentID curExtentID,
                                    DMS_ACCESS_TYPE accessType,
                                    INT64 maxRecords,
                                    INT64 skipNum,
                                    INT32 flag,
                                    _dpsITransLockCallback * callback = NULL ) ;
   } ;
   typedef _dmsExtScannerFactory dmsExtScannerFactory ;

   dmsExtScannerFactory* dmsGetScannerFactory() ;

   // Class to implment lock call back funtions for DMS scanner
   class dmsTransLockCallback : public _dpsITransLockCallback
   {
      public:
      //dmsTransLockCallback() {};
      dmsTransLockCallback( dpsTransCB  * transCB, 
                            _pmdEDUCB   * eduCB,
                            _dmsRecordRW * recordRW ) ;

      virtual ~dmsTransLockCallback() {};

      virtual void beforeLockAcquire( const dpsTransLockId    &lockId,
                                      const INT32              irc,
                                      const DPS_TRANSLOCK_TYPE requestLockMode,
                                 const DPS_TRANSLOCK_OP_MODE_TYPE opMode
                                ) {};
      virtual void afterLockAcquire( const dpsTransLockId     &lockId,
                                           INT32              &irc,
                                     const DPS_TRANSLOCK_TYPE  requestLockMode,
                                     const DPS_TRANSLOCK_OP_MODE_TYPE opMode,
                                           dpsTransRetInfo *pdpsTxResInfo) ;

      virtual void beforeLockRelease( const dpsTransLockId     &lockId,
                                      const DPS_TRANSLOCK_TYPE  lockMode,
                                      const UINT64              refCounter,
                                      oldVersionContainer      *oldVer ) ;

      virtual void afterLockRelease( const dpsTransLockId &lockId ) {};

      void setRecordRW ( dmsRecordRW * recordRW ) { _recordRW = recordRW; }
      
      CHAR * getWorkingArea () { return (CHAR *) _oldVer ; }
      void resetWorkingArea () { _oldVer = NULL ; }
      UINT32 isolationLevel() const { return _isolationLevel ; }
      BOOLEAN lockwaitLevel() const { return _lockwaitLevel ; }

      INT32 saveOldVersionRecord( _dmsRecordRW *recordRW );
      oldVersionCB * getOldVCB() { return _transCB->getOldVCB(); }

      // INT32 saveOldVersionIndexes( );

      private:
      dpsTransCB        * _transCB;   // use it to access global old copy tree
      pmdEDUCB          * _eduCB;
      UINT32              _isolationLevel;
      BOOLEAN              _lockwaitLevel;
      _dpsTransExecutor * _transExecutor;  // use it to access thread local buf
      // DMS related information
      dmsRecordRW       * _recordRW;
      // working area to be setup by callback function so the update can
      // put proper old copy into the area right before the update
      oldVersionContainer *_oldVer;
   };

}

#endif //DMSSCANNER_HPP__

