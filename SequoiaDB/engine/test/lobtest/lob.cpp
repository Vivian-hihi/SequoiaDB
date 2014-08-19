#include "client.h"
#include <stdio.h>

const static INT32 size = 1024* 1024 + 5 ;
CHAR *buf ;

static INT32 loop = 0 ;

int a( )
{
   int rc = SDB_OK ;
   sdbConnectionHandle conn = SDB_INVALID_HANDLE ;
   sdbCollectionHandle cl = SDB_INVALID_HANDLE ;
   sdbLobHandle lob = SDB_INVALID_HANDLE ;
   bson_oid_t oid ;
   UINT32 read = 0 ;

   rc = sdbConnect( "localhost", "11810", "", "", &conn ) ;
   if ( SDB_OK != rc )
   {
      goto error ;
   }

   rc = sdbGetCollection( conn, "foo.bar", &cl ) ;
   if ( SDB_OK != rc )
   {
      goto error ;
   }
   for ( UINT32 i = 0; i < 100000; i++ )
   { 
   bson_oid_gen( &oid ) ;
   rc = sdbOpenLob( cl, &oid, SDB_LOB_CREATEONLY, &lob ) ;
   if ( SDB_OK != rc )
   {
      goto error ;
   }
   printf("opened\n") ;

   for ( UINT32 i = 0; i < 5; i++ )
   {
   memset( buf, 'a' + i, size ) ;
   rc = sdbWriteLob( lob, buf, size ) ;
   if ( SDB_OK != rc )
   {
      goto error ;
   }
   }
   printf("write done\n") ;

   rc = sdbCloseLob( &lob ) ;
   if ( SDB_OK != rc )
   {
      goto error ;
   }
   printf("close\n" ) ;

   if ( loop++ < 1000 )
   {
      goto done ;
   }

//   getchar() ;

   rc = sdbOpenLob( cl, &oid, SDB_LOB_READ, &lob ) ;
   if ( SDB_OK != rc )
   {
      goto error ;
   }
   printf("opened\n") ;

   {
   UINT32 total  = 0 ;
   while ( total < size * 5 )
   {
      memset( buf, 0, size ) ;
      UINT32 read = 0 ;
      rc = sdbReadLob( lob, size - 1, buf, &read ) ;
      if ( SDB_OK != rc )
      {
         goto error ;
      }

      printf( "loop read:%d\n", read) ;
      total += read ;
   }

   rc = sdbReadLob( lob, size - 1, buf, &read ) ;
   if ( SDB_EOF == rc )
   {
      rc = SDB_OK ;
   }
   else
   {
      rc = SDB_SYS ;
      goto error ;
   }

   if ( total != size * 5 )
   {
      printf("%d", total );
      rc = SDB_SYS ;
      goto error ;
   }
   }

   rc = sdbCloseLob( &lob ) ;
   if ( SDB_OK != rc )
   {
      goto error ;
   }
   printf("close\n" ) ;

   rc = sdbRemoveLob( cl, &oid ) ;
   if ( SDB_OK != rc )
   {
      goto error ;
   }
   printf("removed\n" );

   rc = sdbOpenLob( cl, &oid, 0x00000004, &lob ) ;
   if ( SDB_FNE != rc )
   {
      goto error ;
   }
   else if ( SDB_OK == rc )
   {
      rc = -10 ;
      goto error ;
   }
   else
   {
      rc = SDB_OK ;
   }
   }
done:
   if ( SDB_INVALID_HANDLE != lob )
   {
      sdbCloseLob( &lob ) ;
   }
   if ( SDB_INVALID_HANDLE != cl )
   {
      sdbReleaseCollection( cl ) ;
   }
   if ( SDB_INVALID_HANDLE != conn )
   {
      sdbDisconnect( conn ) ;
      sdbReleaseConnection( conn ) ;
   }

   printf("%d",rc ) ;
   return rc ;
error:
   goto done ;
}


int main()
{
   buf = new char[size] ;

   for ( UINT32 i = 0; i < 100000; i++)
   {
      int rc = a() ;
      if ( SDB_OK != rc )
      {
         printf("%d", rc ) ;
         break ;
      }
   }
   delete []buf ;
   return 0 ;
}
