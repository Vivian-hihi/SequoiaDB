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
         _writeCode( &context, code ) ;
         if ( ++codeNum > maxCodeNum )
         {
            rc = SDB_UTIL_COMPRESS_ABORT ;
            goto error ;
         }

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
                                                  UINT32 maxSize,
                                                  BOOLEAN &varLenCode )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__UTILCOMPRESSORLZW__COMPRESSLEVELTHREE ) ;
      UINT32 length = 0 ;
      UINT32 remainLen = sourceLen ;
      UINT32 currPos = 0 ;
      LZW_CODE code = UTIL_INVALID_DICT_CODE ;
      UINT8 lenIdx = 0 ;
      UINT8 splitSize = 0 ;
      UINT32 varLenTotalBits = 0 ;
      BOOLEAN useVarLenComp = FALSE ;
      UINT32 fixLenTotalBits = 0 ;
      utilLZWDictionary *dictionary = context.getDictionary() ;
      SDB_ASSERT( dictionary, "Dictionary should not be NULL" ) ;
      vector<LZW_CODE> codeVec ;

      do
      {
         length = remainLen ;
         code = dictionary->findStrExt( (BYTE*)(source + currPos ), length ) ;
         dictionary->getVarLenInfo( code, lenIdx, splitSize ) ;
         codeVec.push_back( code ) ;
         varLenTotalBits += splitSize + UTIL_VAR_LEN_FLAG_SIZE ;
         currPos += length ;
         remainLen -= length ;
      } while ( remainLen > 0 ) ;

      fixLenTotalBits = codeVec.size() * dictionary->getCodeSize() ;

      useVarLenComp = ( ( varLenTotalBits * 100 / fixLenTotalBits ) <= 95 ) ?
                      TRUE : FALSE ;

      if ( useVarLenComp )
      {
         if ( varLenTotalBits > ( maxSize * 8 ) )
         {
            rc = SDB_UTIL_COMPRESS_ABORT ;
            goto error ;
         }

         for ( vector<LZW_CODE>::iterator itr = codeVec.begin();
               itr != codeVec.end(); ++itr )
         {
            _writeVarLenCode( &context, *itr ) ;
         }
      }
      else
      {
         if ( fixLenTotalBits > ( maxSize * 8 ) )
         {
            rc = SDB_UTIL_COMPRESS_ABORT ;
            goto error ;
         }

         for ( vector<LZW_CODE>::iterator itr = codeVec.begin();
               itr != codeVec.end(); ++itr )
         {
            _writeCode( &context, *itr ) ;
         }
      }

      _flushBits( &context ) ;

      varLenCode = useVarLenComp ;

   done:
      PD_TRACE_EXITRC( SDB__UTILCOMPRESSORLZW__COMPRESSLEVELTHREE, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__UTILCOMPRESSORLZW__DECODEFIXLENCODE, "_utilCompressorLZW::_decodeFixLenCode" )
   void _utilCompressorLZW::_decodeFixLenCode( utilLZWContext &context,
                                               CHAR *dest, UINT32 &destLen )
   {
      PD_TRACE_ENTRY( SDB__UTILCOMPRESSORLZW__DECODEFIXLENCODE ) ;
      UINT32 strLen = 0 ;
      UINT32 totalOut = 0 ;
      LZW_CODE code = UTIL_INVALID_DICT_CODE ;
      utilLZWDictionary *dictionary = context.getDictionary() ;
      SDB_ASSERT( dictionary, "Dictionary should not be NULL" ) ;

      while ( context._streamPos < context._streamLen )
      {
         code = _readCode( &context ) ;
         strLen = dictionary->getStrExt( code, (UINT8*)(dest + totalOut),
                                         destLen - totalOut ) ;
         totalOut += strLen ;
      }

      destLen = totalOut ;
      PD_TRACE_EXIT( SDB__UTILCOMPRESSORLZW__DECODEFIXLENCODE ) ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__UTILCOMPRESSORLZW__DECODEVARLENCODE, "_utilCompressorLZW::_decodeVarLenCode" )
   void _utilCompressorLZW::_decodeVarLenCode( utilLZWContext &context,
                                                 CHAR *dest,
                                                 UINT32 &destLen)
   {
      PD_TRACE_ENTRY( SDB__UTILCOMPRESSORLZW__DECODEVARLENCODE ) ;
      UINT32 strLen = 0 ;
      UINT32 totalOut = 0 ;
      LZW_CODE code = UTIL_INVALID_DICT_CODE ;
      utilLZWDictionary *dictionary = context.getDictionary() ;
      SDB_ASSERT( dictionary, "Dictionary should not be NULL" ) ;

      while ( context._streamPos < context._streamLen )
      {
         code = _readVarLenCode( &context ) ;
         strLen = dictionary->getStrExt( code, (UINT8*)(dest + totalOut),
                                         destLen - totalOut ) ;
         totalOut += strLen ;
      }

      destLen = totalOut ;
      PD_TRACE_EXIT( SDB__UTILCOMPRESSORLZW__DECODEVARLENCODE ) ;
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
      BOOLEAN varLenCode = FALSE ;
      _utilLZWHeader head ;

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

      head.setLength( sourceLen );
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
            rc = _compressLevelThree( context, source, sourceLen,
                                      maxSize, varLenCode ) ;
            break ;
      }

      PD_RC_CHECK( rc, PDERROR, "Failed to compress data, rc: %d", rc ) ;

      if ( varLenCode )
      {
         head.setVarLenFlag() ;
      }

      ossMemcpy( dest, &head, sizeof( _utilLZWHeader ) ) ;

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
      length = ((_utilLZWHeader *)source)->getLength() ;

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
      utilLZWHeader *head = NULL ;

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
      head = ( utilLZWHeader * )source ;

      if ( head->isVarLenCode() )
      {
         _decodeVarLenCode( context, dest, destLen ) ;
      }
      else
      {
         _decodeFixLenCode( context, dest, destLen );
      }

   done:
      PD_TRACE_EXITRC( SDB__UTILCOMPRESSORLZW_DECOMPRESS, rc ) ;
      return rc ;
   error:
      goto done ;
   }
}

