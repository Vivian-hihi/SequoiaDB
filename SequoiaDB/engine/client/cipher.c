#include "ossMem.h"
#include "ossUtil.h"
#include "openssl/des.h"
#include "openssl/sha.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>


#define BYTES_PER_TIME               8
#define KEY_BYTE_LENGTH              8

#define RANDOM_ARRAY_MAX_LENGTH      16
#define ARRAY_CONTENT_LENGTH         RANDOM_ARRAY_MAX_LENGTH
#define USERNAME_MAX_LENGTH          1000

#define TOKEN_MAX_LENGTH             256
#define CIPHER_STRING_MAX_LENGTH     TOKEN_MAX_LENGTH + RANDOM_ARRAY_MAX_LENGTH

static INT16 _hexChar2dec( CHAR c )
{
   INT16 i = 0 ;
   if ( c >= '0' && c <= '9' )
   {
      i = c - '0' ;
   }
   else if ( c >= 'a' && c <= 'f' )
   {
      i = 10 + c - 'a' ;
   }
   else
   {
      i = 10 + c - 'A' ;
   }
   return i ;
}

static void _hexToByte( const CHAR *hex, CHAR *bytes, UINT32 *byteLength )
{
  UINT32 len = ossStrlen( hex ) ;
  const CHAR* base = hex ;
  UINT32 i = 0 ;
  UINT32 pos = 0 ;

  for ( ; i < len; i += 2, pos++ )
  {
	 const CHAR* c = base + i ;
	 bytes[pos] =  ( CHAR )( ( _hexChar2dec( c[0] ) << 4 ) |
							         _hexChar2dec( c[1] ) );
    ( *byteLength )++ ;
  }
}

static void _arrayRemoveElement( CHAR *array, UINT32 index,
                                 UINT32 count, UINT32 *arraySize )
{
   UINT32 removeLength = count > *arraySize - index ? *arraySize - index : count ;
   UINT32 copyLength = *arraySize - ( index + removeLength ) ;

   if ( NULL == array )
   {
      return ;
   }

   if ( *arraySize <= index )
   {
      return ;
   }

   ossMemcpy( array + index, array + index + removeLength, copyLength ) ;
   if ( NULL != arraySize )
   {
      *arraySize -= removeLength ;
   }
}

static void _arrayAppendElement( CHAR *array, UINT32 startPos, CHAR *appendArray,
                                 UINT32 index, UINT32 count, UINT32 *arraySize )
{
   if ( NULL == array )
   {
      return ;
   }
   
   ossMemcpy( array + startPos, appendArray + index, count ) ;

   if ( NULL != arraySize )
   {
      *arraySize += count ;
   }
}

static INT32 _extractRandomArray( CHAR *cipherText, UINT32 *cipherTextSize, 
                                  CHAR *array, UINT32 *arraySize )
{
   INT32 rc = SDB_OK ;

   UINT32         cipherLowPos = 0 ;
   UINT32         cipherMidPos  = 0 ;
   UINT32         cipherHighPos = 0 ;
   CHAR           arrayLowContent[ARRAY_CONTENT_LENGTH] = { '\0' } ;
   CHAR           arrayMidContent[ARRAY_CONTENT_LENGTH] = { '\0' } ;
   CHAR           arrayHighContent[ARRAY_CONTENT_LENGTH] = { '\0' } ;
   UINT32         arrayLowContentSize = 0 ;
   UINT32         arrayMidContentSize = 0 ;
   UINT8          arrayLowLen = 0 ;
   UINT8          arrayMidLen  = 0 ;
   UINT8          arrayHighLen = 0 ;

   if ( 0 == *cipherTextSize || 0 != *arraySize )
   {
      goto error ;
   }

   // calculate random array position in cipherText, then extract
   cipherLowPos = (UINT8)cipherText[0] ;
   if ( *cipherTextSize <= cipherLowPos )
   {
      goto error ;
   }
   _arrayRemoveElement( cipherText, 0, 1, cipherTextSize ) ;

   arrayLowLen = cipherText[cipherLowPos] ;
   ossMemcpy( arrayLowContent, cipherText + cipherLowPos, arrayLowLen + 2 ) ;
   arrayLowContentSize = arrayLowLen + 2 ;

   cipherMidPos = (UINT8)arrayLowContent[arrayLowContentSize - 1] ;
   if ( *cipherTextSize <= cipherMidPos )
   {
      goto error ;
   }
   arrayMidLen = cipherText[cipherMidPos] ;
   ossMemcpy( arrayMidContent, cipherText + cipherMidPos, arrayMidLen + 2 ) ;
   arrayMidContentSize = arrayMidLen + 2 ;

   cipherHighPos = (UINT8)arrayMidContent[arrayMidContentSize - 1] ;
   if ( *cipherTextSize <= cipherHighPos )
   {
      goto error ;
   }
   arrayHighLen = cipherText[cipherHighPos] ;
   ossMemcpy( arrayHighContent, cipherText + cipherHighPos, arrayHighLen + 1 ) ;

   // erase random array from cipherText.
   _arrayRemoveElement( cipherText, cipherHighPos, arrayHighLen + 1, cipherTextSize ) ;
   _arrayRemoveElement( cipherText, cipherMidPos, arrayMidLen + 2, cipherTextSize ) ;
   _arrayRemoveElement( cipherText, cipherLowPos, arrayLowLen + 2, cipherTextSize ) ;

   if ( RANDOM_ARRAY_MAX_LENGTH < ( arrayLowLen + arrayMidLen + arrayHighLen ) )
   {
      goto error ;
   }

   _arrayAppendElement( array, *arraySize, arrayLowContent, 1, arrayLowLen, arraySize ) ;
   _arrayAppendElement( array, *arraySize, arrayMidContent, 1, arrayMidLen, arraySize ) ;
   _arrayAppendElement( array, *arraySize, arrayHighContent, 1, arrayHighLen, arraySize ) ;

done:
   return rc ;
error:
   rc = SDB_SYS ;
   goto done ;
}

static void _hashToKey( CHAR *cipherString, UINT32 cipherStringSize,
                        UINT8 *cipherKey, UINT32 desiredLength )
{
   UINT8 hash[SHA256_DIGEST_LENGTH] ;
   SHA256_CTX sha256 ;

   SHA256_Init( &sha256 ) ;
   SHA256_Update( &sha256, cipherString, cipherStringSize ) ;
   SHA256_Final( hash, &sha256 ) ;

   ossMemcpy( cipherKey, hash, desiredLength < SHA256_DIGEST_LENGTH ?
                               desiredLength : SHA256_DIGEST_LENGTH ) ;
}

static INT32 _decrypt( CHAR *cipherText, const CHAR *token,
                       CHAR *clearText, UINT32 clearTextLen )
{
   INT32                 rc = SDB_OK ;

   CHAR                  randArray[RANDOM_ARRAY_MAX_LENGTH] = {'\0'} ;
   UINT32                randArraySize = 0 ;
   CHAR *                passwdEncrypted = NULL ;
   UINT32                passwdEncryptedSize = 0 ;
   UINT32                passwdLen = 0 ;
   CHAR*                 passwd = NULL ;
   CHAR                  cipherString[CIPHER_STRING_MAX_LENGTH] = {'\0'} ;
   UINT32                cipherStringSize = 0 ;
   DES_cblock            keyEncrypt ; 
   DES_key_schedule      keySchedule ;
   const_DES_cblock      inputText ; 
   DES_cblock            outputText ;
   UINT32                posInClearText = 0 ;
   UINT32                clearTextSize = 0 ;
   UINT32                i = 0 ;
   UINT32                j = 0 ;

   if ( NULL == cipherText )
   {
      rc = SDB_SYS ;
      goto error ;
   }

   passwdEncrypted = ( CHAR * )malloc( strlen( cipherText ) / 2 ) ;

   if ( NULL == passwdEncrypted )
   {
      rc = SDB_OOM ;
      goto error ;
   }

   _hexToByte( cipherText, passwdEncrypted, &passwdEncryptedSize ) ;

   rc = _extractRandomArray( passwdEncrypted, &passwdEncryptedSize,
                             randArray, &randArraySize ) ;
   if ( SDB_OK != rc )
   {
      goto error ;
   }

   if ( NULL != token && 0 != strlen( token ) )
   {
      ossStrncpy( cipherString, token, CIPHER_STRING_MAX_LENGTH - randArraySize  ) ;
      cipherStringSize += strlen( token ) ;
   }
   _arrayAppendElement( cipherString, NULL == token ? 0 : strlen( token ),
                        randArray, 0, randArraySize, &cipherStringSize ) ;

   _hashToKey( cipherString, cipherStringSize,
               ( UINT8 * )&keyEncrypt, KEY_BYTE_LENGTH ) ;

   passwdLen = passwdEncryptedSize ;
   passwd = passwdEncrypted ;

   DES_set_odd_parity( &keyEncrypt ) ;
   DES_set_key_checked( &keyEncrypt, &keySchedule ) ;

   for ( i = 0; i < passwdLen / BYTES_PER_TIME; i++ )
   {  
      ossMemcpy( inputText, passwd + i * BYTES_PER_TIME, BYTES_PER_TIME ) ;
      DES_ecb_encrypt( &inputText, &outputText, &keySchedule, DES_DECRYPT ) ;  

      for ( j = 0; j < BYTES_PER_TIME; j++ )
      {
         if ( ( clearTextLen - 1 ) < ( clearTextSize + 1 ) )
         {
            break ;
         }
         _arrayAppendElement( clearText, clearTextSize,
                              ( CHAR * )&outputText[j], 0, 1, &clearTextSize ) ;
      }
   }

   // remove trailing padding 0s
   posInClearText = clearTextSize - 1 ;
   while ( 0 == clearText[posInClearText] )
   {
      posInClearText-- ;
   }
   if ( ( clearTextSize - 1 ) != posInClearText )
   {
      clearText[posInClearText + 1] = '\0' ;
   }

done:
   if ( NULL != passwdEncrypted )
   {
      free( passwdEncrypted ) ;
   }

   return rc ;
error:
   rc = SDB_SYS ;
   goto done ;
}

void _extractUserInfo( const CHAR *userInfo, CHAR *userName,
                       CHAR *fullName )
{
   CHAR *atPos = ossStrchr( userInfo, '@' ) ;

   if ( NULL != atPos )
   {
      ossStrncpy( userName, userInfo, atPos - userInfo ) ;
   }
   else
   {
      ossStrncpy( userName, userInfo, strlen( userInfo ) ) ;
   }

   ossStrncpy( fullName, userInfo, strlen( userInfo ) ) ;
}

INT32 _readn( INT32 fd, void *vptr, size_t n )
{
   size_t  nleft ;
   INT32   nread ;
   char    *ptr ;

   ptr = vptr ;
   nleft = n ;
   while ( nleft > 0 )
   {
#if defined (_LINUX)
      if ( ( nread = read( fd, ptr, nleft ) ) < 0 )
#elif defined (_WINDOWS)
      if ( ( nread = _read( fd, ptr, nleft ) ) < 0 )
#endif
      {
         if ( errno == EINTR )
            nread = 0 ;      /* and call read() again */
         else
            return -1 ;
      }
      else if ( nread == 0 )
         break ;              /* EOF */

      nleft -= nread ;
      ptr += nread ;
   }
   return ( n - nleft ) ;         /* return >= 0 */
}

INT32 _findAndDecrypt( const CHAR *userName, const CHAR *fullName,
                       const CHAR *token, const CHAR *path, CHAR **clearText )
{
   INT32 rc = SDB_OK ;

   INT32 foundFullNameCount = 0 ;
   INT32 foundHalfNameCount = 0 ;
   CHAR  *atPos = NULL ;
   CHAR  *colonPos = NULL ;
   CHAR  *fileContent = NULL ;
   off_t fileSize = 0 ;
   INT32 fd ;
   UINT32 nleft = fileSize ;
   CHAR   *foundNewline = NULL ;
   CHAR   *startPosition = fileContent ;
   CHAR   *matchedPosition = NULL ;
   UINT32 matchedLength = 0 ;
   UINT32 fileFullNameLen = 0 ;
   UINT32 fileUserNameLen = 0 ;
   UINT32 fullNameLen = strlen( fullName ) ;
   UINT32 userNameLen = strlen( userName ) ;
#if defined (_LINUX)
   struct stat stat  ;
#elif defined (_WINDOWS)
   struct _stat stat ;
#endif

   if ( NULL == userName || NULL == path ||
        NULL == fullName  )
   {
      rc = SDB_INVALIDARG ;
      goto error ;
   }

#if defined (_LINUX)

   fd = open( path, O_RDONLY ) ;
   if ( -1 == fd )
   {
      rc = SDB_FNE ;
      goto error ;
   }

   if ( -1 == fstat( fd, &stat ) )
   {
      rc = errno ;
      goto error ;
   }

#elif defined (_WINDOWS)

   fd = _open( path, O_RDONLY ) ;
   if ( -1 == fd )
   {
      rc = SDB_FNE ;
      goto error ;
   }

   if ( -1 == _fstat( fd, &stat ) )
   {
      rc = errno ;
      goto error ;
   }
#endif

   fileSize = stat.st_size ;
   fileContent = ( CHAR * )malloc( fileSize ) ;
   if ( NULL == fileContent )
   {
      rc = SDB_OOM ;
      goto error ;
   }

   nleft = _readn( fd, ( void * )fileContent, fileSize ) ;
   if ( 0 > nleft )
   {
      rc = SDB_SYS ;
      goto error ;
   }

   startPosition = fileContent ;
   while ( nleft > 0 )
   {
      foundNewline = memchr( startPosition + 1, '\n', nleft ) ;
      if ( NULL != foundNewline )
      {

         colonPos = memchr( startPosition, ':', 
                            foundNewline - startPosition ) ;
         atPos = memchr( startPosition, '@', 
                         foundNewline - startPosition ) ;
         if ( NULL == colonPos )
         {
            rc = SDB_SYS ;
            goto error ;
         }

         fileFullNameLen = colonPos - startPosition ;
         if ( NULL != atPos )
         {
            fileUserNameLen = atPos - startPosition ;
         }

         if ( 0 == memcmp( startPosition, fullName,
                           fileFullNameLen > fullNameLen ?
                           fileFullNameLen : fullNameLen ) )
         {
            foundFullNameCount++ ;
            matchedPosition = colonPos + 1 ;
            matchedLength = foundNewline - matchedPosition ;
            break ;
         }
         else if ( NULL != atPos &&
                   0 == memcmp( startPosition, userName, 
                                fileUserNameLen > userNameLen ?
                                fileUserNameLen : userNameLen ) )
         {
            foundHalfNameCount++ ;
            matchedPosition = colonPos + 1 ;
            matchedLength = foundNewline - matchedPosition ;
         }
      }
      else
      {
         break ;
      }
      
      nleft -= ( foundNewline + 1 ) - startPosition ;
      startPosition = foundNewline + 1 ;      
   }

   if ( 1 == foundFullNameCount || 1 == foundHalfNameCount )
   {
      matchedPosition[matchedLength] = '\0' ;

      *clearText = ( CHAR * )malloc( matchedLength ) ;
      if ( NULL == *clearText )
      {
         rc = SDB_OOM ;
         goto error ;
      }

      _decrypt( matchedPosition, token, *clearText, matchedLength ) ;
   }
   else if ( 1 < foundHalfNameCount )
   {
      printf( "ambiguous user name, try providing cluster name." ) ;
      rc = SDB_INVALIDARG ;
      goto error ;
   }
   else
   {
      printf( "no matching user name." ) ;
      rc = SDB_INVALIDARG ;
      goto error ;
   }

done:

   if ( NULL != fileContent )
   {
      free( fileContent ) ;
   }

   return rc ;
error:
   goto done ;
}

INT32 decryptUserCipher( const CHAR* pUsername, const CHAR *pToken,
                         const CHAR *pPath, CHAR **pPasswd )
{
   INT32   rc = SDB_OK ;

   CHAR    userName[USERNAME_MAX_LENGTH] = {'\0'} ;
   CHAR    fullName[USERNAME_MAX_LENGTH] = {'\0'} ;

   _extractUserInfo( pUsername, userName, fullName ) ;

   rc = _findAndDecrypt( userName, fullName, pToken,
                         pPath, pPasswd ) ;
   if ( SDB_OK != rc )
   {
      printf( "decrypt user %s passwd failed.", fullName ) ;
      rc = SDB_SYS ;
      goto error ;
   }

done:
   return rc ;
error:
   goto done ;
}

