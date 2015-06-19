/*******************************************************************************

   Copyright (C) 2011-2015 SequoiaDB Ltd.

   This program is free software: you can redistribute it and/or modify
   it under the term of the GNU Affero General Public License, version 3,
   as published by the Free Software Foundation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warrenty of
   MARCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU Affero General Public License for more details.

   You should have received a copy of the GNU Affero General Public License
   along with this program. If not, see <http://www.gnu.org/license/>.

   Source File Name = dmsIndexBuilderImpl.cpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          8/6/2015  David Li  Initial Draft

   Last Changed =

*******************************************************************************/
#include "dmsIndexBuilderImpl.hpp"
#include "dmsStorageIndex.hpp"
#include "dmsStorageData.hpp"
#include "dmsScanner.hpp"
#include "ixmKey.hpp"
#include "ixm.hpp"
#include "pmd.hpp"
#include "dmsCB.hpp"

namespace engine
{
   _dmsIndexOnlineBuilder::_dmsIndexOnlineBuilder( _dmsStorageIndex* indexSU,
                                                   _dmsStorageData* dataSU,
                                                   _dmsMBContext* mbContext,
                                                   _pmdEDUCB* eduCB,
                                                   INT32 indexID,
                                                   dmsExtentID indexLID )
   : _dmsIndexBuilder( indexSU, dataSU, mbContext, eduCB, indexID, indexLID )
   {
   }

   _dmsIndexOnlineBuilder::~_dmsIndexOnlineBuilder()
   {
   }

   INT32 _dmsIndexOnlineBuilder::_build()
   {
      INT32 rc = SDB_OK ;
      Ordering ordering = Ordering::make( _indexCB->keyPattern() ) ;

      // loop through each extent
      while ( DMS_INVALID_EXTENT != _currentExtentID )
      {
         rc = _mbContext->mbLock( SHARED ) ;
         if ( rc )
         {
            PD_LOG( PDERROR, "dms mb context lock failed, rc: %d", rc ) ;
            goto error ;
         }

         rc = _beforeExtent() ;
         if ( SDB_OK != rc )
         {
            if ( _DMS_SKIP_EXTENT == rc )
            {
               _mbContext->mbUnlock() ;
               continue ;
            }

            goto error ;
         }

         dmsExtScanner extScanner( _suData, _mbContext, NULL, _currentExtentID ) ;
         dmsRecordID recordID ;
         ossValuePtr recordDataPtr ;

         while ( SDB_OK == ( rc = extScanner.advance( recordID, recordDataPtr, _eduCB ) ) )
         {
            rc = _insertKey( recordDataPtr, recordID, ordering ) ;
            if ( SDB_OK != rc )
            {
               goto error ;
            }
         }

         if ( SDB_DMS_EOC != rc )
         {
            PD_LOG ( PDERROR, "Failed to get record: %d", rc ) ;
            goto error ;
         }
         else
         {
            rc = SDB_OK ;
         }

         _currentExtentID = _extent->_nextExtent ;

         rc = _afterExtent() ;
         if ( SDB_OK != rc )
         {
            goto error ;
         }

         _mbContext->mbUnlock() ;
      }

   done:
      _mbContext->mbUnlock() ;
      return rc ;
   error:
      goto done ;
   }

   _dmsIndexOfflineBuilder::_dmsIndexOfflineBuilder( _dmsStorageIndex* indexSU,
                                                     _dmsStorageData* dataSU,
                                                     _dmsMBContext* mbContext,
                                                     _pmdEDUCB* eduCB,
                                                     INT32 indexID,
                                                     dmsExtentID indexLID )
   : _dmsIndexBuilder( indexSU, dataSU, mbContext, eduCB, indexID, indexLID )
   {
      _sorter = NULL ;
      _bufSize = 0 ;
      _bufExtSize = 0 ;
      _eoc = FALSE ;
   }

   _dmsIndexOfflineBuilder::~_dmsIndexOfflineBuilder()
   {
      if ( NULL != _sorter )
      {
         sdbGetDMSCB()->releaseIxmKeySorter( _sorter ) ;
         _sorter = NULL ;
      }
   }

   INT32 _dmsIndexOfflineBuilder::_init()
   {
      INT32 rc = SDB_OK ;

      Ordering ordering = Ordering::make( _indexCB->keyPattern() ) ;
      _dmsIxmKeyComparer comparer( ordering ) ;

      _bufSize = pmdGetOptionCB()->getSortBufSize() * 1024 * 1024 ;
      // add extend size for fetching records by extent granularity
      // and to prevent sorter's buffer overflowing.
      // so we assign max extent size to ensure the sorter can't 
      // overflow when fetching records from a extent.
      _bufExtSize = DMS_MAX_EXTENT_SZ ;

      _sorter = sdbGetDMSCB()->createIxmKeySorter( _bufSize + _bufExtSize, comparer ) ;
      if ( NULL == _sorter )
      {
         PD_LOG( PDERROR, "failed to create ixmKey sorter" ) ;
         rc = SDB_SYS ;
         goto error ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _dmsIndexOfflineBuilder::_fillSorter()
   {
      INT32 rc = SDB_OK ;

      for(;;)
      {
         if ( DMS_INVALID_EXTENT == _currentExtentID )
         {
            if ( _eoc )
            {
               rc = SDB_DMS_EOC ;
               goto error ;
            }
            else
            {
               _eoc = TRUE ;
               rc = SDB_OK ;
               goto done ;
            }
         }

         if ( _sorter->usedBufferSize() > _bufSize )
         {
            // the sorter's buffer is logical overflow (actually not overflow in sorter),
            // so stop fetching records from next extent
            PD_LOG( PDDEBUG, "sorter is full, bufSize=%d, usedBufSize=%d, total=%d",
                    _bufSize, _sorter->usedBufferSize(), _sorter->bufferSize() ) ;
            goto done ;
         }

         rc = _mbContext->mbLock( SHARED ) ;
         if ( SDB_OK != rc )
         {
            PD_LOG( PDERROR, "dms mb context lock failed, rc: %d", rc ) ;
            goto error ;
         }

         rc = _beforeExtent() ;
         if ( SDB_OK != rc )
         {
            if ( _DMS_SKIP_EXTENT == rc )
            {
               _mbContext->mbUnlock() ;
               continue ;
            }

            goto error ;
         }

         dmsExtScanner extScanner( _suData, _mbContext, NULL, _currentExtentID ) ;
         dmsRecordID recordID ;
         ossValuePtr recordDataPtr ;

         while ( SDB_OK == ( rc = extScanner.advance( recordID, recordDataPtr, _eduCB ) ) )
         {
            BSONObjSet keySet ;
            BSONObjSet::iterator it ;

            rc = _getKeySet( recordDataPtr, keySet ) ;
            if ( SDB_OK != rc )
            {
               goto error ;
            }

            for ( it = keySet.begin() ; it != keySet.end() ; it++ )
            {
               ixmKeyOwned key( *(it) ) ;
               rc = _sorter->push( key, recordID ) ;
               if ( SDB_OK != rc )
               {
                  SDB_ASSERT( SDB_DMS_EOC != rc, "sorter can't overflow" ) ;
                  goto error ;
               }
            }
         }

         if ( SDB_DMS_EOC == rc )
         {
            rc = SDB_OK ;
         }
         else
         {
            goto error ;
         }

         _currentExtentID = _extent->_nextExtent ;

         rc = _afterExtent() ;
         if ( SDB_OK != rc )
         {
            goto error ;
         }

         _mbContext->mbUnlock() ;
      }

   done:
      return rc ;
   error:
      _mbContext->mbUnlock() ;
      goto done ;
   }


   INT32 _dmsIndexOfflineBuilder::_insertKeys( const Ordering& ordering )
   {
      #define _KEYS_PER_BATCH 10000
      INT32 rc = SDB_OK ;

      for (;;)
      {
         ixmKey key ;
         dmsRecordID recordID ;

         if ( _eduCB->isInterrupted() )
         {
            rc = SDB_APP_INTERRUPT ;
            goto error ;
         }

         rc = _checkIndexAfterLock( SHARED ) ;
         if ( SDB_OK != rc )
         {
            goto error ;
         }

         for ( INT32 i = 0 ; i < _KEYS_PER_BATCH ; i++ )
         {
            rc = _sorter->fetch( key, recordID ) ;
            if ( SDB_OK == rc )
            {
               rc = _insertKey( key, recordID, ordering ) ;
               if ( SDB_OK != rc )
               {
                  goto error ;
               }
            }
            else if ( SDB_DMS_EOC != rc )
            {
               goto error ;
            }
            else
            {
               rc = SDB_OK ;
               goto done ;
            }
         }

         _mbContext->mbUnlock() ;
      }

   done:
      _mbContext->mbUnlock() ;
      return rc ;
   error:
      goto done ;
   }

   INT32 _dmsIndexOfflineBuilder::_build()
   {
      INT32 rc = SDB_OK ;

      Ordering ordering = Ordering::make( _indexCB->keyPattern() ) ;

      rc = _init() ;
      if ( SDB_OK != rc )
      {
         goto before_build_error ;
      }

      rc = _beforeBuild() ;
      if ( SDB_OK != rc )
      {
         goto before_build_error ;
      }

      for(;;)
      {
         if ( _eoc )
         {
            goto build_done ;
         }

         rc = _sorter->reset() ;
         if ( SDB_OK != rc )
         {
            goto build_error ;
         }

         rc = _fillSorter() ;
         if ( SDB_OK != rc )
         {
            goto build_error ;
         }

         rc = _sorter->sort() ;
         if ( SDB_OK != rc )
         {
            goto build_error ;
         }

         rc = _insertKeys( ordering ) ;
         if ( SDB_OK != rc )
         {
            goto build_error ;
         }
      }

   done:
      _mbContext->mbUnlock() ;
      return rc ;
   before_build_error:
      goto done ;
   build_done:
      {
         INT32 ret = _afterBuild() ;
         if ( SDB_OK == rc )
         {
            rc = ret ;
         }
      }
      goto done ;
   build_error:
      goto build_done ;
   }

   INT32 _dmsIndexOfflineBuilder::_beforeBuild()
   {
      INT32 rc = SDB_OK ;

      if ( SDB_OK == ( rc = _checkIndexAfterLock( EXCLUSIVE ) ) )
      {
         if ( DMS_IS_MB_FLAG_CREATE_INDEX_OFFLINE( _mbContext->mb()->_flag ) )
         {
            // can not create index offline in the same collection at the same time
            rc = SDB_DMS_INCOMPATIBLE_MODE ;
         }
         else
         {
            DMS_SET_MB_FLAG_CREATE_INDEX_OFFLINE( _mbContext->mb()->_flag ) ;
         }
         _mbContext->mbUnlock() ;
      }

      return rc ;
   }

   INT32 _dmsIndexOfflineBuilder::_afterBuild()
   {
      INT32 rc = SDB_OK ;

      if ( SDB_OK == ( rc = _mbContext->mbLock( EXCLUSIVE ) ) )
      {
         // should unset mb flag no matter if index is valid or not
         if ( DMS_IS_MB_FLAG_CREATE_INDEX_OFFLINE( _mbContext->mb()->_flag ) )
         {
            DMS_UNSET_MB_FLAG_CREATE_INDEX_OFFLINE( _mbContext->mb()->_flag ) ;
         }
         _mbContext->mbUnlock() ;
      }

      return rc ;
   }
}

