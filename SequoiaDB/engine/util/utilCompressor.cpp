#include "utilCompressorSnappy.hpp"
#include "utilCompressorLZW.hpp"
#include "msgDef.hpp"

namespace engine
{
   utilCompressor* getCompressorByType( UTIL_COMPRESSOR_TYPE type )
   {
      static utilCompressorSnappy snappyCompressor ;
      static utilCompressorLZW lzwCompressor ;

      if ( UTIL_COMPRESSOR_LZW == type )
      {
         return &lzwCompressor ;
      }
      else if ( UTIL_COMPRESSOR_SNAPPY == type )
      {
         return &snappyCompressor ;
      }
      else
      {
         return NULL ;
      }
   }

   const CHAR *utilCompressType2String( UINT8 type )
   {
      switch( type )
      {
         case UTIL_COMPRESSOR_SNAPPY :
            return VALUE_NAME_SNAPPY ;
            break ;
         case UTIL_COMPRESSOR_LZW :
            return VALUE_NAME_LZW ;
            break ;
         default :
            return "Invalid" ;
      }
   }

} /* engine */

