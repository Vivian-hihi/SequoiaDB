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
      PD_TRACE_ENTRY( SDB__UTILCOMPRESSORLZW_COMPRESSBOUND ) ;

      if ( UTIL_INVALID_DICT == dictionary )
      {
         PD_LOG( PDERROR, "Dictionary is required for lzw compressor" ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }

      maxCompressedLen =
         ( (( utilLZWDictHead *)dictionary)->_codeSize * srcLen  + 7 ) / 8
         + sizeof( UINT32 ) ;
   done:
      PD_TRACE_EXITRC( SDB__UTILCOMPRESSORLZW_COMPRESSBOUND, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__UTILCOMPRESSORLZW_COMPRESS, "_utilCompressorLZW::compress" )
   INT32 _utilCompressorLZW::compress( const CHAR *source, UINT32 sourceLen,
                                       CHAR *dest, UINT32 &destLen,
                                       const utilDictHandle dictHandle )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__UTILCOMPRESSORLZW_COMPRESS ) ;

      UINT32 curPos = 0 ;
      UINT32 length = 0;
      UINT32 remainLen = sourceLen ;
      utilLZWContext context ;
      LZW_CODE code = DICT_INVALID_NODE ;
      utilLZWDictionary dictionary ;

      if ( UTIL_INVALID_DICT == dictHandle )
      {
         PD_LOG( PDERROR, "Dictionary is required for lzw compressor" ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }

      dictionary.attach( dictHandle ) ;
      context.setDictionary( &dictionary ) ;

      context._stream = (BYTE* )dest ;
      context._streamLen = destLen ;

      /* Store the header at the beginning. */
      (( _utilLZWHeader * )( context._stream ))->_uncompressedLen = sourceLen ;
      context._streamPos += sizeof( _utilLZWHeader ) ;

      do
      {
         length = remainLen ;
         code = dictionary.findStrExt( (BYTE*)(source + curPos), length ) ;
         _writeCode( &context, code ) ;
         curPos += length ;
         remainLen -= length ;
      } while ( remainLen > 0 ) ;

      _flushBits( &context ) ;
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
      UINT32 strLen = 0 ;
      LZW_CODE code = DICT_INVALID_NODE ;
      UINT32 totalOut = 0 ;
      utilLZWContext context ;
      utilLZWDictionary dictionary ;

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

      for( ; context._streamPos < context._streamLen; )
      {
         code = _readCode( &context ) ;
         strLen = dictionary.getStrExt( code, (UINT8*)(dest + totalOut),
                                        destLen - totalOut ) ;
         totalOut += strLen ;
      }

      destLen = totalOut ;

   done:
      PD_TRACE_EXITRC( SDB__UTILCOMPRESSORLZW_DECOMPRESS, rc ) ;
      return rc ;
   error:
      goto done ;
   }
}

