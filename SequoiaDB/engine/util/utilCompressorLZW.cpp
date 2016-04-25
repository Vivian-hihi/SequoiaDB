/*******************************************************************************


   Copyright (C) 2011-2016 SequoiaDB Ltd.

   This program is free software: you can redistribute it and/or modify
   it under the term of the GNU Affero General Public License, version 3,
   as published by the Free Software Foundation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warrenty of
   MARCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU Affero General Public License for more details.

   You should have received a copy of the GNU Affero General Public License
   along with this program. If not, see <http://www.gnu.org/license/>.

   Source File Name = utilCompressorLZW.cpp

   Descriptive Name = Implementation of LZW compression.

   When/how to use:

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          07/12/2015  YSD Initial Draft

   Last Changed =

*******************************************************************************/
#include "pd.hpp"
#include "utilCompressorLZW.hpp"
#include "pdTrace.hpp"
#include "utilTrace.hpp"

namespace engine
{
   // PD_TRACE_DECLARE_FUNCTION ( SDB__UTILCOMPRESSORLZW_CONSTRUCTOR, "_utilCompressorLZW::_utilCompressorLZW" )
   _utilCompressorLZW::_utilCompressorLZW()
   : _utilCompressor( UTIL_COMPRESSOR_LZW )
   {
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__UTILCOMPRESSORLZW__COMPRESSLEVELONE, "_utilCompressorLZW::_compressLevelOne" )
   INT32 _utilCompressorLZW::_compressLevelOne( utilLZWContext &context,
                                                const CHAR* source,
                                                UINT32 sourceLen,
                                                UINT32 maxSize )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__UTILCOMPRESSORLZW__COMPRESSLEVELONE ) ;
      LZW_CODE code = 0 ;
      UINT32 length = 0 ;
      UINT32 remainLen = sourceLen ;
      UINT32 currPos = 0 ;
      UINT32 codeNum = 0 ;
      UINT32 maxCodeNum = 0 ;
      utilLZWDictionary *dictionary = context.getDictionary() ;
      SDB_ASSERT( dictionary, "Dictionary should not be NULL" ) ;

      maxCodeNum = maxSize * 8 / dictionary->getCodeSize() ;
      do
      {
         length = remainLen ;
         code = dictionary->findStrExt( (BYTE*)(source + currPos), length ) ;
         if ( ++codeNum > maxCodeNum )
         {
            rc = SDB_UTIL_COMPRESS_ABORT ;
            goto error ;
         }

         _writeCode( &context, code ) ;
         currPos += length ;
         remainLen -= length ;
      } while ( remainLen > 0 ) ;

      _flushBits( &context ) ;
   done:
      PD_TRACE_EXITRC( SDB__UTILCOMPRESSORLZW__COMPRESSLEVELONE, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__UTILCOMPRESSORLZW__COMPRESSLEVELTWO, "_utilCompressorLZW::_compressLevelTwo" )
   INT32 _utilCompressorLZW::_compressLevelTwo( utilLZWContext &context,
                                                const CHAR* source,
                                                UINT32 sourceLen,
                                                UINT32 maxSize )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__UTILCOMPRESSORLZW__COMPRESSLEVELTWO ) ;
      LZW_CODE code = 0 ;
      UINT32 length = 0 ;
      UINT32 remainLen = sourceLen ;
      UINT32 currPos = 0 ;
      UINT32 maxBitNum = maxSize * 8 ;
      UINT32 totalBitNum = 0 ;
      utilLZWDictionary *dictionary = context.getDictionary() ;
      SDB_ASSERT( dictionary, "Dictionary should not be NULL" ) ;

      do
      {
         length = remainLen ;
         code = dictionary->findStrExt( (BYTE*)(source + currPos), length ) ;
         totalBitNum += _writeVarLenCode( &context, code ) ;
         if ( totalBitNum > maxBitNum )
         {
            rc = SDB_UTIL_COMPRESS_ABORT ;
            goto error ;
         }
         currPos += length ;
         remainLen -= length ;
      } while ( remainLen > 0 ) ;

      _flushBits( &context ) ;
   done:
      PD_TRACE_EXITRC( SDB__UTILCOMPRESSORLZW__COMPRESSLEVELTWO, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__UTILCOMPRESSORLZW__COMPRESSLEVELTHREE, "_utilCompressorLZW::_compressLevelThree" )
   INT32 _utilCompressorLZW::_compressLevelThree( utilLZWContext &context,
                                                  const CHAR* source,
                                                  UINT32 sourceLen,
                                                  UINT32 maxSize )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__UTILCOMPRESSORLZW__COMPRESSLEVELTHREE ) ;

   done:
      PD_TRACE_EXITRC( SDB__UTILCOMPRESSORLZW__COMPRESSLEVELTHREE, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__UTILCOMPRESSORLZW__DECOMPRESSLEVELONE, "_utilCompressorLZW::_decompressLevelOne" )
   void _utilCompressorLZW::_decompressLevelOne( utilLZWContext &context,
                                                 CHAR *dest,
                                                 UINT32 &destLen )
   {
      PD_TRACE_ENTRY( SDB__UTILCOMPRESSORLZW__DECOMPRESSLEVELONE ) ;
      UINT32 strLen = 0 ;
      UINT32 totalOut = 0 ;
      LZW_CODE code = UTIL_INVALID_DICT_CODE ;
      utilLZWDictionary *dictionary = context.getDictionary() ;
      SDB_ASSERT( dictionary, "Dictionary should not be NULL" ) ;

      for( ; context._streamPos < context._streamLen; )
      {
         code = _readCode( &context ) ;
         strLen = dictionary->getStrExt( code, (UINT8*)(dest + totalOut),
                                         destLen - totalOut ) ;
         totalOut += strLen ;
      }

      destLen = totalOut ;
      PD_TRACE_EXIT( SDB__UTILCOMPRESSORLZW__DECOMPRESSLEVELONE ) ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__UTILCOMPRESSORLZW__DECOMPRESSLEVELTWO, "_utilCompressorLZW::_decompressLevelTwo" )
   void _utilCompressorLZW::_decompressLevelTwo( utilLZWContext &context,
                                                 CHAR *dest,
                                                 UINT32 &destLen)
   {
      PD_TRACE_ENTRY( SDB__UTILCOMPRESSORLZW__DECOMPRESSLEVELTWO ) ;
      UINT32 strLen = 0 ;
      UINT32 totalOut = 0 ;
      LZW_CODE code = UTIL_INVALID_DICT_CODE ;
      utilLZWDictionary *dictionary = context.getDictionary() ;
      SDB_ASSERT( dictionary, "Dictionary should not be NULL" ) ;

      for( ; context._streamPos < context._streamLen; )
      {
         code = _readVarLenCode( &context ) ;
         strLen = dictionary->getStrExt( code, (UINT8*)(dest + totalOut),
                                         destLen - totalOut ) ;
         totalOut += strLen ;
      }

      destLen = totalOut ;
      PD_TRACE_EXIT( SDB__UTILCOMPRESSORLZW__DECOMPRESSLEVELTWO ) ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__UTILCOMPRESSORLZW__DECOMPRESSLEVELTHREE, "_utilCompressorLZW::_decompressLevelThree" )
   void _utilCompressorLZW::_decompressLevelThree( utilLZWContext &context,
                                                   CHAR *dest,
                                                   UINT32 &destLen)
   {
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__UTILCOMPRESSORLZW_COMPRESSBOUND, "_utilCompressorLZW::compressBound" )
   INT32 _utilCompressorLZW::compressBound( UINT32 srcLen,
                                            UINT32 &maxCompressedLen,
                                            const utilDictHandle dictionary )
   {
      /*
       * In the worst scenario, no string in the source with length greater than
       * 1 can be found in the dictionary. In this case, each character in the
       * source should be represented by one dictionary code separately. If the
       * code size is greater than 8, the data will expand after encoding...
       * 4 more bytes are reserved at the beginning to store the original length
       */
      INT32 rc = SDB_OK ;
      UINT64 size = 0 ;
      PD_TRACE_ENTRY( SDB__UTILCOMPRESSORLZW_COMPRESSBOUND ) ;

      if ( UTIL_INVALID_DICT == dictionary )
      {
         PD_LOG( PDERROR, "Dictionary is required for lzw compressor" ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }

      size = ( (( utilLZWDictHead *)dictionary)->_codeSize * srcLen  + 7 ) / 8
             + sizeof( UINT32 ) ;
      /* Overflow check */
      if ( 0 != ( size >> 32 ) )
      {
         PD_LOG( PDERROR, "Input length too big: %u", srcLen ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }

      maxCompressedLen = size ;

   done:
      PD_TRACE_EXITRC( SDB__UTILCOMPRESSORLZW_COMPRESSBOUND, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__UTILCOMPRESSORLZW_COMPRESS, "_utilCompressorLZW::compress" )
   INT32 _utilCompressorLZW::compress( const CHAR *source, UINT32 sourceLen,
                                       CHAR *dest, UINT32 &destLen,
                                       const utilDictHandle dictHandle,
                                       const utilCompressStrategy *strategy )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__UTILCOMPRESSORLZW_COMPRESS ) ;
      utilLZWContext context ;
      utilLZWDictionary dictionary ;
      UINT8 minRatio = 0 ;
      UTIL_COMPRESSION_LEVEL level = UTIL_COMPRESSOR_DFT_LEVEL ;
      UINT32 maxSize = 0 ;

      if ( UTIL_INVALID_DICT == dictHandle )
      {
         PD_LOG( PDERROR, "Dictionary is required for lzw compressor" ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }

      if ( !strategy )
      {
         minRatio = UTIL_COMPRESSOR_DFT_MIN_RATIO ;
         level = UTIL_COMPRESSOR_DFT_LEVEL ;
      }
      else
      {
         minRatio = strategy->_minRatio ;
         level = strategy->_level ;
      }

      maxSize = sourceLen * minRatio / 100 ;

      dictionary.attach( dictHandle ) ;
      context.setDictionary( &dictionary ) ;
      context._stream = (BYTE* )dest ;
      context._streamLen = destLen ;

      /* Store the header at the beginning. */
      (( _utilLZWHeader * )( context._stream ))->_uncompressedLen = sourceLen ;
      context._streamPos += sizeof( _utilLZWHeader ) ;

      switch( level )
      {
         case UTIL_COMP_BEST_SPEED:
            rc = _compressLevelOne( context, source, sourceLen, maxSize ) ;
            break ;
         case UTIL_COMP_BALANCE:
            rc = _compressLevelTwo( context, source, sourceLen, maxSize ) ;
            break ;
         default:
            rc = _compressLevelThree( context, source, sourceLen, maxSize ) ;
            break ;
      }

      PD_RC_CHECK( rc, PDERROR, "Failed to compress data, rc: %d", rc ) ;

      destLen = context._streamPos ;

   done:
      PD_TRACE_EXITRC( SDB__UTILCOMPRESSORLZW_COMPRESS, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__UTILCOMPRESSORLZW_GETUNCOMPRESSLEN, "_utilCompressorLZW::getUncompressedLen" )
   INT32 _utilCompressorLZW::getUncompressedLen( const CHAR *source,
                                                 UINT32 sourceLen,
                                                 UINT32 &length )
   {
      PD_TRACE_ENTRY( SDB__UTILCOMPRESSORLZW_GETUNCOMPRESSLEN ) ;
      length = ((_utilLZWHeader *)source)->_uncompressedLen ;

      PD_TRACE_EXIT( SDB__UTILCOMPRESSORLZW_GETUNCOMPRESSLEN) ;

      return SDB_OK ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__UTILCOMPRESSORLZW_DECOMPRESS, "_utilCompressorLZW::decompress" )
   INT32 _utilCompressorLZW::decompress( const CHAR *source, UINT32 sourceLen,
                                         CHAR *dest, UINT32 &destLen,
                                         const utilDictHandle dictHandle )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__UTILCOMPRESSORLZW_DECOMPRESS ) ;
      utilLZWContext context ;
      utilLZWDictionary dictionary ;
      UINT32 level = UTIL_COMPRESSOR_DFT_LEVEL ;

      if ( UTIL_INVALID_DICT == dictHandle )
      {
         PD_LOG( PDERROR, "Dictionary is required for lzw compressor" ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }

      dictionary.attach( dictHandle ) ;
      context.setDictionary( &dictionary ) ;

      context._stream = (BYTE *)source ;
      context._streamLen = sourceLen ;
      /* Skip the length at the beginning. */
      context._streamPos = sizeof( UINT32 ) ;

      switch( level )
      {
         case UTIL_COMP_BEST_SPEED:
            _decompressLevelOne( context, dest, destLen );
            break ;
         case UTIL_COMP_BALANCE:
            _decompressLevelTwo( context, dest, destLen ) ;
            break ;
         default:
            _decompressLevelThree( context, dest, destLen ) ;
            break ;
      }

   done:
      PD_TRACE_EXITRC( SDB__UTILCOMPRESSORLZW_DECOMPRESS, rc ) ;
      return rc ;
   error:
      goto done ;
   }
}

