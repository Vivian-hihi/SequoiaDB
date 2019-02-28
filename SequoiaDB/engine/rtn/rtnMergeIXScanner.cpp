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

   Source File Name = rtnMergeIXScanner.cpp

   Descriptive Name = Runtime Index Merge Scanner

   When/how to use: this program may be used on binary and text-formatted
   versions of Runtime component. This file contains code for index traversal,
   including advance, pause, resume operations.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          11/14/2012  CYX Initial Draft

   Last Changed =

*******************************************************************************/
#include "rtnIXScanner.hpp"
#include "rtnMergeIXScanner.hpp"
#include "rtnIXScannerFactory.hpp"
#include "pdTrace.hpp"
#include "rtnTrace.hpp"
#include "dpsTransCB.hpp"

namespace engine
{

   _rtnMergeIXScanner::_rtnMergeIXScanner( ixmIndexCB *indexCB,
                                rtnPredicateList *predList,
                                _dmsStorageUnit *su, _pmdEDUCB *cb,
                                scannerSharedInfo * sharedInfo )
   : _order(Ordering::make(indexCB->keyPattern()))
   {
      _direction = predList->getDirection();
      _firstRun = TRUE;
      _wasFromLeft = FALSE;
      _isValid = TRUE;
      //_transLockwait = TRUE;  // default wait for lock
      _type = SCANNER_TYPE_MERGE;
      if ( sharedInfo == NULL )
      {
         _sharedInfo.init();
      }
      else 
      {
         _sharedInfo.init(sharedInfo);
      }
      _savedRID.reset();
      PD_LOG ( PDDEBUG, "IX Merge Scanner created." );
   }

   void _rtnMergeIXScanner::setMergeScanner( IXScannerType ltype,
                            IXScannerType rtype,
                            ixmIndexCB *indexCB,
                            rtnPredicateList *predList,
                            dmsStorageUnit *su, pmdEDUCB *cb )
   {
      scannerFactory f;
      SDB_ASSERT ( indexCB && predList && su,
                   "indexCB, predList and su can't be NULL" ) ;
      _leftIXScanner = f.getScanner( ltype, indexCB, predList, su, 
                                     cb, &_sharedInfo );   
      _rightIXScanner = f.getScanner( rtype, indexCB, predList, su, 
                                      cb, &_sharedInfo );
      SDB_ASSERT ( _leftIXScanner && _rightIXScanner, 
                   "Merge IX scanner creation failed" ) ;
   }

   // destructor, do all rtnDiskIXScanner clean up plus free _memIXScanner
   _rtnMergeIXScanner::~_rtnMergeIXScanner()
   {
#ifdef _DEBUG
      PD_LOG ( PDDEBUG, "Freeing IX Merge Scanner." );
#endif
      if ( _leftIXScanner ) 
      {
         SDB_OSS_DEL _leftIXScanner;
      }
      if ( _rightIXScanner )
      {
         SDB_OSS_DEL _rightIXScanner;
      }
   }

   // Description:
   //    This is a merge advance specifically for MemIXTree and Disk scan
   // Dependency:
   //
   // PD_TRACE_DECLARE_FUNCTION ( SDB__RTNMERGEIXSCAN_ADVANCE, "_rtnMergeIXScanner::advance" )
   INT32 _rtnMergeIXScanner::advance( dmsRecordID &rid )
   {
      PD_TRACE_ENTRY ( SDB__RTNMERGEIXSCAN_ADVANCE ) ;
      INT32     rc        = SDB_OK;  // return rc
      INT32     rcl       = SDB_OK;  // rc from left scanner
      INT32     rcr       = SDB_OK;  // rc from right scanner
      BOOLEAN   leftDone  ;
      BOOLEAN   rightDone ;
      BOOLEAN   leftInit  ;
      BOOLEAN   rightInit ;
      BOOLEAN   leftValid  ;
      BOOLEAN   rightValid ;

   begin:
      leftDone  = _leftIXScanner->eof();  
      rightDone = _rightIXScanner->eof();  
      leftInit  = _leftIXScanner->initialized();
      rightInit = _rightIXScanner->initialized();
      leftValid = _leftIXScanner->isValid() ;
      rightValid = _rightIXScanner->isValid() ;

      SDB_ASSERT( ( leftInit || rightInit ), 
                  "At least one scanner must be initilized"); 

      if ( ( leftDone || !leftInit ) && ( rightDone || !rightInit ) )
      {
         rc = SDB_IXM_EOC;
         PD_LOG ( PDDEBUG, "IX Merge Scanner has reach the end." );
              
         goto done;
      }
      // quick return if one of the scanner is finished or is not
      // initialized at all
      if ( leftDone || !leftInit )
      {
         // only advance if last time was from right or right was not valid
         if( !_wasFromLeft || !rightValid )
         {
            rc = _rightIXScanner->advance( _rrid ) ;
         }
         // else 
         
         rid =  _rrid;
         _wasFromLeft = FALSE;
         goto done ;
      }

      if ( rightDone || !rightInit )
      {
         // only advance if last time was from left or left was not valid
         if( _wasFromLeft || !leftValid )
         {
            rc = _leftIXScanner->advance( _lrid ) ;
         }
         rid = _lrid;
         _wasFromLeft = TRUE;
         goto done ;
      }

      if( _firstRun )
      {
         // first time advance both
         rcl = _leftIXScanner->advance( _lrid );
         rcr = _rightIXScanner->advance( _rrid ) ;
         _firstRun = FALSE;
      }
      // otherwise 
      else
      {
         if ( _wasFromLeft || !leftValid )
         {
            // last index used was from left scanner, or left became invalid
            // after resume, we need to advance left 
            rcl = _leftIXScanner->advance( _lrid ) ;
         }
         if ( !_wasFromLeft || !rightValid )
         {
            // last index used was from right scanner, or right became invalid
            // after resume, we need to advance right
            rcr = _rightIXScanner->advance( _rrid ) ;
         }
      }

      // handle the case advance() return EOC
      if( SDB_IXM_EOC == rcl )
      {
         rcl = SDB_OK;
         if ( rightDone )
         { 
            rc = SDB_IXM_EOC;
         }
         else
         {
            rc = rcr;
         }
         rid = _rrid;
         _wasFromLeft = FALSE;
         goto done;
      }
         
      if( SDB_IXM_EOC == rcr )
      {
         rcr = SDB_OK;
         if ( rightDone )
         { 
            rc = SDB_IXM_EOC;
         }
         else
         {
            rc = rcl;
         }
         rid = _lrid;
         _wasFromLeft = TRUE;
         goto done;
      }
      
      // in case of any other errors, goto error
      if( SDB_OK != rcl || SDB_OK != rcr )
      {
         rc = (rcl!= SDB_OK) ? rcl : rcr;
         goto error;
      }
      // if we reach here, both scanners did return valid rid, 
      // now compare index value from both scanners and return the one
      // come to the front.
      {
         ixmKeyOwned lkey( *(_leftIXScanner->getCurKeyObj()) );
         ixmKeyOwned rkey( *(_rightIXScanner->getCurKeyObj()) );
         if( lkey.woCompare(rkey, _order) * _direction > 0 )
         {
            _wasFromLeft = FALSE;
            rid = _rrid;
         }
         else
         {
            _wasFromLeft = TRUE;
            rid = _lrid;
         }
      }
   done :
      // before we decide to return the rid, try to put it in dup buffer
      if( SDB_OK == rc )
      {
         if ( _sharedInfo.isLocal() &&
              ( _sharedInfo.getDupBuf()->end() !=
                _sharedInfo.getDupBuf()->find ( rid ) ) )
         {
            // if we are able to find the recordid in dupBuffer, that
            // means we've already processed the record, so let's also
            // jump back to begin
            rid.reset() ;
            goto begin ;
         }
         _sharedInfo.getDupBuf()->insert ( rid ) ;
      }
#ifdef _DEBUG
      PD_LOG ( PDDEBUG, "IX Merge Scanner advance, rc=%d, "
               "_wasFromLeft=%d, rid=(%d, %d)", 
               rc, _wasFromLeft, rid._extent, rid._offset );

      PD_TRACE1 ( SDB__RTNMERGEIXSCAN_ADVANCE, 
                  PD_PACK_INT(_wasFromLeft) ) ;
#endif
      PD_TRACE_EXITRC ( SDB__RTNMERGEIXSCAN_ADVANCE, rc ) ;

      return rc ;

   error :
      PD_LOG ( PDERROR, "Error during merge advance. rcl=%d, rcr=%d",
               rcl, rcr );
      goto done ;

   }

   // restoring the bson key and rid for the current index rid. This is done by
   // comparing the current key/rid pointed by _curIndexRID still same as
   // previous saved one. If so it means there's no change in this key, and we
   // can move on the from the current index rid. Otherwise we have to locate
   // the new position for the saved key+rid
   // this is used in query scan only
   // PD_TRACE_DECLARE_FUNCTION ( SDB__RTNMERGEIXSCAN_RESUMESCAN, "_rtnMergeIXScanner::resumeScan" )
   INT32 _rtnMergeIXScanner::resumeScan( )
   {
      SINT32 rc = SDB_OK;
      SINT32 rcl = SDB_OK;
      SINT32 rcr = SDB_OK ;
      PD_TRACE_ENTRY ( SDB__RTNMERGEIXSCAN_RESUMESCAN ) ;

      rcl = _leftIXScanner->resumeScan( );
      if( (SDB_OK != rcl) && (SDB_IXM_EOC != rcl) )
      {
         rc = rcl;
         goto error;
      }
      
      rcr = _rightIXScanner->resumeScan( );
      if( SDB_OK != rcr && (SDB_IXM_EOC != rcr) )
      {
         rc = rcr;
         goto error;
      }

      // Only EOC if both scanner hit EOC
      if( (SDB_IXM_EOC == rcl) && (SDB_IXM_EOC == rcr) )
      {
         rc = SDB_IXM_EOC;
         goto done;
      }

      // trigger relocateRID if anything changed from either side
      if ( !_leftIXScanner->isValid() || !_rightIXScanner->isValid() )
      {
         // tree latch is held only after first keylocate done
         if ( _leftIXScanner->initialized() &&
              ((_rtnMemIXTreeScanner*) _leftIXScanner )
                 ->firstKeylocateDone() )
         {
            rcl = _leftIXScanner->relocateRID( _savedObj,
                                               _savedRID,
                                               TRUE );
            if( SDB_OK != rcl && (SDB_IXM_EOC != rcl) )
            {
               rc = rcl;
               goto error;
            }
         }

         if ( _rightIXScanner->initialized() )
         {
            rcr = _rightIXScanner->relocateRID( _savedObj,
                                                _savedRID,
                                                TRUE );
            if( SDB_OK != rcr && (SDB_IXM_EOC != rcr) )
            {
               rc = rcr;
               goto error;
            }
         }
      }
   done :
      PD_TRACE_EXITRC ( SDB__RTNMERGEIXSCAN_RESUMESCAN, rc ) ;
      return rc ;
   error :
      PD_LOG ( PDERROR, "Error during merge resume. rc=%d, rcr=%d, rcl=%d",
               rc, rcr, rcl );
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__RTNMERGEIXSCAN_PAUSESCAN, "_rtnMergeIXScanner::pauseScan" )
   INT32 _rtnMergeIXScanner::pauseScan(  )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB__RTNMERGEIXSCAN_PAUSESCAN ) ;
      rc = _leftIXScanner->pauseScan();
      if( SDB_OK != rc )
      {
         goto error;
      }

      rc = _rightIXScanner->pauseScan();
      if( SDB_OK != rc )
      {
         goto error;
      }

      // globally save the obj and rid incase things changed after resume, 
      // at which time we need to do relocateRID based on these saved value
      _savedRID = getSavedRIDFromChild() ;
      _savedObj = getSavedObjFromChild()->copy() ;
      
#ifdef _DEBUG
      PD_LOG ( PDDEBUG, "IX Merge Scanner pauseScan, "
               "_wasFromLeft=%d, _savedObj=%s, _savedRID=(%d, %d)", 
                _wasFromLeft, _savedObj.toString().c_str(), 
                _savedRID._extent, _savedRID._offset );
#endif
   done :
      PD_TRACE_EXITRC ( SDB__RTNMERGEIXSCAN_PAUSESCAN, rc ) ;
      return rc ;
   error :
      PD_LOG ( PDERROR, "Error during merge pause. rc=%d", rc );
      goto done ;
   }
}

