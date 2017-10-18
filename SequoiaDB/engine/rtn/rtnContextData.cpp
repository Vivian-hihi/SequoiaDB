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

   Source File Name = rtnContextData.cpp

   Descriptive Name = RunTime Data Context

   When/how to use: this program may be used on binary and text-formatted
   versions of Runtime component. This file contains structure for Runtime
   Context.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          5/26/2017   David Li  Split from rtnContext.cpp

   Last Changed =

*******************************************************************************/
#include "rtnContextData.hpp"
#include "rtnContextSort.hpp"
#include "rtn.hpp"
#include "rtnIXScanner.hpp"
#include "dmsScanner.hpp"
#include "dmsStorageUnit.hpp"
#include "pdTrace.hpp"
#include "rtnTrace.hpp"
#include "pmdController.hpp"

namespace engine
{
   /*
      _rtnContextData implement
   */

   RTN_CTX_AUTO_REGISTER(_rtnContextData, RTN_CONTEXT_DATA, "DATA")

   _rtnContextData::_rtnContextData( INT64 contextID, UINT64 eduID )
   :_rtnContextBase( contextID, eduID )
   {
      _dmsCB            = NULL ;
      _su               = NULL ;
      _mbContext        = NULL ;
      _scanType         = UNKNOWNSCAN ;
      _numToReturn      = -1 ;
      _numToSkip        = 0 ;

      _extentID         = DMS_INVALID_EXTENT ;
      _lastExtLID       = DMS_INVALID_EXTENT ;
      _segmentScan      = FALSE ;
      _indexBlockScan   = FALSE ;
      _scanner          = NULL ;
      _direction        = 0 ;
      _queryModifier    = NULL ;
   }

   _rtnContextData::~_rtnContextData ()
   {
      if ( _scanner )
      {
         SDB_OSS_DEL _scanner ;
         _scanner = NULL ;
      }

      // first set activity and release plan
      _planRuntime.setQueryActivity( contextID(), MON_SELECT,
                                     _monCtxCB._startTimestampTick,
                                     _monCtxCB.queryTimeSpent ) ;
      _planRuntime.releasePlan() ;

      // second release mb context
      if ( _mbContext && _su )
      {
         _su->data()->releaseMBContext( _mbContext ) ;
      }
      // last unlock su
      if ( _dmsCB && _su && -1 != contextID() )
      {
         _dmsCB->suUnlock ( _su->CSID() ) ;
      }
      // query modifier
      if ( _queryModifier )
      {
         SDB_OSS_DEL _queryModifier ;
         _queryModifier = NULL ;
         _dmsCB->writeDown( pmdGetThreadEDUCB() ) ;
      }
   }

   std::string _rtnContextData::name() const
   {
      return "DATA" ;
   }

   RTN_CONTEXT_TYPE _rtnContextData::getType() const
   {
      return RTN_CONTEXT_DATA ;
   }

   BOOLEAN _rtnContextData::isWrite() const
   {
      return _queryModifier ? TRUE : FALSE ;
   }

   void _rtnContextData::_toString( stringstream & ss )
   {
      if ( _su && _planRuntime.getPlan() )
      {
         ss << ",Collection:" << _planRuntime.getCLFullName() ;
      }
      ss << ",ScanType:" << ( ( TBSCAN == _scanType ) ? "TBSCAN" : "IXSCAN" ) ;
      if ( _numToReturn > 0 )
      {
         ss << ",NumToReturn:" << _numToReturn ;
      }
      if ( _numToSkip > 0 )
      {
         ss << ",NumToSkip:" << _numToSkip ;
      }
   }

   INT32 _rtnContextData::_openIXScan( dmsStorageUnit *su,
                                       dmsMBContext *mbContext,
                                       pmdEDUCB *cb,
                                       const BSONObj *blockObj,
                                       INT32 direction )
   {
      INT32 rc = SDB_OK ;

      rtnPredicateList *predList = NULL ;

      // for index scan, we maintain context by runtime instead of by DMS
      ixmIndexCB indexCB ( _planRuntime.getIndexCBExtent(), su->index(), NULL ) ;
      if ( !indexCB.isInitialized() )
      {
         PD_LOG ( PDERROR, "unable to get proper index control block" ) ;
         rc = SDB_SYS ;
         goto error ;
      }
      if ( indexCB.getLogicalID() != _planRuntime.getIndexLID() )
      {
         PD_LOG( PDERROR, "Index[extent id: %d] logical id[%d] is not "
                 "expected[%d]", _planRuntime.getIndexCBExtent(),
                 indexCB.getLogicalID(), _planRuntime.getIndexLID() ) ;
         rc = SDB_IXM_NOTEXIST ;
         goto error ;
      }
      // get the predicate list
      predList = _planRuntime.getPredList() ;
      SDB_ASSERT ( predList, "predList can't be NULL" ) ;

      // create scanner
      if ( _scanner )
      {
         SDB_OSS_DEL _scanner ;
      }
      // _scanner should be deleted in context destructor
      _scanner = SDB_OSS_NEW rtnIXScanner ( &indexCB, predList,
                                            su, cb ) ;
      if ( !_scanner )
      {
         PD_LOG ( PDERROR, "Unable to allocate memory for scanner" ) ;
         rc = SDB_OOM ;
         goto error ;
      }
      _scanner->setMonCtxCB ( &_monCtxCB ) ;

      // index block scan
      if ( blockObj )
      {
         SDB_ASSERT( direction == 1 || direction == -1,
                     "direction must be 1 or -1" ) ;

         _direction = direction ;
         rc = _parseIndexBlocks( *blockObj, _indexBlocks, _indexRIDs ) ;
         PD_RC_CHECK( rc, PDERROR, "Parse index blocks failed, rc: %d", rc ) ;
         _indexBlockScan = TRUE ;

         if ( _indexBlocks.size() < 2 )
         {
            _hitEnd = TRUE ;
         }
      }

   done :
      return rc ;
   error :
      goto done ;
   }

   INT32 _rtnContextData::_openTBScan( dmsStorageUnit *su,
                                       dmsMBContext *mbContext,
                                       pmdEDUCB * cb,
                                       const BSONObj *blockObj )
   {
      INT32 rc = SDB_OK ;

      if ( blockObj )
      {
         rc = _parseSegments( *blockObj, _segments ) ;
         PD_RC_CHECK( rc, PDERROR, "Parse segments[%s] failed, rc: %d",
                      blockObj->toString().c_str(), rc ) ;

         _segmentScan = TRUE ;
         _extentID = _segments.size() > 0 ? *_segments.begin() :
                     DMS_INVALID_EXTENT ;
      }
      else
      {
         _extentID = mbContext->mb()->_firstExtentID ;
      }

      if ( DMS_INVALID_EXTENT == _extentID )
      {
         _hitEnd = TRUE ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _rtnContextData::open( dmsStorageUnit *su, dmsMBContext *mbContext,
                                pmdEDUCB *cb,
                                const BSONObj &selector, INT64 numToReturn,
                                INT64 numToSkip,
                                const BSONObj *blockObj,
                                INT32 direction )
   {
      INT32 rc = SDB_OK ;
      BOOLEAN isStictType = FALSE ;

      SDB_ASSERT( su && mbContext, "Invalid param" ) ;
      SDB_ASSERT( _planRuntime.getPlan(), "Invalid plan" ) ;

      if ( _isOpened )
      {
         rc = SDB_DMS_CONTEXT_IS_OPEN ;
         goto error ;
      }

      rc = mbContext->mbLock( SHARED ) ;
      PD_RC_CHECK( rc, PDERROR, "dms mb context lock failed, rc: %d", rc ) ;

      if ( !dmsAccessAndFlagCompatiblity ( mbContext->mb()->_flag,
                                           DMS_ACCESS_TYPE_QUERY ) )
      {
         PD_LOG ( PDERROR, "Incompatible collection mode: %d",
                  mbContext->mb()->_flag ) ;
         rc = SDB_DMS_INCOMPATIBLE_MODE ;
         goto error ;
      }
      if ( OSS_BIT_TEST( mbContext->mb()->_attributes, 
                         DMS_MB_ATTR_STRICTDATAMODE ) )
      {
         isStictType = TRUE ;
      }

      _isOpened = TRUE ;
      _hitEnd = FALSE ;

      if ( TBSCAN == _planRuntime.getScanType() )
      {
         rc = _openTBScan( su, mbContext, cb, blockObj ) ;
         PD_RC_CHECK( rc, PDERROR, "Failed to open tbscan, rc: %d", rc ) ;
      }
      else if ( IXSCAN == _planRuntime.getScanType() )
      {
         rc = _openIXScan( su, mbContext, cb, blockObj, direction ) ;
         PD_RC_CHECK( rc, PDERROR, "Failed to open ixscan, rc: %d", rc ) ;
      }
      else
      {
         rc = SDB_SYS ;
         PD_LOG( PDERROR, "Unknow scan type: %d", _planRuntime.getScanType() ) ;
         goto error ;
      }

      // once context is opened, let's construct matcher and selector
      if ( !selector.isEmpty() )
      {
         try
         {
            rc = _selector.loadPattern ( selector, isStictType ) ;
         }
         catch ( std::exception &e )
         {
            PD_LOG ( PDERROR, "Invalid pattern is detected for select: %s: %s",
                     selector.toString().c_str(), e.what() ) ;
            rc = SDB_INVALIDARG ;
            goto error ;
         }
         PD_RC_CHECK( rc, PDERROR, "Invalid pattern is detected for select: "
                      "%s, rc: %d", selector.toString().c_str(), rc ) ;
      }

      _dmsCB = pmdGetKRCB()->getDMSCB() ;
      _su = su ;
      _mbContext = mbContext ;
      _scanType = _planRuntime.getScanType() ;
      _numToReturn = numToReturn ;
      _numToSkip = numToSkip > 0 ? numToSkip : 0 ;

      if ( 0 == _numToReturn )
      {
         _hitEnd = TRUE ;
      }

   done:
      mbContext->mbUnlock() ;
      return rc ;
   error:
      _isOpened = FALSE ;
      _hitEnd = TRUE ;
      goto done ;
   }

   INT32 _rtnContextData::openTraversal( dmsStorageUnit *su,
                                         dmsMBContext *mbContext,
                                         rtnIXScanner *scanner,
                                         pmdEDUCB *cb,
                                         const BSONObj &selector,
                                         INT64 numToReturn,
                                         INT64 numToSkip )
   {
      INT32 rc = SDB_OK ;
      BOOLEAN strictDataMode = FALSE ;

      SDB_ASSERT( su && mbContext && scanner, "Invalid param" ) ;

      if ( _isOpened )
      {
         rc = SDB_DMS_CONTEXT_IS_OPEN ;
         goto error ;
      }
      if ( IXSCAN != _planRuntime.getScanType() )
      {
         rc = SDB_INVALIDARG ;
         PD_LOG( PDERROR, "Open traversal must IXSCAN" ) ;
         goto error ;
      }

      rc = mbContext->mbLock( SHARED ) ;
      PD_RC_CHECK( rc, PDERROR, "dms mb context lock failed, rc: %d", rc ) ;

      if ( !dmsAccessAndFlagCompatiblity ( mbContext->mb()->_flag,
                                           DMS_ACCESS_TYPE_QUERY ) )
      {
         PD_LOG ( PDERROR, "Incompatible collection mode: %d",
                  mbContext->mb()->_flag ) ;
         rc = SDB_DMS_INCOMPATIBLE_MODE ;
         goto error ;
      }
      if ( OSS_BIT_TEST( mbContext->mb()->_attributes, 
                         DMS_MB_ATTR_STRICTDATAMODE ) )
      {
         strictDataMode = TRUE ;
      }

      if ( _scanner )
      {
         SDB_OSS_DEL _scanner ;
      }
      _scanner = scanner ;

      // once context is opened, let's construct matcher and selector
      if ( !selector.isEmpty() )
      {
         try
         {
            rc = _selector.loadPattern ( selector, strictDataMode ) ;
         }
         catch ( std::exception &e )
         {
            PD_LOG ( PDERROR, "Invalid pattern is detected for select: %s: %s",
                     selector.toString().c_str(), e.what() ) ;
            rc = SDB_INVALIDARG ;
            goto error ;
         }
         PD_RC_CHECK( rc, PDERROR, "Invalid pattern is detected for select: "
                      "%s, rc: %d", selector.toString().c_str(), rc ) ;
      }

      _dmsCB = pmdGetKRCB()->getDMSCB() ;
      _su = su ;
      _mbContext = mbContext ;
      _scanType = _planRuntime.getScanType() ;
      _numToReturn = numToReturn ;
      _numToSkip = numToSkip > 0 ? numToSkip : 0 ;

      _isOpened = TRUE ;
      _hitEnd = FALSE ;

      if ( 0 == _numToReturn )
      {
         _hitEnd = TRUE ;
      }

   done:
      mbContext->mbUnlock() ;
      return rc ;
   error:
      goto done ;
   }

   void _rtnContextData::setQueryModifier ( rtnQueryModifier* modifier )
   {
      SDB_ASSERT( NULL == _queryModifier, "_queryModifier already exists" ) ;

      _queryModifier = modifier ;
   }

   INT32 _rtnContextData::_queryModify( pmdEDUCB* eduCB,
                                        const dmsRecordID& recordID,
                                        ossValuePtr recordDataPtr,
                                        BSONObj& obj )
   {
      INT32 rc = SDB_OK ;

      SDB_ASSERT( NULL != _queryModifier, "_queryModifier can't be null" ) ;

      if ( _queryModifier->isUpdate() )
      {
         BSONObj* newObjPtr = NULL ;

         if ( _queryModifier->returnNew() )
         {
            newObjPtr = &obj ;
         }
         else
         {
            obj = obj.getOwned() ;
         }

         SDB_ASSERT( NULL != _queryModifier->getDollarList(),
                     "dollarList can't be null" ) ;

         rc = _su->data()->updateRecord( _mbContext, recordID,
                                         recordDataPtr, eduCB, getDPSCB(),
                                         _queryModifier->getModifier(),
                                         newObjPtr ) ;
         PD_RC_CHECK( rc, PDERROR, "Update record failed, rc: %d", rc ) ;
         _queryModifier->getDollarList()->clear() ;
      }
      else if ( _queryModifier->isRemove() )
      {
         rc = _su->data()->deleteRecord( _mbContext, recordID,
                                         recordDataPtr, eduCB, getDPSCB() ) ;
         PD_RC_CHECK( rc, PDERROR, "Delete record failed, rc: %d", rc ) ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _rtnContextData::_prepareData( pmdEDUCB *cb )
   {
      vector<INT64>* dollarList = NULL ;
      DMS_ACCESS_TYPE accessType = DMS_ACCESS_TYPE_FETCH ;
      INT32 rc = SDB_OK ;

      if ( _queryModifier )
      {
         if ( _queryModifier->isUpdate() )
         {
            accessType = DMS_ACCESS_TYPE_UPDATE ;
            dollarList = _queryModifier->getDollarList() ;
         }
         else if ( _queryModifier->isRemove() )
         {
            accessType = DMS_ACCESS_TYPE_DELETE ;
         }
         else
         {
            SDB_ASSERT( FALSE, "_queryModifier is invalid" ) ;
            PD_LOG( PDERROR, "_queryModifier is invalid" ) ;
            rc = SDB_SYS ;
            goto error ;
         }
      }

      if ( TBSCAN == _scanType )
      {
         rc = _prepareByTBScan( cb, accessType, dollarList ) ;
      }
      else if ( IXSCAN == _scanType )
      {
         rc = _prepareByIXScan( cb, accessType, dollarList ) ;
      }
      else
      {
         rc = SDB_INVALIDARG ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _rtnContextData::_selectAndAppend( mthSelector *selector,
                                            BSONObj &obj )
   {
      INT32 rc = SDB_OK ;
      BSONObj selObj ;

      if ( selector )
      {
         rc = selector->select( obj, selObj ) ;
         PD_RC_CHECK( rc, PDERROR, "Failed to build select record,"
                      "src obj: %s, rc: %d", obj.toString().c_str(),
                      rc ) ;
      }
      else
      {
         selObj = obj ;
      }

      rc = append( selObj ) ;
      PD_RC_CHECK( rc, PDERROR, "Append obj[%s] failed, rc: %d",
                   selObj.toString().c_str(), rc ) ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _rtnContextData::_innerAppend( mthSelector *selector,
                                        _mthRecordGenerator &generator )
   {
      INT32 rc = SDB_OK ;

      while ( generator.hasNext() )
      {
         BSONObj record ;
         rc = generator.getNext( record ) ;
         PD_RC_CHECK( rc, PDERROR, "get next record failed:rc=%d", rc ) ;

         rc = _selectAndAppend( selector, record ) ;
         PD_RC_CHECK( rc, PDERROR, "selectAndAppend failed:rc=%d", rc ) ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _rtnContextData::_prepareNormalTbScan( pmdEDUCB * cb,
                                                DMS_ACCESS_TYPE accessType,
                                                vector<INT64>* dollarList,
                                                mthMatchRuntime *matchRuntime,
                                                mthSelector *selector )
   {
      INT32 rc = SDB_OK ;
      INT32 startNumRecords = numRecords() ;

      dmsRecordID recordID ;
      ossValuePtr recordDataPtr = 0 ;
      _mthRecordGenerator generator ;
      BOOLEAN hasLocked = _mbContext->isMBLock() ;
      monAppCB *pMonAppCB = cb ? cb->getMonAppCB() : NULL ;

      if ( NULL != _queryModifier )
      {
         generator.setQueryModify( TRUE ) ;
      }

      while ( numRecords() == startNumRecords )
      {
         _mthMatchTreeContext mthContext ;
         if ( NULL != dollarList )
         {
            mthContext.enableDollarList() ;
         }

         // prefetch
         if ( eduID() != cb->getID() && !isOpened() )
         {
            rc = SDB_DMS_CONTEXT_IS_CLOSE ;
            goto error ;
         }

         dmsExtScanner extScanner( _su->data(), _mbContext, matchRuntime,
                                   _extentID, accessType, _numToReturn,
                                   _numToSkip ) ;

         while ( SDB_OK == ( rc = extScanner.advance( recordID, generator,
                                                      cb, &mthContext ) ) )
         {
            try
            {
               generator.getDataPtr( recordDataPtr ) ;
               BSONObj obj( (const CHAR*)recordDataPtr ) ;

               if ( _queryModifier )
               {
                  //dollarList is pointed to _queryModifier->getDollarList()
                  mthContext.getDollarList( dollarList ) ;
                  rc = _queryModify( cb, recordID, recordDataPtr, obj ) ;
                  PD_RC_CHECK( rc, PDERROR, "Failed to query modify" ) ;
                  generator.resetValue( obj, &mthContext ) ;
               }

               rc = _innerAppend( selector, generator ) ;
               PD_RC_CHECK( rc, PDERROR, "innerAppend failed:rc=%d", rc ) ;
            }
            catch ( std::exception &e )
            {
               PD_LOG( PDERROR, "Occur exception: %s", e.what() ) ;
               rc = SDB_SYS ;
               goto error ;
            }
            // increase counter
            DMS_MON_OP_COUNT_INC( pMonAppCB, MON_SELECT, 1 ) ;
            // decrease numToReturn
            if ( _numToReturn > 0 )
            {
               --_numToReturn ;
            }

            //do not clear dollarlist flag
            mthContext.clearRecordInfo() ;
         } // end while

         if ( SDB_DMS_EOC != rc )
         {
            PD_LOG( PDERROR, "Extent scanner failed, rc: %d", rc ) ;
            goto error ;
         }

         _numToReturn = extScanner.getMaxRecords() ;
         _numToSkip   = extScanner.getSkipNum() ;

         if ( 0 == _numToReturn )
         {
            _hitEnd = TRUE ;
            break ;
         }

         if ( _segmentScan )
         {
            if ( DMS_INVALID_EXTENT == extScanner.nextExtentID() ||
                 _su->data()->extent2Segment( *_segments.begin() ) !=
                 _su->data()->extent2Segment( extScanner.nextExtentID() ) )
            {
               _segments.erase( _segments.begin() ) ;
               if ( _segments.size() > 0 )
               {
                  _extentID = *_segments.begin() ;
               }
               else
               {
                  _extentID = DMS_INVALID_EXTENT ;
               }
            }
            else
            {
               _extentID = extScanner.nextExtentID() ;
            }
         }
         else
         {
            _extentID = extScanner.nextExtentID() ;
         }
         _lastExtLID = extScanner.curExtent()->_logicID ;
         if ( DMS_INVALID_EXTENT == _extentID )
         {
            _hitEnd = TRUE ;
            break ;
         }
         if ( !hasLocked )
         {
            _mbContext->pause() ;
         }

      } // end while

      if ( !isEmpty() )
      {
         rc = SDB_OK ;
      }
      else
      {
         rc = SDB_DMS_EOC ;
         goto error ;
      }

   done:
      if ( !hasLocked )
      {
         _mbContext->pause() ;
      }
      return rc ;
   error:
      goto done ;
   }

   INT32 _rtnContextData::_prepareCappedTbScan( pmdEDUCB * cb,
                                                DMS_ACCESS_TYPE accessType,
                                                vector<INT64>* dollarList,
                                                mthMatchRuntime *matchRuntime,
                                                mthSelector *selector )
   {
      INT32 rc = SDB_OK ;
      dmsRecordID recordID ;
      _mthRecordGenerator generator ;
      ossValuePtr recordDataPtr = 0 ;
      INT32 startNumRecords = numRecords();
      BOOLEAN hasLocked = _mbContext->isMBLock() ;
      monAppCB *pMonAppCB = cb ? cb->getMonAppCB() : NULL ;

      if ( DMS_INVALID_EXTENT == _extentID )
      {
         SDB_ASSERT( FALSE, "extentID can't be INVALID" ) ;
         _hitEnd = FALSE ;
         rc = SDB_DMS_EOC ;
         goto error ;
      }

      while ( numRecords() == startNumRecords )
      {
         _mthMatchTreeContext mthContext ;
         dmsCappedExtScanner extScanner( (dmsStorageDataCapped *)_su->data(),
                                         _mbContext, matchRuntime, _extentID,
                                         DMS_ACCESS_TYPE_FETCH, _numToReturn,
                                         _numToSkip ) ;
         while ( SDB_OK == (rc = extScanner.advance( recordID, generator,
                                                     cb, &mthContext ) ) )
         {
            try
            {
               generator.getDataPtr( recordDataPtr ) ;
               BSONObj obj( (const CHAR *)recordDataPtr ) ;

               rc = _innerAppend( selector, generator ) ;
               PD_RC_CHECK( rc, PDERROR, "innerAppend failed, rc: %d", rc ) ;
            }
            catch ( std::exception &e )
            {
               PD_LOG( PDERROR, "Occur exception: %s", e.what() ) ;
               rc = SDB_SYS ;
               goto error ;
            }
            DMS_MON_OP_COUNT_INC( pMonAppCB, MON_SELECT, 1 ) ;
            if ( _numToReturn > 0 )
            {
               --_numToReturn ;
            }

            mthContext.clearRecordInfo() ;
         }

         if ( SDB_DMS_EOC != rc )
         {
            PD_LOG( PDERROR, "Extent scanner failed, rc: %d", rc ) ;
            goto error ;
         }

         _numToReturn = extScanner.getMaxRecords() ;
         _numToSkip   = extScanner.getSkipNum() ;

         if ( 0 == _numToReturn )
         {
            _hitEnd = TRUE ;
            break ;
         }

         _extentID = extScanner.nextExtentID() ;
         _lastExtLID = extScanner.curExtent()->_logicID ;
         if ( DMS_INVALID_EXTENT == _extentID )
         {
            _hitEnd = TRUE ;
            break ;
         }
         if ( !hasLocked )
         {
            _mbContext->pause() ;
         }
      }

      if ( !isEmpty() )
      {
         rc = SDB_OK ;
      }
      else
      {
         rc = SDB_DMS_EOC ;
         goto error ;
      }

   done:
      if ( !hasLocked )
      {
         _mbContext->pause() ;
      }
      return rc ;
   error:
      goto done ;
   }

   INT32 _rtnContextData::_prepareByTBScan ( pmdEDUCB * cb,
                                             DMS_ACCESS_TYPE accessType,
                                             vector<INT64>* dollarList )
   {
      INT32 rc                = SDB_OK ;
      mthMatchRuntime *matchRuntime = _planRuntime.getMatchRuntime( TRUE ) ;
      mthSelector *selector   = NULL ;

      if ( _selector.isInitialized() )
      {
         selector = &_selector ;
      }

      _mthRecordGenerator generator ;

      if ( DMS_INVALID_EXTENT == _extentID )
      {
         SDB_ASSERT( FALSE, "extentID can't be INVALID" ) ;
         _hitEnd = TRUE ;
         rc = SDB_DMS_EOC ;
         goto error ;
      }

      if ( NULL != _queryModifier )
      {
         generator.setQueryModify( TRUE ) ;
      }

      if ( DMS_STORAGE_CAPPED == _su->type() )
      {
         rc = _prepareCappedTbScan( cb, accessType, dollarList,
                                    matchRuntime, selector ) ;
      }
      else
      {
         rc = _prepareNormalTbScan( cb, accessType, dollarList,
                                    matchRuntime, selector ) ;
      }
      if ( rc && SDB_DMS_EOC != rc )
      {
         PD_LOG( PDERROR, "Prepare data failed, rc: %d", rc ) ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _rtnContextData::_prepareByIXScan( pmdEDUCB *cb,
                                            DMS_ACCESS_TYPE accessType,
                                            vector<INT64>* dollarList )
   {
      INT32 rc                   = SDB_OK ;
      rtnIXScanner *scanner      = _scanner ;
      mthMatchRuntime *matchRuntime = _planRuntime.getMatchRuntime( TRUE ) ;
      mthSelector *selector      = NULL ;
      monAppCB * pMonAppCB       = cb ? cb->getMonAppCB() : NULL ;
      BOOLEAN hasLocked          = _mbContext->isMBLock() ;
      INT32 startNumRecords      = numRecords();

      dmsRecordID rid ;
      BSONObj dataRecord ;

      if ( _selector.isInitialized() )
      {
         selector = &_selector ;
      }

      _mthRecordGenerator generator ;
      dmsRecordID recordID ;
      ossValuePtr recordDataPtr = 0 ;

      if ( NULL != _queryModifier )
      {
         generator.setQueryModify( TRUE ) ;
      }

      // loop until we read something in the buffer
      while ( numRecords() == startNumRecords )
      {
         _mthMatchTreeContext mthContext ;
         if ( NULL != dollarList )
         {
            mthContext.enableDollarList() ;
         }

         // prefetch
         if ( eduID() != cb->getID() && !isOpened() )
         {
            rc = SDB_DMS_CONTEXT_IS_CLOSE ;
            goto error ;
         }

         dmsIXSecScanner secScanner( _su->data(), _mbContext, matchRuntime,
                                     scanner, accessType, _numToReturn,
                                     _numToSkip ) ;
         if ( _indexBlockScan )
         {
            secScanner.enableIndexBlockScan( _indexBlocks[0],
                                             _indexBlocks[1],
                                             _indexRIDs[0],
                                             _indexRIDs[1],
                                             _direction ) ;
         }
         if ( isCountMode() )
         {
            secScanner.enableCountMode() ;
         }

         while ( SDB_OK == ( rc = secScanner.advance( recordID, generator,
                                                      cb, &mthContext ) ) )
         {
            if ( !isCountMode() )
            {
               try
               {
                  generator.getDataPtr( recordDataPtr ) ;
                  BSONObj obj( (const CHAR*)recordDataPtr ) ;

                  if ( _queryModifier )
                  {
                     //dollarList is pointed to _queryModifier->getDollarList()
                     mthContext.getDollarList( dollarList ) ;
                     rc = _queryModify( cb, recordID, recordDataPtr, obj ) ;
                     PD_RC_CHECK( rc, PDERROR, "Failed to query modify" ) ;
                     generator.resetValue( obj, &mthContext ) ;
                  }

                  rc = _innerAppend( selector, generator ) ;
                  PD_RC_CHECK( rc, PDERROR, "innerAppend failed:rc=%d", rc ) ;

                  // make sure we still have room to read another
                  // record_max_sz (i.e. 16MB). if we have less than 16MB
                  // to 256MB, we can't safely assume the next record we
                  // read will not overflow the buffer, so let's just break
                  // before reading the next record
                  if ( buffEndOffset() + DMS_RECORD_MAX_SZ >
                       RTN_RESULTBUFFER_SIZE_MAX )
                  {
                     secScanner.stop () ;
                     // let's break if there's no room for another max record
                     break ;
                  }
               }
               catch ( std::exception &e )
               {
                  PD_LOG( PDERROR, "Occur exception: %s", e.what() ) ;
                  rc = SDB_SYS ;
                  goto error ;
               }
               // increase counter
               DMS_MON_OP_COUNT_INC( pMonAppCB, MON_SELECT, 1 ) ;
            }
            else
            {
               static BSONObj dummyObj ;
               rc = append( dummyObj ) ;
               PD_RC_CHECK( rc, PDERROR, "Append empty obj failed, rc: %d",
                            rc ) ;
            }

            //do not clear dollarlist flag
            mthContext.clearRecordInfo() ;
         }

         if ( rc && SDB_DMS_EOC != rc )
         {
            PD_LOG( PDERROR, "Extent scanner failed, rc: %d", rc ) ;
            goto error ;
         }

         _numToReturn = secScanner.getMaxRecords() ;
         _numToSkip   = secScanner.getSkipNum() ;

         if ( 0 == _numToReturn )
         {
            _hitEnd = TRUE ;
            break ;
         }

         if ( secScanner.eof() )
         {
            if ( _indexBlockScan )
            {
               _indexBlocks.erase( _indexBlocks.begin() ) ;
               _indexBlocks.erase( _indexBlocks.begin() ) ;
               _indexRIDs.erase( _indexRIDs.begin() ) ;
               _indexRIDs.erase( _indexRIDs.begin() ) ;
               if ( _indexBlocks.size() < 2 )
               {
                  _hitEnd = TRUE ;
                  break ;
               }
            }
            else
            {
               _hitEnd = TRUE ;
               break ;
            }
         }

         if ( !hasLocked )
         {
            _mbContext->pause() ;
         }
      } // end while

      if ( !isEmpty() )
      {
         rc = SDB_OK ;
      }
      else
      {
         rc = SDB_DMS_EOC ;
         goto error ;
      }

   done :
      if ( !hasLocked )
      {
         _mbContext->pause() ;
      }
      return rc ;
   error :
      goto done ;
   }

   INT32 _rtnContextData::_parseSegments( const BSONObj &obj,
                                          vector< dmsExtentID > &segments )
   {
      INT32 rc = SDB_OK ;
      BSONElement ele ;
      segments.clear() ;

      BSONObjIterator it ( obj ) ;
      while ( it.more() )
      {
         ele = it.next() ;
         if ( NumberInt != ele.type() )
         {
            PD_LOG( PDWARNING, "Datablocks[%s] value type[%d] is not NumberInt",
                    obj.toString().c_str(), ele.type() ) ;
            rc = SDB_INVALIDARG ;
            goto error ;
         }
         segments.push_back( ele.numberInt() ) ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _rtnContextData::_parseRID( const BSONElement & ele,
                                     dmsRecordID & rid )
   {
      INT32 rc = SDB_OK ;
      rid.reset() ;

      if ( ele.eoo() )
      {
         goto done ;
      }
      else if ( Array != ele.type() )
      {
         rc = SDB_INVALIDARG ;
         PD_LOG( PDWARNING, "Field[%s] type is not Array",
                 ele.toString().c_str() ) ;
         goto error ;
      }
      else
      {
         UINT32 count = 0 ;
         BSONElement ridEle ;
         BSONObjIterator it( ele.embeddedObject() ) ;
         while ( it.more() )
         {
            ridEle = it.next() ;
            if ( NumberInt != ridEle.type() )
            {
               rc = SDB_INVALIDARG ;
               PD_LOG( PDERROR, "RID type is not NumberInt in field[%s]",
                       ele.toString().c_str() ) ;
               goto error ;
            }
            if ( 0 == count )
            {
               rid._extent = ridEle.numberInt() ;
            }
            else if ( 1 == count )
            {
               rid._offset = ridEle.numberInt() ;
            }

            ++count ;
         }

         if ( 2 != count )
         {
            rc = SDB_INVALIDARG ;
            PD_LOG( PDERROR, "RID array size[%d] is not 2", count ) ;
            goto error ;
         }
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _rtnContextData::_parseIndexBlocks( const BSONObj &obj,
                                             vector< BSONObj > &indexBlocks,
                                             vector< dmsRecordID > &indexRIDs )
   {
      INT32 rc = SDB_OK ;
      BSONElement ele ;
      BSONObj indexObj ;
      BSONObj startKey, endKey ;
      dmsRecordID startRID, endRID ;

      indexBlocks.clear() ;
      indexRIDs.clear() ;

      BSONObjIterator it ( obj ) ;
      while ( it.more() )
      {
         ele = it.next() ;
         if ( Object != ele.type() )
         {
            PD_LOG( PDWARNING, "Indexblocks[%s] value type[%d] is not Object",
                    obj.toString().c_str(), ele.type() ) ;
            rc = SDB_INVALIDARG ;
            goto error ;
         }
         indexObj = ele.embeddedObject() ;
         // StartKey
         rc = rtnGetObjElement( indexObj, FIELD_NAME_STARTKEY, startKey ) ;
         PD_RC_CHECK( rc, PDWARNING, "Failed to get field[%s] from obj[%s], "
                      "rc: %d", FIELD_NAME_STARTKEY,
                      indexObj.toString().c_str(), rc ) ;
         // EndKey
         rc = rtnGetObjElement( indexObj, FIELD_NAME_ENDKEY, endKey ) ;
         PD_RC_CHECK( rc, PDWARNING, "Failed to get field[%s] from obj[%s], "
                      "rc: %d", FIELD_NAME_ENDKEY,
                      indexObj.toString().c_str(), rc ) ;
         // StartRID
         rc = _parseRID( indexObj.getField( FIELD_NAME_STARTRID ), startRID ) ;
         PD_RC_CHECK( rc, PDWARNING, "Failed to parse %s, rc: %d",
                      FIELD_NAME_STARTRID, rc ) ;

         // EndRID
         rc = _parseRID( indexObj.getField( FIELD_NAME_ENDRID ), endRID ) ;
         PD_RC_CHECK( rc, PDWARNING, "Failed to parse %s, rc: %d",
                      FIELD_NAME_ENDRID, rc ) ;

         indexBlocks.push_back( rtnNullKeyNameObj( startKey ).getOwned() ) ;
         indexBlocks.push_back( rtnNullKeyNameObj( endKey ).getOwned() ) ;

         indexRIDs.push_back( startRID ) ;
         indexRIDs.push_back( endRID ) ;
      }

      if ( indexBlocks.size() != indexRIDs.size() )
      {
         rc = SDB_SYS ;
         PD_LOG( PDERROR, "block array size is not the same with rid array" ) ;
         goto error ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   /*
      _rtnContextParaData implement
   */

   RTN_CTX_AUTO_REGISTER(_rtnContextParaData, RTN_CONTEXT_PARADATA, "PARADATA")

   _rtnContextParaData::_rtnContextParaData( INT64 contextID, UINT64 eduID )
   :_rtnContextData( contextID, eduID )
   {
      _isParalled = FALSE ;
      _curIndex   = 0 ;
      _step       = 1 ;
   }

   _rtnContextParaData::~_rtnContextParaData()
   {
      vector< rtnContextData* >::iterator it = _vecContext.begin() ;
      while ( it != _vecContext.end() )
      {
         (*it)->_close () ;
         ++it ;
      }
      it = _vecContext.begin() ;
      while ( it != _vecContext.end() )
      {
         (*it)->waitForPrefetch() ;
         SDB_OSS_DEL (*it) ;
         ++it ;
      }
      _vecContext.clear () ;
   }

   std::string _rtnContextParaData::name() const
   {
      return "PARADATA" ;
   }

   RTN_CONTEXT_TYPE _rtnContextParaData::getType () const
   {
      return RTN_CONTEXT_PARADATA ;
   }

   INT32 _rtnContextParaData::open( dmsStorageUnit *su, dmsMBContext *mbContext,
                                    pmdEDUCB *cb,
                                    const BSONObj &selector, INT64 numToReturn,
                                    INT64 numToSkip, const BSONObj *blockObj,
                                    INT32 direction )
   {
      INT32 rc = SDB_OK ;
      _step = pmdGetKRCB()->getOptionCB()->maxSubQuery() ;
      if ( 0 == _step )
      {
         _step = 1 ;
      }

      rc = _rtnContextData::open( su, mbContext, cb, selector,
                                  numToReturn, numToSkip, blockObj,
                                  direction ) ;
      if ( rc )
      {
         goto error ;
      }

      if ( eof() )
      {
         goto done ;
      }

      if ( TBSCAN == _scanType && FALSE == _segmentScan )
      {
         rc = _su->getSegExtents( NULL, _segments, mbContext ) ;
         PD_RC_CHECK( rc, PDERROR, "Failed to get segment extent, rc: %d",
                      rc ) ;
         if ( _segments.size() <= 1 )
         {
            _segments.clear() ;
            goto done ;
         }
         _segmentScan = TRUE ;
      }
      else if ( IXSCAN == _scanType && FALSE == _indexBlockScan )
      {
         rc = rtnGetIndexSeps( &_planRuntime, su, mbContext, cb, _indexBlocks,
                               _indexRIDs ) ;
         PD_RC_CHECK( rc, PDERROR, "Failed to get index seperations, rc: %d",
                      rc ) ;
         if ( _indexBlocks.size() <= 2 )
         {
            _indexBlocks.clear() ;
            _indexRIDs.clear() ;
            goto done ;
         }
         _indexBlockScan = TRUE ;
         _direction = 1 ;
      }

      if ( ( _segmentScan && _segments.size() <= 1 ) ||
           ( _indexBlockScan && _indexBlocks.size() <= 2 ) )
      {
         goto done ;
      }

      _isParalled = TRUE ;
      mbContext->mbUnlock() ;

      if ( numToReturn > 0 && numToSkip > 0 )
      {
         numToReturn += numToSkip ;
      }

      while ( NULL != ( blockObj = _nextBlockObj() ) )
      {
         rc = _openSubContext( blockObj, selector, cb, numToReturn ) ;
         if ( rc )
         {
            goto error ;
         }
      }

      _checkAndPrefetch () ;

   done:
      mbContext->mbUnlock() ;
      return rc ;
   error:
      goto done ;
   }

   void _rtnContextParaData::_removeSubContext( rtnContextData *pContext )
   {
      vector< rtnContextData* >::iterator it = _vecContext.begin() ;
      while ( it != _vecContext.end() )
      {
         if ( *it == pContext )
         {
            pContext->waitForPrefetch() ;
            SDB_OSS_DEL pContext ;
            _vecContext.erase( it ) ;
            break ;
         }
         ++it ;
      }
   }

   INT32 _rtnContextParaData::_openSubContext( const BSONObj *blockObj,
                                               const BSONObj &selector,
                                               _pmdEDUCB *cb,
                                               INT64 numToReturn )
   {
      INT32 rc = SDB_OK ;

      dmsMBContext *mbContext = NULL ;
      rtnContextData *dataContext = NULL ;

      rc = _su->data()->getMBContext( &mbContext, _planRuntime.getCLMBID(),
                                      DMS_INVALID_CLID, -1 ) ;
      PD_RC_CHECK( rc, PDERROR, "Failed to get dms mb context, rc: %d", rc ) ;
      PD_CHECK( _planRuntime.getCLLID() == mbContext->clLID(), SDB_DMS_NOTEXIST,
                error, PDERROR, "Failed to get dms mb context, rc: %d",
                SDB_DMS_NOTEXIST ) ;

      // create a new context
      dataContext = SDB_OSS_NEW rtnContextData( -1, eduID() ) ;
      if ( !dataContext )
      {
         rc = SDB_OOM ;
         PD_LOG( PDERROR, "Alloc sub context out of memory" ) ;
         goto error ;
      }
      _vecContext.push_back( dataContext ) ;

      dataContext->getPlanRuntime()->inheritRuntime( &_planRuntime ) ;

      rc = dataContext->open( _su, mbContext, cb, selector,
                              numToReturn, 0, blockObj, _direction ) ;
      PD_RC_CHECK( rc, PDERROR, "Open sub context failed, blockObj: %s, "
                   "rc: %d", blockObj->toString().c_str(), rc ) ;

      mbContext = NULL ;

      dataContext->enablePrefetch ( cb, &_prefWather ) ;
      // sample timetamp
      if ( cb->getMonConfigCB()->timestampON )
      {
         dataContext->getMonCB()->recordStartTimestamp() ;
      }
      dataContext->getSelector().setStringOutput(
         getSelector().getStringOutput() ) ;

   done :
      return rc ;
   error :
      if ( mbContext )
      {
         _su->data()->releaseMBContext( mbContext ) ;
      }
      goto done ;
   }

   INT32 _rtnContextParaData::_checkAndPrefetch ()
   {
      INT32 rc = SDB_OK ;
      rtnContextData *pContext = NULL ;
      vector< rtnContextData* >::iterator it = _vecContext.begin() ;
      while ( it != _vecContext.end() )
      {
         pContext = *it ;
         if ( pContext->eof() && pContext->isEmpty() )
         {
            pContext->waitForPrefetch() ;
            SDB_OSS_DEL pContext ;
            it = _vecContext.erase( it ) ;
            continue ;
         }
         else if ( !pContext->isEmpty() ||
                   pContext->_getWaitPrefetchNum() > 0 )
         {
            ++it ;
            continue ;
         }
         pContext->_onDataEmpty() ;
         ++it ;
      }

      if ( _vecContext.size() == 0 )
      {
         rc = SDB_DMS_EOC ;
         _hitEnd = TRUE ;
      }

      return rc ;
   }

   const BSONObj* _rtnContextParaData::_nextBlockObj ()
   {
      BSONArrayBuilder builder ;
      UINT32 curIndex = _curIndex ;

      if ( _curIndex >= _step ||
           ( TBSCAN == _scanType && _curIndex >= _segments.size() ) ||
           ( IXSCAN == _scanType && _curIndex + 1 >= _indexBlocks.size() ) )
      {
         return NULL ;
      }
      ++_curIndex ;

      if ( TBSCAN == _scanType )
      {
         while ( curIndex < _segments.size() )
         {
            builder.append( _segments[curIndex] ) ;
            curIndex += _step ;
         }
      }
      else if ( IXSCAN == _scanType )
      {
         while ( curIndex + 1 < _indexBlocks.size() )
         {
            builder.append( BSON( FIELD_NAME_STARTKEY <<
                                  _indexBlocks[curIndex] <<
                                  FIELD_NAME_ENDKEY <<
                                  _indexBlocks[curIndex+1] <<
                                  FIELD_NAME_STARTRID <<
                                  BSON_ARRAY( _indexRIDs[curIndex]._extent <<
                                              _indexRIDs[curIndex]._offset ) <<
                                  FIELD_NAME_ENDRID <<
                                  BSON_ARRAY( _indexRIDs[curIndex+1]._extent <<
                                              _indexRIDs[curIndex+1]._offset )
                                 )
                            ) ;
            curIndex += _step ;
         }
      }
      else
      {
         return NULL ;
      }

      _blockObj = builder.arr() ;
      return &_blockObj ;
   }

   INT32 _rtnContextParaData::_getSubCtxWithData( rtnContextData **ppContext,
                                                  _pmdEDUCB *cb )
   {
      INT32 rc = SDB_OK ;
      UINT32 index = 0 ;

      do
      {
         index = 0 ;
         _prefWather.reset() ;

         while ( index < _vecContext.size() )
         {
            rc = _vecContext[index]->prefetchResult () ;
            if ( rc && SDB_DMS_EOC != rc )
            {
               goto error ;
            }
            rc = SDB_OK ;

            if ( !_vecContext[index]->isEmpty() &&
                 !_vecContext[index]->_isInPrefetching () )
            {
               *ppContext = _vecContext[index] ;
               goto done ;
            }
            ++index ;
         }
      } while ( _prefWather.waitDone( OSS_ONE_SEC * 5 ) > 0 ) ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _rtnContextParaData::_getSubContextData( pmdEDUCB * cb )
   {
      INT32 rc = SDB_OK ;
      rtnContextData *pContext = NULL ;
      INT64 maxReturnNum = -1 ;
      INT32 startNumRecords = numRecords();

      while ( numRecords() == startNumRecords && 0 != _numToReturn )
      {
         pContext = NULL ;
         if ( cb->isInterrupted() )
         {
            rc = SDB_APP_INTERRUPT ;
            goto error ;
         }

         if ( _numToSkip <= 0 )
         {
            rc = _getSubCtxWithData( &pContext, cb ) ;
            if ( rc )
            {
               goto error ;
            }
         }

         if ( !pContext && _vecContext.size() > 0 )
         {
            pContext = _vecContext[0] ;
         }

         // get data
         if ( pContext )
         {
            rtnContextBuf buffObj ;
            if ( _numToSkip > 0 )
            {
               maxReturnNum = _numToSkip ;
            }
            else
            {
               maxReturnNum = -1 ;
            }

            // get data
            rc = pContext->getMore( maxReturnNum, buffObj, cb ) ;
            if ( rc )
            {
               _removeSubContext( pContext ) ;
               if ( SDB_DMS_EOC != rc )
               {
                  PD_LOG( PDERROR, "Failed to get more from sub context, "
                          "rc: %d", rc ) ;
                  goto error ;
               }
               continue ;
            }

            if ( _numToSkip > 0 )
            {
               _numToSkip -= buffObj.recordNum() ;
               continue ;
            }

            if ( _numToReturn > 0 && buffObj.recordNum() > _numToReturn )
            {
               buffObj.truncate( _numToReturn ) ;
            }
            // append data
            rc = appendObjs( buffObj.data(), buffObj.size(),
                             buffObj.recordNum() ) ;
            PD_RC_CHECK( rc, PDERROR, "Failed to add objs, rc: %d", rc ) ;
            if ( _numToReturn > 0 )
            {
               _numToReturn -= buffObj.recordNum() ;
            }
         } // end if ( pContext )

         if ( SDB_OK != _checkAndPrefetch() )
         {
            break ;
         }
      } // while ( isEmpty() && 0 != _numToReturn )

      if ( 0 == _numToReturn )
      {
         _hitEnd = TRUE ;
      }

      if ( !isEmpty() )
      {
         rc = SDB_OK ;
      }
      else
      {
         rc = SDB_DMS_EOC ;
         goto error ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _rtnContextParaData::_prepareData( pmdEDUCB * cb )
   {
      if ( !_isParalled )
      {
         return _rtnContextData::_prepareData( cb ) ;
      }
      else
      {
         return _getSubContextData( cb ) ;
      }
   }

   RTN_CTX_AUTO_REGISTER(_rtnContextSort, RTN_CONTEXT_SORT, "SORT")

   _rtnContextSort::_rtnContextSort( INT64 contextID, UINT64 eduID )
   :_rtnContextBase( contextID, eduID ),
    _dataContext( NULL ),
    _eduCB( NULL ),
    _keyGen ( BSONObj() ),
    _dataSorted ( FALSE ),
    _skip( 0 ),
    _limit( -1 ),
    _planForExplain( NULL )
   {

   }

   _rtnContextSort::~_rtnContextSort()
   {
      _skip = 0 ;
      _limit = 0 ;
      _planForExplain = NULL ;

      if ( NULL != _dataContext )
      {
         pmdGetKRCB()->getRTNCB()->contextDelete( _dataContext->contextID(), _eduCB ) ;
         _dataContext = NULL ;
      }

      _eduCB = NULL ;
   }

   std::string _rtnContextSort::name() const
   {
      return "SORT" ;
   }

   RTN_CONTEXT_TYPE _rtnContextSort::getType() const
   {
      return RTN_CONTEXT_SORT ;
   }

   INT32 _rtnContextSort::open( const BSONObj &orderby,
                                rtnContext *context,
                                pmdEDUCB *cb,
                                SINT64 numToSkip,
                                SINT64 numToReturn )
   {
      SDB_ASSERT( !orderby.isEmpty(), "impossible" ) ;
      SDB_ASSERT( NULL != cb, "possible" ) ;
      SDB_ASSERT( NULL != context, "impossible" ) ;
      INT32 rc = SDB_OK ;
      UINT64 sortBufSz = pmdGetOptionCB()->getSortBufSize() ;
      SINT64 limit = numToReturn ;

      if ( 0 < limit && 0 < numToSkip )
      {
         limit += numToSkip ;
      }

      rc = _sorting.init( sortBufSz, orderby,
                          contextID(), limit, cb ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to init sort:%d", rc ) ;
         goto error ;
      }

      _isOpened = TRUE ;
      _hitEnd = FALSE ;
      _skip = numToSkip ;
      _limit = numToReturn ;

      rc = _rebuildSrcContext( orderby, context ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to rebuild src context:%d", rc ) ;
         goto error ;
      }

      if ( RTN_CONTEXT_DATA == context->getType() )
      {
         /// WARNING: do not use this plan to do anything
         ///  except keeping plan for explain. -- yunwu.
         _planForExplain = ( ( _rtnContextData * )context )->getPlanRuntime() ;
      }

      _dataContext = context ;
      _eduCB = cb ;
      _orderby = orderby.getOwned() ;
      _keyGen = _ixmIndexKeyGen( _orderby ) ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _rtnContextSort::_rebuildSrcContext( const BSONObj &orderBy,
                                              rtnContext *srcContext )
   {
      INT32 rc = SDB_OK ;
      const BSONObj &selector = srcContext->getSelector().getPattern() ;
      if ( selector.isEmpty() )
      {
         goto done ;
      }
      else
      {
         BOOLEAN needRebuild = FALSE ;
         rtnNeedResetSelector( selector, orderBy, needRebuild ) ;
         if ( needRebuild )
         {
            rc = srcContext->getSelector().move( _selector ) ;
            if ( SDB_OK != rc )
            {
               PD_LOG( PDERROR, "failed to rebuild selector:%d", rc ) ;
               goto error ;
            }
         }
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _rtnContextSort::_sortData( _pmdEDUCB *cb )
   {
      INT32 rc = SDB_OK ;
      rtnContextBuf bufObj ;
      BSONObj obj ;

      for(;;)
      {
         rc = _dataContext->getMore( -1, bufObj, cb ) ;
         if ( SDB_DMS_EOC == rc )
         {
            rc = _sorting.sort( cb ) ;
            if ( SDB_OK != rc )
            {
               PD_LOG( PDERROR, "failed to sort: %d", rc ) ;
               goto error ;
            }
            break ;
         }
         else if ( SDB_OK != rc )
         {
            PD_LOG( PDERROR, "failed to getmore:%d", rc ) ;
            goto error ;
         }

         while ( SDB_OK == ( rc = bufObj.nextObj( obj ) ) )
         {
            BSONElement arrEle ;
            BSONObjSet keySet( _orderby ) ;
            rc = _keyGen.getKeys( obj, keySet, &arrEle ) ;
            if ( SDB_OK != rc )
            {
               PD_LOG( PDERROR, "failed gen sort keys:%d", rc ) ;
               goto error ;
            }

            SDB_ASSERT( !keySet.empty(), "can not be empty" ) ;
            const BSONObj &keyObj = *(keySet.begin() ) ;

            rc = _sorting.push( keyObj,
                                obj.objdata(), obj.objsize(),
                                &arrEle, cb ) ;
            if ( SDB_OK != rc )
            {
               PD_LOG( PDERROR, "failed to push obj: %d", rc ) ;
               goto error ;
            }
         }

         if ( SDB_DMS_EOC != rc )
         {
            PD_LOG( PDERROR, "failed to get next obj from objBuf: %d", rc ) ;
            goto error ;
         }
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _rtnContextSort::_prepareData( _pmdEDUCB *cb )
   {
      INT32 rc = SDB_OK ;

      const INT32 maxNum = 1000000 ;
      const INT32 breakBufferSize = 2097152 ; /// 2MB
      const INT32 minRecordNum = 4 ;
      BSONObj key ;
      BSONObj obj ;
      monAppCB *pMonAppCB = cb ? cb->getMonAppCB() : NULL ;

      if ( 0 == _limit )
      {
         _hitEnd = TRUE ;
         rc = SDB_DMS_EOC ;
         goto error ;
      }

      if ( !_dataSorted )
      {
         rc = _sortData( cb ) ;
         if ( SDB_OK != rc )
         {
            PD_LOG( PDERROR, "failed to sort data:%d", rc ) ;
            goto error ;
         }
         _dataSorted = TRUE ;
      }

      for ( INT32 i = 0; i < maxNum; i++ )
      {
         const CHAR* objdata ;
         INT32 objlen ;
         rc = _sorting.fetch( key, &objdata, &objlen, cb ) ;
         if ( SDB_DMS_EOC == rc )
         {
            _hitEnd = TRUE ;
            break ;
         }
         else if ( SDB_OK != rc )
         {
            PD_LOG( PDERROR, "failed to fetch from sorting:%d", rc ) ;
            goto error ;
         }
         else if ( 0 < _skip )
         {
            --_skip ;
            /// wo do not want to break this loop when get nothing.
            --i ;
            continue ;
         }
         else if ( 0 == _limit )
         {
            _hitEnd = TRUE ;
            break ;
         }
         else
         {
            const BSONObj *record = NULL ;
            BSONObj selected ;
            obj = BSONObj( objdata ) ;
            if ( _selector.isInitialized() )
            {
               rc = _selector.select( obj, selected ) ;
               if ( SDB_OK != rc )
               {
                  PD_LOG( PDERROR, "failed to select fields from obj:%d", rc ) ;
                  goto error ;
               }
               record = &selected ;
            }
            else
            {
               record = &obj ;
            }

            rc = append( *record ) ;
            PD_RC_CHECK( rc, PDERROR, "Append obj[%s] failed, rc: %d",
                      obj.toString().c_str(), rc ) ;

            if ( 0 < _limit )
            {
               --_limit ;
            }
         }

         DMS_MON_OP_COUNT_INC( pMonAppCB, MON_SELECT, 1 ) ;

         if ( minRecordNum <= i && buffEndOffset() >= breakBufferSize )
         {
            break ;
         }

         if ( buffEndOffset() + DMS_RECORD_MAX_SZ > RTN_RESULTBUFFER_SIZE_MAX )
         {
            break ;
         }
      }

      if ( SDB_OK != rc )
      {
         goto error ;
      }
      else if ( !isEmpty() )
      {
         rc = SDB_OK ;
      }
      else
      {
         rc = SDB_DMS_EOC ;
         goto error ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   void _rtnContextSort::_toString( stringstream &ss )
   {
      if ( _limit > 0 )
      {
         ss << ",NumToReturn:" << _limit ;
      }
      if ( _skip > 0 )
      {
         ss << ",NumToSkip:" << _skip ;
      }
      if ( !_orderby.isEmpty() )
      {
         ss << ",Orderby:" << _orderby.toString().c_str() ;
      }
   }

   /*
      _rtnContextTemp implement
   */

   RTN_CTX_AUTO_REGISTER(_rtnContextTemp, RTN_CONTEXT_TEMP, "TEMP")

   _rtnContextTemp::_rtnContextTemp( INT64 contextID, UINT64 eduID )
   :_rtnContextData( contextID, eduID )
   {
   }

   _rtnContextTemp::~_rtnContextTemp ()
   {
      // release temp collection
      if ( _dmsCB && _mbContext )
      {
         _dmsCB->getTempSUMgr()->release( _mbContext ) ;
      }
   }

   std::string _rtnContextTemp::name() const
   {
      return "TEMP" ;
   }

   RTN_CONTEXT_TYPE _rtnContextTemp::getType () const
   {
      return RTN_CONTEXT_TEMP ;
   }

   RTN_CTX_AUTO_REGISTER(_rtnContextExplain, RTN_CONTEXT_EXPLAIN, "EXPLAIN")

   _rtnContextExplain::_rtnContextExplain( INT64 contextID,
                                           UINT64 eduID )
   :_rtnContextBase( contextID, eduID ),
    _queryContextID( -1 ),
    _recordNum( 0 ),
    _cbOfQuery( NULL ),
    _explained( FALSE )
   {
      _needRun = FALSE ;
      _needDetail = FALSE ;
      _beginUsrCpu = 0.0 ;
      _beginSysCpu = 0.0 ;
      _endUsrCpu = 0.0 ;
      _endSysCpu = 0.0 ;
   }

   _rtnContextExplain::~_rtnContextExplain()
   {
      if ( -1 != _queryContextID )
      {
         sdbGetRTNCB()->contextDelete( _queryContextID,
                                       _cbOfQuery ) ;
         _queryContextID = -1 ;
         _cbOfQuery = NULL ;
      }
   }

   std::string _rtnContextExplain::name() const
   {
      return "EXPLAIN" ;
   }

   INT32 _rtnContextExplain::open( const rtnQueryOptions &options,
                                   const BSONObj &explainOptions )
   {
      INT32 rc = SDB_OK ;
      if ( _isOpened )
      {
         rc = SDB_DMS_CONTEXT_IS_OPEN ;
         goto error ;
      }

      _options = options ;
      rc = _options.getOwned() ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "options failed to get owned:%d", rc ) ;
         goto error ;
      }

      try
      {
         BSONElement e = explainOptions.getField( FIELD_NAME_RUN ) ;
         if ( e.eoo() )
         {
            _needRun = FALSE ;
         }
         else if ( e.isNumber() )
         {
            _needRun = e.numberInt() == 0 ? FALSE : TRUE ;
         }
         else if ( e.isBoolean() )
         {
            _needRun = e.booleanSafe() ;
         }
         else
         {
            _needRun = FALSE ;
         }

         e = explainOptions.getField( FIELD_NAME_DETAIL ) ;
         if ( e.eoo() )
         {
            _needDetail = FALSE ;
         }
         else if ( e.isNumber() )
         {
            _needDetail = e.numberInt() == 0 ? FALSE : TRUE ;
         }
         else if ( e.isBoolean() )
         {
            _needDetail = e.booleanSafe() ;
         }
         else
         {
            _needDetail = FALSE ;
         }
      }
      catch( std::exception &e )
      {
         PD_LOG( PDERROR, "Ocurr exception: %s", e.what() ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }

      _isOpened = TRUE ;
      _hitEnd = FALSE ;
   done:
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_RTNCONTEXTEXPLAIN__PREPAREDATA, "_rtnContextExplain::_prepareData" )
   INT32 _rtnContextExplain::_prepareData( _pmdEDUCB *cb )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB_RTNCONTEXTEXPLAIN__PREPAREDATA ) ;

      if ( _explained )
      {
         rc = SDB_DMS_EOC ;
         goto error ;
      }

      rc = _prepareToExplain( cb ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to prepare for explaining:%d", rc ) ;
         goto error ;
      }

      rc = _explainQuery( cb ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to explain query:%d", rc ) ;
         goto error ;
      }

      rc = _commitResult( cb ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to commit result:%d", rc ) ;
         goto error ;
      }

      _explained = TRUE ;
      _hitEnd    = TRUE ;

   done:
      PD_TRACE_EXITRC( SDB_RTNCONTEXTEXPLAIN__PREPAREDATA, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   void _rtnContextExplain::_toString( stringstream &ss )
   {
      ss << ",NeedRun:" << _needRun ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_RTNCONTEXTEXPLAIN__PREPARETOEXPLAIN, "_rtnContextExplain::_prepareToExplain" )
   INT32 _rtnContextExplain::_prepareToExplain( _pmdEDUCB *cb )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB_RTNCONTEXTEXPLAIN__PREPARETOEXPLAIN ) ;
      BSONObj dummy ;
      INT64 queryContextID = -1 ;
      rtnContextBuf ctxBuf ;
      optAccessPlanRuntime *planRuntime = NULL ;
      const CHAR* hostName = NULL ;
      stringstream ss ;
      _rtnContextBase *contextOfQuery = NULL ;
      ossTime userTime, sysTime ;

      rc = rtnQuery( _options, cb, sdbGetDMSCB(), sdbGetRTNCB(),
                     queryContextID, &contextOfQuery ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to query data:%d", rc ) ;
         goto error ;
      }

      if ( NULL == contextOfQuery )
      {
         PD_LOG( PDERROR, "can not get the context of query" ) ;
         rc = SDB_SYS ;
         goto error ;
      }

      planRuntime = contextOfQuery->getPlanRuntime() ;
      if ( NULL == planRuntime ||
           NULL == planRuntime->getPlan() )
      {
         PD_LOG( PDERROR, "plan should not be NULL" ) ;
         rc = SDB_SYS ;
         goto error ;
      }

      _builder.append( FIELD_NAME_NAME,
                       _options._fullName ) ;
      _builder.append( FIELD_NAME_SCANTYPE,
                       IXSCAN == planRuntime->getScanType() ?
                       VALUE_NAME_IXSCAN : VALUE_NAME_TBSCAN ) ;
      _builder.append( FIELD_NAME_INDEXNAME,
                       planRuntime->getIndexName() ) ;
      _builder.appendBool( FIELD_NAME_USE_EXT_SORT,
                           planRuntime->sortRequired() ) ;
      _builder.append( FIELD_NAME_QUERY,
                       planRuntime->getMatchTree()->getParsedQuery(
                                         planRuntime->getParameters() ) ) ;
      if ( IXSCAN == planRuntime->getScanType() &&
           NULL != planRuntime->getPredList() )
      {
         _builder.append( FIELD_NAME_IX_BOUND,
                          planRuntime->getPredList()->getBound() ) ;
      }
      else
      {
         _builder.appendNull( FIELD_NAME_IX_BOUND ) ;
      }

      _builder.appendBool( FIELD_NAME_NEED_MATCH,
                           !planRuntime->getMatchTree()->isMatchesAll() ) ;

      hostName = pmdGetKRCB()->getHostName() ;
      ss << hostName << ":" << pmdGetOptionCB()->getServiceAddr() ;
      _builder.append( FIELD_NAME_NODE_NAME, ss.str() ) ;
      _builder.append( FIELD_NAME_GROUPNAME, pmdGetKRCB()->getGroupName() ) ;

      if ( _needDetail )
      {
         BSONObjBuilder subBuilder( _builder.subobjStart( FIELD_NAME_DETAIL ) ) ;

         if ( planRuntime->getPlan()->isCached() )
         {
            subBuilder.append( OPT_FIELD_CACHE_STATUS,
                               planRuntime->isNewPlan() ?
                               OPT_CACHE_STATUS_NEWCACHE :
                               OPT_CACHE_STATUS_HITCACHE ) ;
         }
         else
         {
            subBuilder.append( OPT_FIELD_CACHE_STATUS,
                               OPT_CACHE_STATUS_NOCACHE ) ;
         }

         planRuntime->getPlan()->toBSON( subBuilder, TRUE, FALSE ) ;
         if ( !planRuntime->getParameters().isEmpty() )
         {
            planRuntime->getParameters().toBSON( subBuilder ) ;
         }
         subBuilder.done() ;
      }

      /// get some info before explain
      _beginMon = *cb->getMonAppCB() ;
      ossGetCurrentTime( _beginTime ) ;

      /// get begin cpu time info
      ossGetCPUUsage( cb->getThreadHandle(), userTime, sysTime ) ;
      _beginUsrCpu = userTime.seconds + (FLOAT64)userTime.microsec / 1000000 ;
      _beginSysCpu = sysTime.seconds + (FLOAT64)sysTime.microsec / 1000000 ;

      _queryContextID = queryContextID ;
      _cbOfQuery = cb ;
   done:
      PD_TRACE_EXITRC( SDB_RTNCONTEXTEXPLAIN__PREPARETOEXPLAIN, rc ) ;
      return rc ;
   error:
      if ( -1 != queryContextID )
      {
         sdbGetRTNCB()->contextDelete( queryContextID,
                                       cb ) ;
      }
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_RTNCONTEXTEXPLAIN__EXPLAINQUERY, "_rtnContextExplain::_explainQuery" )
   INT32 _rtnContextExplain::_explainQuery( _pmdEDUCB *cb )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB_RTNCONTEXTEXPLAIN__EXPLAINQUERY ) ;
      rtnContextBuf ctxBuf ;
      BSONObj record ;
      ossTime userTime, sysTime ;

      /// here we do not use $count coz it does not surpport
      /// 'limit' and 'skip'.
      while ( _needRun )
      {
         rc = rtnGetMore( _queryContextID, -1, ctxBuf, cb, sdbGetRTNCB() ) ;
         if ( SDB_DMS_EOC == rc )
         {
            rc = SDB_OK ;
            _queryContextID = -1 ;  /// context has been freed in getmore.
            break ;
         }
         else if ( SDB_OK != rc )
         {
            PD_LOG( PDERROR, "failed to get more from context[%lld]"
                    ", rc:%d", _queryContextID, rc ) ;
            _queryContextID = -1 ;
            goto error ;
         }
         else
         {
            while ( TRUE )
            {
               rc = ctxBuf.nextObj( record ) ;
               if ( SDB_DMS_EOC == rc )
               {
                  rc = SDB_OK ;
                  break ;
               }
               else if ( SDB_OK != rc )
               {
                  PD_LOG( PDERROR, "Failed to get more from buf of "
                          "context[%lld],rc:%d ", _queryContextID, rc ) ;
                  goto error ;
               }
               else
               {
                  ++_recordNum ;
               }
            }
         }
      }

      ossGetCurrentTime( _endTime ) ;
      _endMon = *cb->getMonAppCB() ;
      /// get end cpu time info
      ossGetCPUUsage( cb->getThreadHandle(), userTime, sysTime ) ;
      _endUsrCpu = userTime.seconds + (FLOAT64)userTime.microsec / 1000000 ;
      _endSysCpu = sysTime.seconds + (FLOAT64)sysTime.microsec / 1000000 ;

   done:
      if ( -1 != _queryContextID )
      {
         sdbGetRTNCB()->contextDelete( _queryContextID,
                                       _cbOfQuery ) ;
         _queryContextID = -1 ;
      }
      PD_TRACE_EXITRC( SDB_RTNCONTEXTEXPLAIN__EXPLAINQUERY, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_RTNCONTEXTEXPLAIN__COMMITRESULT, "_rtnContextExplain::_commitResult" )
   INT32 _rtnContextExplain::_commitResult( _pmdEDUCB *cb )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB_RTNCONTEXTEXPLAIN__COMMITRESULT ) ;

      _builder.appendNumber( FIELD_NAME_RETURN_NUM, _recordNum ) ;
      UINT64 beginTime = _beginTime.time * 1000000 + _beginTime.microtm  ;
      UINT64 endTime = _endTime.time * 1000000 + _endTime.microtm  ;
      _builder.append( FIELD_NAME_ELAPSED_TIME,
                       FLOAT64( ( endTime - beginTime ) / 1000000.0 ) ) ;
      _builder.appendNumber( FIELD_NAME_INDEXREAD,
                             (INT64)( _endMon.totalIndexRead -
                             _beginMon.totalIndexRead ) ) ;
      _builder.appendNumber( FIELD_NAME_DATAREAD,
                             (INT64)( _endMon.totalDataRead -
                             _beginMon.totalDataRead ) ) ;

      _builder.append( FIELD_NAME_USERCPU,
                       _endUsrCpu - _beginUsrCpu ) ;

      _builder.append( FIELD_NAME_SYSCPU,
                       _endSysCpu - _beginSysCpu ) ;

      rc = append( _builder.obj() ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed append obj to context[%lld]:%d",
                 contextID(), rc ) ;
         goto error ;
      }

   done:
      PD_TRACE_EXITRC( SDB_RTNCONTEXTEXPLAIN__COMMITRESULT, rc ) ;
      return rc ;
   error:
      goto done ;
   }
}

