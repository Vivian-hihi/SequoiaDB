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
      PD_LOG ( PDDEBUG, "Freeing IX Merge Scanner." );
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
      BOOLEAN   leftDone  = _leftIXScanner->eof();  
      BOOLEAN   rightDone = _rightIXScanner->eof();  
      BOOLEAN   leftInit  = _leftIXScanner->initialized();
      BOOLEAN   rightInit = _rightIXScanner->initialized();

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
         _wasFromLeft = FALSE;
         return _rightIXScanner->advance( rid ) ;
      }

      if ( rightDone || !rightInit )
      {
         _wasFromLeft = TRUE;
         return _leftIXScanner->advance( rid ) ;
      }

      if( _firstRun )
      {
         // first time advance both
         rcl = _leftIXScanner->advance( _lrid );
         rcr = _rightIXScanner->advance( _rrid ) ;
         _firstRun = FALSE;
      }
      // otherwise 
      else if ( TRUE == _wasFromLeft )
      {
         // last index used was from left scanner, advance left 
         rcl = _leftIXScanner->advance( _lrid ) ;
      }
      else
      {
         rcr = _rightIXScanner->advance( _rrid ) ;
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
      PD_TRACE1 ( SDB__RTNMERGEIXSCAN_ADVANCE, 
                  PD_PACK_INT(_wasFromLeft) ) ;
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
   done :
      PD_TRACE_EXITRC ( SDB__RTNMERGEIXSCAN_RESUMESCAN, rc ) ;
      return rc ;
   error :
      PD_LOG ( PDERROR, "Error during merge resume. rc=%d", rc );
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

   done :
      PD_TRACE_EXITRC ( SDB__RTNMERGEIXSCAN_PAUSESCAN, rc ) ;
      return rc ;
   error :
      PD_LOG ( PDERROR, "Error during merge pause. rc=%d", rc );
      goto done ;
   }
}

