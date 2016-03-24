#include "utilCompressorSnappy.hpp"
#include "pd.hpp"
#include "pdTrace.hpp"
#include "utilTrace.hpp"

namespace engine
{
   // PD_TRACE_DECLARE_FUNCTION( SDB__UTILCOMPRESSORSNAPPY_COMPRESSBOUND, "_utilCompressorSnappy::compressBound")
   INT32 _utilCompressorSnappy::compressBound( UINT32 srcLen,
                                              UINT32 &maxCompressedLen,
                                              const CHAR *dictionary )
   {
      PD_TRACE_ENTRY( SDB__UTILCOMPRESSORSNAPPY_COMPRESSBOUND ) ;
      (void)dictionary ;

      SDB_ASSERT( !dictionary, "snappy does not use any dictionary" ) ;
      maxCompressedLen = ( UINT32 )snappy::MaxCompressedLength( srcLen ) ;

      PD_TRACE_EXIT( SDB__UTILCOMPRESSORSNAPPY_COMPRESSBOUND ) ;
      return SDB_OK ;
   }

   // PD_TRACE_DECLARE_FUNCTION( SDB__UTILCOMPRESSORSNAPPY_COMPRESS, "_utilCompressorSnappy::compress")
   INT32 _utilCompressorSnappy::compress( const CHAR *source, UINT32 sourceLen,
                                          CHAR *dest, UINT32 &destLen,
                                          const CHAR *dictionary )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__UTILCOMPRESSORSNAPPY_COMPRESS ) ;
      size_t resultLen = 0 ;
      (void)dictionary ;

      SDB_ASSERT( !dictionary, "snappy does not use any dictionary" ) ;
      if ( destLen < ( UINT32 )snappy::MaxCompressedLength( sourceLen ) )
      {
         PD_LOG( PDERROR, "Buffer for decompressed data is not big enough" ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }
      snappy::RawCompress ( source, (size_t)sourceLen, dest, &resultLen ) ;
      destLen = ( UINT32 )resultLen ;

   done:
      PD_TRACE_EXITRC( SDB__UTILCOMPRESSORSNAPPY_COMPRESS, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION( SDB__UTILCOMPRESSORSNAPPY_GETUNCOMPRESSEDLEN, "_utilCompressorSnappy::getUncompressedLen")
   INT32 _utilCompressorSnappy::getUncompressedLen( const CHAR *source,
                                                    UINT32 sourceLen,
                                                    UINT32 &length)
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__UTILCOMPRESSORSNAPPY_GETUNCOMPRESSEDLEN ) ;
      size_t resultLen = 0 ;
      if ( !snappy::GetUncompressedLength ( source, sourceLen, &resultLen ) )
      {
         rc = SDB_CORRUPTED_RECORD ;
         PD_LOG( PDERROR, "Failed to get uncompressed length" ) ;
         goto error ;
      }

      length = resultLen ;
   done:
      PD_TRACE_EXITRC( SDB__UTILCOMPRESSORSNAPPY_GETUNCOMPRESSEDLEN, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION( SDB__UTILCOMPRESSORSNAPPY_DECOMPRESS, "_utilCompressorSnappy::decompress")
   INT32 _utilCompressorSnappy::decompress( const CHAR *source,
                                            UINT32 sourceLen,
                                            CHAR *dest,
                                            UINT32 &destLen,
                                            const CHAR *dictionary )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__UTILCOMPRESSORSNAPPY_DECOMPRESS ) ;

      SDB_ASSERT( !dictionary, "snappy does not use any dictionary" ) ;

      if ( !snappy::RawUncompress ( source, (size_t)sourceLen, dest ) )
      {
         rc = SDB_CORRUPTED_RECORD ;
         PD_LOG( PDERROR, "Failed to uncompress record" )  ;
         goto error ;
      }
   done:
      PD_TRACE_EXITRC( SDB__UTILCOMPRESSORSNAPPY_DECOMPRESS, rc ) ;
      return rc ;
   error:
      goto done ;
   }
}

