#include "openssl/md5.h"
#include <stdio.h>
#include <string.h>
int main ( int argc, char **argv )
{
   MD5_CTX c ;
   unsigned char md[MD5_DIGEST_LENGTH] ;
   if ( argc !=2 )
   {
      printf ( "Syntax: %s <string>\n", (char*)argv[0]) ;
      return 0 ;
   }
   char *pString = (char*)argv[1] ;
   MD5_Init(&c) ;
   MD5_Update(&c, pString, strlen(pString)) ;
   MD5_Final(&(md[0]), &c) ;
   for ( int i=0; i<MD5_DIGEST_LENGTH; i++)
   {
      printf("%02x", md[i]) ;
   }
   printf("\n") ;
   return 0 ;
}
