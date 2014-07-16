#include "ossPrimitiveFileOp.hpp"
#include "pd.hpp"
#include "omagentUtil.hpp"

namespace CLSMGR
{

   INT32 checkBuffer ( CHAR **ppBuffer, INT32 *bufferSize,
                       INT32 packetLength )
   {
      INT32 rc = SDB_OK ;
      if ( packetLength > *bufferSize )
      {
         CHAR *pOrigMem = *ppBuffer ;
         INT32 newSize = ossRoundUpToMultipleX ( packetLength, SDB_PAGE_SIZE ) ;
         if ( newSize < 0 )
         {
            pdLog ( PDERROR, __FUNC__, __FILE__, __LINE__,
                    "new buffer overflow" ) ;
            rc = SDB_INVALIDARG ;
            goto error ;
         }
         *ppBuffer = (CHAR*)SDB_OSS_REALLOC ( *ppBuffer, sizeof(CHAR)*(newSize)) ;
         if ( !*ppBuffer )
         {
            PD_LOG ( PDERROR, "Failed to allocate %d bytes send buffer",
                     newSize ) ;
            rc = SDB_OOM ;
            // realloc does NOT free original memory if it fails, so we have to
            // assign pointer to original
            *ppBuffer = pOrigMem ;
            goto error ;
         }
         *bufferSize = newSize ;
      }
   done :
      return rc ;
   error :
      goto done ;
   }


   INT32 readFile ( const CHAR * name , CHAR ** buf , UINT32 * bufSize,
                    UINT32 * readSize )
   {
      ossPrimitiveFileOp op ;
      ossPrimitiveFileOp::offsetType offset ;
      INT32 rc = SDB_OK ;

      SDB_ASSERT ( name && buf && bufSize, "Invalid arguments" ) ;

      rc = op.Open ( name , OSS_PRIMITIVE_FILE_OP_READ_WRITE ) ;
      if ( rc != SDB_OK )
      {
         ossPrintf ( "Can't open file: %s"OSS_NEWLINE, name ) ;
         goto error ;
      }

      rc = op.getSize ( &offset ) ;
      if ( rc != SDB_OK )
      {
         goto error ;
      }

      if ( *bufSize < offset.offset + 1 )
      {
         if ( *buf )
         {
            SDB_OSS_FREE( *buf ) ;
            *buf = NULL ;
            *bufSize = 0 ;
         }
         *buf = (CHAR *) SDB_OSS_MALLOC ( offset.offset + 1 ) ;
         if ( ! *buf )
         {
            rc = SDB_OOM ;
            PD_LOG ( PDERROR , "fail to alloc memory" ) ;
            goto error ;
         }
         *bufSize = offset.offset + 1 ;
      }

      rc = op.Read ( offset.offset , *buf , NULL ) ;
      if ( rc != SDB_OK )
      {
         goto error ;
      }
      (*buf)[ offset.offset ] = 0 ;
      if ( readSize ) *readSize = offset.offset ;

   done :
      op.Close() ;
      return rc ;
   error :
      goto done ;
   }

}
