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

   Source File Name = dmsCompress.hpp

   Descriptive Name =

   When/how to use: str util

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          20/06/2014  XJH Initial Draft

   Last Changed =

******************************************************************************/

#ifndef DMSCOMPRESS_HPP__
#define DMSCOMPRESS_HPP__

#include "core.hpp"
#include "ossRWMutex.hpp"
#include "utilCompressor.hpp"
#include "../bson/bson.h"

using namespace bson ;

namespace engine
{
   class _pmdEDUCB ;

   class _dmsCompressorEntry
   {
      friend class _dmsCompressorGuard ;
   public:
      _dmsCompressorEntry() ;

      void setCompressor( utilCompressor *compressor ) ;
      void setDictionary( const CHAR *dictionary ) ;

      OSS_INLINE utilCompressor* getCompressor() { return _compressor ; }
      OSS_INLINE const CHAR* getDictionary() { return _dictionaryAddr ; }
      OSS_INLINE BOOLEAN ready() { return ( NULL != _compressor ) ; }

      void reset() ;

   private:
      utilCompressor *_compressor ;
      const CHAR *_dictionaryAddr ;
      ossRWMutex _lock ;
   } ;
   typedef _dmsCompressorEntry dmsCompressorEntry ;

   class _dmsCompressorGuard
   {
   public:
      _dmsCompressorGuard( _dmsCompressorEntry *compEntry, OSS_LATCH_MODE mode )
         : _lock( &compEntry->_lock),
           _mode( mode )
      {
         if ( SHARED == _mode )
         {
            _lock->lock_r() ;
         }
         else if ( EXCLUSIVE == _mode )
         {
            _lock->lock_w() ;
         }
      }

      ~_dmsCompressorGuard()
      {
         release() ;
      }

      void release()
      {
         if ( SHARED == _mode )
         {
            _lock->release_r() ;
         }
         else if ( EXCLUSIVE == _mode )
         {
            _lock->release_w() ;
         }
         _mode = -1 ;
      }

   private:
      ossRWMutex *_lock ;
      INT32 _mode ;
   } ;
   typedef _dmsCompressorGuard dmsCompressorGuard ;

   /*
      ppData: output data pointer, not need release
   */
   INT32 dmsCompress ( _pmdEDUCB *cb, _dmsCompressorEntry *compressorEntry,
                       const CHAR *pInputData, INT32 inputSize,
                       const CHAR **ppData, INT32 *pDataSize ) ;

   INT32 dmsCompress ( _pmdEDUCB *cb, _dmsCompressorEntry *compressorEntry,
                       const BSONObj &obj, const CHAR* pOIDPtr, INT32 oidLen,
                       const CHAR **ppData, INT32 *pDataSize ) ;

   /*
      ppData: output data pointer, not need release
   */
   INT32 dmsUncompress ( _pmdEDUCB *cb, _dmsCompressorEntry *compressorEntry,
                         const CHAR *pInputData, INT32 inputSize,
                         const CHAR **ppData, INT32 *pDataSize ) ;
}

#endif // DMSCOMPRESS_HPP__

