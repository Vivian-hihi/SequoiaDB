#include "ossMem.h"
#include "ossUtil.h"
#include "openssl/des.h"
#include "openssl/sha.h"


#define BYTES_PER_TIME               8
#define KEY_BYTE_LENGTH              8

#define PASSWD_MAX_LENGTH            256
#define RANDOM_ARRAY_MAX_LENGTH      16
#define PASSWD_ENCRYPTED_MAX_LENGTH  PASSWD_MAX_LENGTH + RANDOM_ARRAY_MAX_LENGTH
#define ARRAY_CONTENT_LENGTH         RANDOM_ARRAY_MAX_LENGTH
#define CIPHERTEXT_MAX_LENGTH        512
#define USERNAME_MAX_LENGTH          256

#define CIPHER_STRING_MAX_LENGTH     256
#define CIPHERFILE_LINE_MAX_LENGTH   256



static INT16 _hexChar2dec( CHAR c )
{
   if( '0' <= c && c <= '9' )
   {
	  return (INT16)( c - '0' ) ;
   }
   else if ( 'a' <= c && c <= 'f' )
   {
	  return (INT16)( ( c - 'a' ) + 10 ) ;
   }
   else if( 'A' <= c && c <= 'F' )
   {
	  return (INT16)( ( c - 'A' ) + 10 ) ;
   }
   return -1 ;
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
   cipherLowPos = cipherText[0] ;
   if ( *cipherTextSize <= cipherLowPos )
   {
      goto error ;
   }
   _arrayRemoveElement( cipherText, 0, 1, cipherTextSize ) ;

   arrayLowLen = cipherText[cipherLowPos] ;
   ossMemcpy( arrayLowContent, cipherText + cipherLowPos, arrayLowLen + 2 ) ;
   arrayLowContentSize = arrayLowLen + 2 ;

   cipherMidPos = arrayLowContent[arrayLowContentSize - 1] ;
   if ( *cipherTextSize <= cipherMidPos )
   {
      goto error ;
   }
   arrayMidLen = cipherText[cipherMidPos] ;
   ossMemcpy( arrayMidContent, cipherText + cipherMidPos, arrayMidLen + 2 ) ;
   arrayMidContentSize = arrayMidLen + 2 ;

   cipherHighPos = arrayMidContent[arrayMidContentSize - 1] ;
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

static INT32 _decrypt( const CHAR *cipherText, const CHAR *token,
                       CHAR *clearText, UINT32 clearTextLen )
{
   INT32                 rc = SDB_OK ;

   CHAR                  randArray[RANDOM_ARRAY_MAX_LENGTH] = {'\0'} ;
   UINT32                randArraySize = 0 ;
   CHAR                  passwdEncrypted[PASSWD_ENCRYPTED_MAX_LENGTH] = {'\0'} ;
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

   if ( NULL == cipherText ||
        2 * PASSWD_ENCRYPTED_MAX_LENGTH < strlen( cipherText ) )
   {
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
   //ossStrncpy( userInfo, userName, strlen( userName ) ) ;
}

INT32 _findCipherText( const CHAR *userName, const CHAR *fullName,
                       const CHAR *path, CHAR *cipherText )
{
   INT32 rc = SDB_OK ;

   INT32 foundFullNameCount = 0 ;
   INT32 foundHalfNameCount = 0 ;
   FILE  *fp = NULL ;
   CHAR  line[CIPHERFILE_LINE_MAX_LENGTH] = { '\0' } ;
   CHAR  matchedUserInfo[CIPHERFILE_LINE_MAX_LENGTH] = { '\0' } ;
   CHAR  *atPos = NULL ;
   CHAR  *colonPos = NULL ;

   if ( NULL == userName || NULL == path ||
        NULL == fullName || NULL == cipherText )
   {
      rc = SDB_INVALIDARG ;
      goto error ;
   }

   fp = fopen( path, "r" ) ;
   if ( NULL == fp )
   {
      rc = SDB_FNE ;
      goto error ;
   }

   while ( NULL != fgets( line, CIPHERFILE_LINE_MAX_LENGTH, fp ) )
   {
      colonPos = ossStrchr( line, ':' ) ;
      atPos = ossStrchr( line, '@' ) ;
      if ( NULL == colonPos )
      {
         rc = SDB_SYS ;
         goto error ;
      }

      if ( 0 == ossStrncmp( line, fullName, colonPos - line ) )
      {
         foundFullNameCount++ ;
         ossMemset( matchedUserInfo, 0, CIPHERFILE_LINE_MAX_LENGTH ) ;
         ossStrncpy( matchedUserInfo, line, CIPHERFILE_LINE_MAX_LENGTH ) ;
         break ;
      }
      else if ( NULL != atPos &&
                0 == ossStrncmp( line, userName, atPos - line ) )
      {
         foundHalfNameCount++ ;
         ossMemset( matchedUserInfo, 0, CIPHERFILE_LINE_MAX_LENGTH ) ;
         ossStrncpy( matchedUserInfo, line, CIPHERFILE_LINE_MAX_LENGTH ) ;
      }
   }

   if ( 1 == foundFullNameCount || 1 == foundHalfNameCount )
   {
      ossStrncpy( cipherText, ossStrstr( matchedUserInfo, ":" ) + 1,
                  CIPHERTEXT_MAX_LENGTH ) ;
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
   if ( NULL != fp )
   {
      fclose( fp ) ;
   }

   return rc ;
error:
   goto done ;
}

INT32 decryptUserCipher( const CHAR* pUsername, const CHAR *pToken,
                         const CHAR *pPath, CHAR *pPasswd, UINT32 passwdLen )
{
   INT32   rc = SDB_OK ;

   CHAR    userName[USERNAME_MAX_LENGTH] = {'\0'} ;
   CHAR    fullName[USERNAME_MAX_LENGTH] = {'\0'} ;
   CHAR    cipherText[CIPHERTEXT_MAX_LENGTH] = {'\0'} ;

   _extractUserInfo( pUsername, userName, fullName ) ;

   rc = _findCipherText( userName, fullName, pPath, cipherText ) ;
   if ( SDB_OK != rc )
   {
      goto error ;
   }

   rc = _decrypt( cipherText, pToken, pPasswd, passwdLen ) ;
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

