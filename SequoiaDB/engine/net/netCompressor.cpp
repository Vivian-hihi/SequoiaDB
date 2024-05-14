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

   Source File Name = netCompressor.cpp

   Descriptive Name =

   When/how to use: this program may be used on binary and text-motionatted
   versions of PD component. This file contains declare of PD functions.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who         Description
   ====== =========== =========== ==============================================
          01/02/2024  fangjiabin  Initial Draft

   Last Changed =

*******************************************************************************/

#include "netCompressor.hpp"
#include "utilCompressorLZ4.hpp"

namespace engine
{
   #define NET_COMPRESS_MIN_SIZE   64 * 1024   // 64K

   _netMsgCompressor::_netMsgCompressor()
   {
      _pCompressBuff = NULL ;
      _compressBuffLen = 0 ;

      _pUncompressBuff = NULL ;
      _uncompressBuffLen = 0 ;

      _pTmpCompressBuff = NULL ;
      _tmpCompressBuffLen = 0 ;

      _compressor = UTIL_COMPRESSOR_INVALID ;

      _needReleaseUncompressBuff = TRUE ;
   }

   _netMsgCompressor::~_netMsgCompressor()
   {
      if ( _pCompressBuff )
      {
         SDB_THREAD_FREE( _pCompressBuff ) ;
         _pCompressBuff = NULL ;
         _compressBuffLen = 0 ;
      }

      if ( _needReleaseUncompressBuff && _pUncompressBuff )
      {
         SDB_THREAD_FREE( _pUncompressBuff ) ;
         _pUncompressBuff = NULL ;
         _uncompressBuffLen = 0 ;
      }

      if ( _pTmpCompressBuff )
      {
         SDB_THREAD_FREE( _pTmpCompressBuff ) ;
         _pTmpCompressBuff = NULL ;
         _tmpCompressBuffLen = 0 ;
      }
   }

   INT32 _netMsgCompressor::compressNetMsg( const MsgHeader *message, const CHAR* body, UINT32 bodyLen,
                                            CHAR** headerDes, CHAR** bodyDes,
                                            UINT32 &newHeaderLen, UINT32 &newBodyLen )
   {
      // body may be null
      SDB_ASSERT ( message && headerDes && bodyDes, "Invalid message" ) ;
      INT32 rc = SDB_OK ;
      utilCompressor * compressor = NULL ;
      UINT32 uncompressedLen = message->messageLength - sizeof(MsgHeader) ;
      UINT32 compressedLen = 0 ;
      MsgHeader headerCpy  ;
      UINT32 compressHeaderLen = uncompressedLen - bodyLen ;
      CHAR* pBuff = NULL ;
      CHAR* pTmpBuff = NULL ;

      *headerDes = (CHAR*)message ;
      *bodyDes = (CHAR*)body ;
      newHeaderLen = message->messageLength - bodyLen ;
      newBodyLen = bodyLen ;

      if ( !_needCompressMsg( message ) )
      {
         goto done ;
      }

      headerCpy = *message ;

      compressor = getCompressorByType( _compressor ) ;
      if ( !compressor )
      {
         PD_LOG( PDWARNING, "Failed to get compressor, rc: %d", SDB_SYS ) ;
         goto done ;
      }

      rc = compressor->compressBound( uncompressedLen, compressedLen ) ;
      PD_RC_CHECK( rc, PDERROR, "Failed to get max compressed length, rc: %d", rc ) ;

      pBuff = _getBuff( compressedLen + sizeof(MsgHeader), &_pCompressBuff, _compressBuffLen ) ;
      if ( !pBuff )
      {
         PD_LOG( PDERROR, "Failed to alloc compress buff, size: %d",
                 compressedLen + sizeof(MsgHeader) ) ;
         rc = SDB_OOM ;
         goto error ;
      }

      if ( compressHeaderLen > 0 )
      {
         pTmpBuff = _getBuff( message->messageLength, &_pTmpCompressBuff, _tmpCompressBuffLen ) ;
         if ( !pTmpBuff )
         {
            PD_LOG( PDERROR, "Failed to alloc compress buff, size: %d",
                    message->messageLength ) ;
            rc = SDB_OOM ;
            goto error ;
         }

         ossMemcpy( pTmpBuff, (CHAR*)message + sizeof(MsgHeader), compressHeaderLen ) ;
         if ( bodyLen > 0 && body )
         {
            ossMemcpy( pTmpBuff + compressHeaderLen, body, bodyLen ) ;
         }

         rc = compressor->compress( pTmpBuff, compressHeaderLen + bodyLen,
                                    (CHAR*)pBuff + sizeof(MsgHeader), compressedLen ) ;
         if ( rc )
         {
            if ( SDB_UTIL_COMPRESS_ABORT == rc )
            {
               rc = SDB_OK ;
               goto done ;
            }
            else
            {
               goto error ;
            }
         }
      }
      else if ( compressHeaderLen == 0 )
      {
         SDB_ASSERT( bodyLen == uncompressedLen, "Invalid msg body len" ) ;
         rc = compressor->compress( body, uncompressedLen,
                                    (CHAR*)pBuff + sizeof(MsgHeader), compressedLen ) ;
         if ( rc )
         {
            if ( SDB_UTIL_COMPRESS_ABORT == rc )
            {
               rc = SDB_OK ;
               goto done ;
            }
            else
            {
               goto error ;
            }
         }
      }
      else
      {
         SDB_ASSERT( FALSE, "Invalid msg length" ) ;
      }

      OSS_BIT_SET( headerCpy.flags, FLAG_COMPRESSED ) ;
      headerCpy.messageLength = sizeof(MsgHeader) + compressedLen ;
      ossMemcpy( pBuff, (CHAR*)(&headerCpy), sizeof(MsgHeader) ) ;

      if ( !body && 0 == bodyLen )
      {
         *headerDes = &pBuff[0] ;
         newHeaderLen = headerCpy.messageLength ;
      }
      else
      {
         *headerDes = &pBuff[0] ;
         newHeaderLen = sizeof(MsgHeader) ;
         *bodyDes = &pBuff[sizeof(MsgHeader)] ;
         newBodyLen = compressedLen ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _netMsgCompressor::compressNetMsg( const MsgHeader *message, UINT32 messageLen,
                                            CHAR** des, UINT32 &messageLenDes )
   {
      SDB_ASSERT ( des && message, "Invalid message" ) ;
      INT32 rc = SDB_OK ;
      utilCompressor * compressor = NULL ;
      UINT32 uncompressedLen = message->messageLength - sizeof(MsgHeader) ;
      UINT32 compressedLen = 0 ;
      MsgHeader headerCpy ;
      CHAR* pBuff = NULL ;

      *des = (CHAR*)message ;
      messageLenDes = messageLen ;

      if ( !_needCompressMsg( message ) )
      {
         goto done ;
      }

      headerCpy = *message ;

      compressor = getCompressorByType( _compressor ) ;
      if ( !compressor )
      {
         PD_LOG( PDWARNING, "Failed to get compressor, rc: %d", SDB_SYS ) ;
         goto done ;
      }

      rc = compressor->compressBound( uncompressedLen, compressedLen ) ;
      PD_RC_CHECK( rc, PDERROR, "Failed to get max compressed length, rc: %d", rc ) ;

      pBuff = _getBuff( compressedLen + sizeof(MsgHeader), &_pCompressBuff, _compressBuffLen ) ;
      if ( !pBuff )
      {
         PD_LOG( PDERROR, "Failed to alloc compress buff, size: %d",
                 compressedLen + sizeof(MsgHeader) ) ;
         rc = SDB_OOM ;
         goto error ;
      }

      rc = compressor->compress( (const CHAR*)message + sizeof(MsgHeader), uncompressedLen,
                                 (CHAR*)pBuff + sizeof(MsgHeader), compressedLen ) ;
      if ( rc )
      {
         if ( SDB_UTIL_COMPRESS_ABORT == rc )
         {
            rc = SDB_OK ;
            goto done ;
         }
         else
         {
            goto error ;
         }
      }

      OSS_BIT_SET( headerCpy.flags, FLAG_COMPRESSED ) ;
      headerCpy.messageLength = sizeof(MsgHeader) + compressedLen ;
      ossMemcpy( pBuff, (CHAR*)(&headerCpy), sizeof(MsgHeader) ) ;
      messageLenDes = headerCpy.messageLength ;
      *des = &pBuff[0] ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _netMsgCompressor::compressNetMsg( const MsgHeader *message, const netIOVec &iov,
                                            CHAR** headerDes, netIOVec &iovDes )
   {
      SDB_ASSERT ( message && headerDes, "Invalid message" ) ;
      INT32 rc = SDB_OK ;
      utilCompressor * compressor = NULL ;
      CHAR *pBuff = NULL ;
      CHAR *pTmpBuff = NULL ;
      BOOLEAN hasCompressed = FALSE ;
      UINT32 uncompressedLenSum = message->messageLength - sizeof(MsgHeader) ;
      UINT32 compressedLen = 0 ;
      UINT32 needCompressedBytes = 0 ;
      netIOV newIov ;

      *headerDes = (CHAR*)message ;

      if ( !_needCompressMsg( message ) )
      {
         goto done ;
      }

      compressor = getCompressorByType( _compressor ) ;
      if ( !compressor )
      {
         PD_LOG( PDWARNING, "Failed to get compressor, rc: %d", SDB_SYS ) ;
         goto done ;
      }

      rc = compressor->compressBound( uncompressedLenSum, compressedLen ) ;
      PD_RC_CHECK( rc, PDERROR, "Failed to get max compressed length, rc: %d", rc ) ;

      pBuff = _getBuff( compressedLen + sizeof(MsgHeader), &_pCompressBuff, _compressBuffLen ) ;
      if ( !pBuff )
      {
         PD_LOG( PDERROR, "Failed to alloc compress buff, size: %d",
                 compressedLen + sizeof(MsgHeader) ) ;
         rc = SDB_OOM ;
         goto error ;
      }

      pTmpBuff = _getBuff( uncompressedLenSum, &_pTmpCompressBuff, _tmpCompressBuffLen ) ;
      if ( !pTmpBuff )
      {
         PD_LOG( PDERROR, "Failed to alloc compress buff, size: %d",
                 uncompressedLenSum ) ;
         rc = SDB_OOM ;
         goto error ;
      }

      for ( netIOVec::const_iterator itr = iov.begin(); itr != iov.end();
            ++itr )
      {
         if ( itr->iovBase && itr->iovLen > 0 )
         {
            ossMemcpy( pTmpBuff + needCompressedBytes, itr->iovBase, itr->iovLen ) ;
            needCompressedBytes += itr->iovLen ;
         }
      }

      SDB_ASSERT( needCompressedBytes == uncompressedLenSum, "Invalid msg length" ) ;

      rc = compressor->compress( (const CHAR*)pTmpBuff, uncompressedLenSum,
                                 (CHAR*)pBuff + sizeof(MsgHeader), compressedLen ) ;
      if ( rc )
      {
         if ( SDB_UTIL_COMPRESS_ABORT == rc )
         {
            rc = SDB_OK ;
            goto done ;
         }
         else
         {
            goto error ;
         }
      }

      ossMemcpy( pBuff, (CHAR*)message, sizeof(MsgHeader) ) ;
      ((MsgHeader*)pBuff)->messageLength = sizeof(MsgHeader) + compressedLen ;
      OSS_BIT_SET( ((MsgHeader*)pBuff)->flags, FLAG_COMPRESSED ) ;

      newIov.iovBase = (CHAR*)pBuff + sizeof(MsgHeader) ;
      newIov.iovLen = compressedLen ;

      try
      {
         iovDes.push_back( newIov ) ;
      }
      catch( std::exception &e )
      {
         rc = ossException2RC( &e ) ;
         PD_LOG( PDERROR, "An exception occurred when pushing new iov: "
                 "%s, rc: %d", e.what(), rc ) ;
         goto error ;
      }

      *headerDes = &pBuff[0] ;
      hasCompressed = TRUE ;

   done:
      if ( !hasCompressed )
      {
         iovDes = iov ;
      }
      return rc ;
   error:
      goto done ;
   }

   INT32 _netMsgCompressor::decompressNetMsg( const MsgHeader *message, CHAR** des )
   {
      SDB_ASSERT ( des && message, "Invalid message" ) ;
      INT32 rc = SDB_OK ;
      CHAR* pBuff = NULL ;
      UINT32 uncompressedLen = 0 ;
      utilCompressor * compressor = NULL ;
      const CHAR* compressedData = NULL ;
      UINT32 compressedLen = 0 ;
      MsgHeader headerCpy ;

      *des = (CHAR*)message ;

      if ( !_isCompressedMsg( message ) )
      {
         goto done ;
      }

      headerCpy = *message ;

      compressor = getCompressorByType( _compressor ) ;
      if ( !compressor )
      {
         rc = SDB_SYS ;
         PD_LOG( PDERROR, "Failed to get compressor, rc: %d", rc ) ;
         goto error ;
      }

      compressedData = (CHAR*)message + sizeof(MsgHeader) ;
      compressedLen = message->messageLength - sizeof(MsgHeader) ;

      rc = compressor->getUncompressedLen( compressedData, compressedLen,
                                           uncompressedLen ) ;
      PD_RC_CHECK( rc, PDERROR, "Failed to get uncompressed length, rc: %d", rc ) ;

      pBuff = _getBuff( uncompressedLen + sizeof(MsgHeader),
                        &_pUncompressBuff, _uncompressBuffLen ) ;
      if ( !pBuff )
      {
         PD_LOG( PDERROR, "Failed to allocate decompression buff, size: %d",
                 uncompressedLen + sizeof(MsgHeader) ) ;
         rc = SDB_OOM ;
         goto error ;
      }

      rc = compressor->decompress( compressedData, compressedLen, pBuff + sizeof(MsgHeader),
                                   uncompressedLen ) ;
      PD_RC_CHECK( rc, PDERROR, "Failed to decompress data, rc: %d", rc ) ;

      OSS_BIT_CLEAR( headerCpy.flags, FLAG_COMPRESSED ) ;
      headerCpy.messageLength = sizeof(MsgHeader) + uncompressedLen ;
      ossMemcpy( pBuff, (CHAR*)(&headerCpy), sizeof(MsgHeader) ) ;
      *des = pBuff ;

   done:
      return rc ;
   error:
      PD_LOG( PDERROR,
              "Failed to decompress msg [ length: %d, type: [%d]%d, tid: %d, "
              "routeID: %d.%d.%d, requestID: %lld, flags: %u ], rc: %d",
              message->messageLength,
              IS_REPLY_TYPE(message->opCode), GET_REQUEST_TYPE(message->opCode), message->TID,
              message->routeID.columns.groupID, message->routeID.columns.nodeID,
              message->routeID.columns.serviceID,
              message->requestID, message->flags, rc ) ;
      goto done ;
   }

   void _netMsgCompressor::setCompressor( UTIL_COMPRESSOR_TYPE netCompressor )
   {
      _compressor = netCompressor ;
   }

   INT32 _netMsgCompressor::_allocBuff( UINT32 len, CHAR **ppBuff, UINT32 *pRealSize )
   {
      INT32 rc = SDB_OK ;

      *ppBuff = (CHAR*)SDB_THREAD_ALLOC( len ) ;
      if ( !(*ppBuff) )
      {
         PD_LOG( PDERROR, "Faield to malloc memory[size: %d] failed", len ) ;
         rc = SDB_OOM ;
         goto error ;
      }
      *pRealSize = len ;
      ossMemset( *ppBuff, 0, len ) ;

   done:
      return rc ;
   error:
      goto done ;
   }

   void _netMsgCompressor::setNeedReleaseUncompressBuff( BOOLEAN need )
   {
      _needReleaseUncompressBuff = need ;
   }

   CHAR* _netMsgCompressor::_getBuff( UINT32 desLen, CHAR** buff, UINT32 &realLen )
   {
      if ( realLen < desLen )
      {
         if ( *buff )
         {
            SDB_THREAD_FREE( *buff ) ;
            *buff = NULL ;
            realLen = 0 ;
         }

         _allocBuff( desLen, buff, &realLen ) ;
      }

      return *buff ;
   }

   BOOLEAN _netMsgCompressor::_needCompressMsg( const MsgHeader *message )
   {
      SDB_ASSERT ( message, "Invalid message" ) ;

      if ( (INT32)MSG_SYSTEM_INFO_LEN == message->messageLength ||
           _compressor == UTIL_COMPRESSOR_INVALID ||
           message->messageLength < NET_COMPRESS_MIN_SIZE ||
           OSS_BIT_TEST( message->flags, FLAG_NOCOMPRESSED_ADVICE ) )
      {
         return FALSE ;
      }
      else
      {
         return TRUE ;
      }
   }

   BOOLEAN _netMsgCompressor::_isCompressedMsg( const MsgHeader *message )
   {
      SDB_ASSERT ( message, "Invalid message" ) ;

      if ( (INT32)MSG_SYSTEM_INFO_LEN == message->messageLength ||
           UTIL_COMPRESSOR_INVALID == _compressor ||
           !OSS_BIT_TEST( message->flags, FLAG_COMPRESSED ) )
      {
         return FALSE ;
      }
      else
      {
         return TRUE ;
      }
   }
}