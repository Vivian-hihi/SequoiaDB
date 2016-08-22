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

   Source File Name = dmsStorageLobData.hpp

   Descriptive Name =

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          17/07/2014  YW Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef DMS_STORAGELOBDATA_HPP_
#define DMS_STORAGELOBDATA_HPP_

#include "dmsLobDef.hpp"
#include "ossIO.hpp"
#include "dmsStorageBase.hpp"
#include "pmdEDU.hpp"
#include "utilCache.hpp"

namespace engine
{
   /*
      _dmsStorageLobData define
   */
   class _dmsStorageLobData : public utilCachFileBase
   {
   public:
      _dmsStorageLobData( const CHAR *fileName,
                          BOOLEAN enableSparse,
                          BOOLEAN useDirectIO ) ;
      virtual ~_dmsStorageLobData() ;

   /// Base class functions
   public:
      virtual const CHAR*     getFileName() const ;

      virtual INT32  write( INT32 pageID, const CHAR *pData,
                            UINT32 len, UINT32 offset,
                            IExecutor *cb ) ;

      virtual INT32  read( INT32 pageID, CHAR *pData,
                           UINT32 len, UINT32 offset,
                           UINT32 &readLen,
                           IExecutor *cb ) ;

   public:
      OSS_INLINE INT64 getFileSz() const
      {
         return _fileSz ;
      }

      OSS_INLINE INT64 getDataSz() const
      {
         return getFileSz() - sizeof( _dmsStorageUnitHeader ) ;
      }

      OSS_INLINE UINT32 pageSize() const { return _pageSz ; }
      OSS_INLINE UINT32 pageSizeSquareRoot() const { return _logarithmic ; }

      OSS_INLINE UINT32 getSegmentSize() const { return DMS_SEGMENT_SZ ; }
      OSS_INLINE UINT32 segmentPages() const { return _segmentPages ; }
      OSS_INLINE UINT32 segmentPagesSquareRoot() const { return _segmentPagesSquare ; }

      INT32 open( const CHAR *path,
                  BOOLEAN createNew,
                  UINT32 totalDataPages,
                  const dmsStorageInfo &info,
                  _pmdEDUCB *cb ) ;

      BOOLEAN isOpened() const ;

      INT32 close() ;

      INT32 remove() ;

      INT32 extend( INT64 len ) ;

      INT32 readRaw( _pmdEDUCB *cb,
                     UINT64 offset,
                     UINT32 len,
                     CHAR *buf,
                     UINT32 &readLen ) ;

      INT32 flush() ;

   private:
      INT32 _initFileHeader( const dmsStorageInfo &info,
                             _pmdEDUCB *cb ) ;

      INT32 _validateFile( const dmsStorageInfo &info,
                           _pmdEDUCB *cb ) ;

      INT32 _getFileHeader( _dmsStorageUnitHeader &header,
                            _pmdEDUCB *cb ) ;

      OSS_INLINE SINT64 getSeek( DMS_LOB_PAGEID page,
                                 UINT32 offset ) const
      {
         INT64 seek = page ;
         return ( seek << _logarithmic ) +
                sizeof( _dmsStorageUnitHeader ) + offset ;
      }

      INT32 _extend( INT64 len ) ;

      INT32 _postOpen( INT32 cause ) ;

   private:
      std::string       _fileName ;
      CHAR              _fullPath[ OSS_MAX_PATHSIZE + 1 ] ;
      OSSFILE           _file ;
      INT64             _fileSz ;
      UINT32            _pageSz ;
      UINT32            _logarithmic ;
      UINT32            _flags ; 
      UINT32            _segmentPages ;
      UINT32            _segmentPagesSquare ;

   } ;
   typedef class _dmsStorageLobData dmsStorageLobData ;
}

#endif // DMS_STORAGELOBDATA_HPP_

