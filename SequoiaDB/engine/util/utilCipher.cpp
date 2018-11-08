#include "ossErr.h"
#include "ossUtil.hpp"
#include "ossIO.hpp"
#include "utilCipher.hpp"
#include "linenoise.h"
#include "openssl/des.h"
#include "openssl/sha.h"
#include <iostream>

namespace engine
{
   cipherMgr::cipherMgr()
   {
   }

   cipherMgr::~cipherMgr()
   {
      _file.close() ;
   }

   INT16 cipherMgr::_hexChar2dec( CHAR c )
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

   void cipherMgr::_hexToByte( const string &hex, string &bytes )
   {
     UINT32 len = hex.length() ;
     const CHAR* base = hex.c_str() ;
     UINT32 i = 0 ;
     UINT32 pos = 0 ;

     for ( ; i < len; i += 2, pos++ )
     {
        const CHAR* c = base + i ;
        bytes.push_back( ( CHAR )( ( _hexChar2dec( c[0] ) << 4 ) |
                                     _hexChar2dec( c[1] ) ) ) ;
     }
   }

   string cipherMgr::_byteToHex( const CHAR* in, INT32 len )
   {
       static const char hexchars[] = "0123456789ABCDEF" ;
       string out;
   
       for ( INT32 i = 0; i < len; ++i )
       {
           CHAR c = in[i] ;
           CHAR high = hexchars[( c & 0xF0 ) >> 4] ;
           CHAR low = hexchars[( c & 0x0F )] ;

           out += high ;
           out += low ;
       }
   
       return out ;
   }

   void cipherMgr::_generateRandomArray( CHAR* array )
   {
       for ( int i = 0; i < RANDOM_ARRAY_BYTE_LENGTH; i++ )
       {
           array[i] = ( CHAR )ossRand() ;
       }
   }

   void cipherMgr::_hashToKey( const string &cipherString,
                               UINT8 *cipherKey, UINT32 desiredLength )
   {
      UINT8 hash[SHA256_DIGEST_LENGTH] ;
      SHA256_CTX sha256 ;

      SHA256_Init( &sha256 ) ;
      SHA256_Update( &sha256, cipherString.c_str(), cipherString.size() ) ;
      SHA256_Final( hash, &sha256 ) ;

      ossMemcpy( cipherKey, hash, desiredLength < SHA256_DIGEST_LENGTH ?
                                  desiredLength : SHA256_DIGEST_LENGTH ) ;
   }

   // generate a random number betweeen begin and end(included)
   INT32 cipherMgr::_randBetween( INT32 begin, INT32 end )
   {
      return begin + ( ossRand() % ( end - begin + 1 ) ) ;
   }

   void cipherMgr::_generateRandArraySplits( UINT32 cipherTextLen,
                                             CHAR *array, INT32 arrayLen,
                                             std::vector<UINT32> &insertPositions,
                                             std::vector<string> &arraySplits )
   {  
      UINT32       cipherLowPos = 0 ;
      UINT32       cipherMidPos  = 0 ;
      UINT32       cipherHighPos = 0 ;
      INT32        cipherEndIndex = cipherTextLen - 1 ;
      UINT32       arrayLowPos = 0 ;
      UINT32       arrayMidPos  = 0 ;
      UINT32       arrayHighPos = 0 ;
      UINT8        arrayLowLen = 0 ;
      UINT8        arrayMidLen  = 0 ;
      UINT8        arrayHighLen = 0 ;
      string       arrayLowContent ;
      string       arrayMidContent ;
      string       arrayHighContent ;
      INT32        arrayEndIndex = arrayLen - 1 ;
      INT32        splitPos1 = 0 ; 
      INT32        splitPos2 = 0 ;

      // calculate insertion position in cipherText, leave space(s) for mid and high.
      cipherLowPos  = _randBetween( 0, cipherEndIndex - 2 ) ;
      cipherMidPos  = _randBetween( cipherLowPos + 1, cipherEndIndex - 1 ) ;
      cipherHighPos = _randBetween( cipherMidPos + 1, cipherEndIndex ) ;

      // divide random array into 3 pieces(low, mid, high) for insertion.
      splitPos1  = _randBetween( 0, arrayEndIndex - 2 ) ;
      splitPos2  = _randBetween( splitPos1 + 1, arrayEndIndex - 1 ) ;
      arrayLowPos = 0 ;
      arrayMidPos  = splitPos1 ;
      arrayHighPos = splitPos2 ;
      arrayLowLen   = splitPos1 - 0 ;
      arrayMidLen   = splitPos2 - splitPos1 ;
      arrayHighLen  = arrayEndIndex - splitPos2 + 1 ;

      //construct insertion content: len + content + offset to next
      arrayLowContent.push_back( ( CHAR )arrayLowLen ) ;
      arrayLowContent.append( array, arrayLowPos, arrayLowLen ) ;
      cipherMidPos += arrayLowContent.size() + 1 ;
      cipherHighPos += arrayLowContent.size() + 1 ;
      arrayLowContent.push_back( ( CHAR )cipherMidPos ) ;

      arrayMidContent.push_back( ( CHAR )arrayMidLen ) ;
      arrayMidContent.append( array, arrayMidPos, arrayMidLen ) ;
      cipherHighPos += arrayMidContent.size() + 1 ;
      arrayMidContent.push_back( ( CHAR )cipherHighPos ) ;

      arrayHighContent.push_back( ( CHAR )arrayHighLen ) ;
      arrayHighContent.append( array, arrayHighPos, arrayHighLen ) ;

      insertPositions.push_back( cipherLowPos ) ;
      insertPositions.push_back( cipherMidPos ) ;
      insertPositions.push_back( cipherHighPos ) ;

      arraySplits.push_back( arrayLowContent ) ;
      arraySplits.push_back( arrayMidContent ) ;
      arraySplits.push_back( arrayHighContent ) ;
   }

   void cipherMgr::_insertRandomArray( string &cipherText, CHAR *array, INT32 arrayLen )
   {
      std::vector<UINT32> insertPositions ;
      std::vector<string> arraySplits ;
      UINT32              totalLen = cipherText.size() > INSERTABLE_MAX_LENGTH ?
                                     INSERTABLE_MAX_LENGTH : cipherText.size() ;
      try
      {
         _generateRandArraySplits( totalLen, array, arrayLen, 
                                   insertPositions, arraySplits ) ;

         cipherText.insert( insertPositions[0], arraySplits[0] ) ;
         cipherText.insert( insertPositions[1], arraySplits[1] ) ;
         cipherText.insert( insertPositions[2], arraySplits[2] ) ;
         cipherText.insert( 0, 1, ( CHAR )insertPositions[0] ) ;
      }
      catch ( exception &e )
      {
         std::cerr << "exception while insert random array: " << e.what() << std::endl ;
      }
   }

   INT32 cipherMgr::_extractRandomArray( string &cipherText, string &array )
   {
      INT32     rc = SDB_OK ;

      UINT8     cipherLowPos = 0 ;
      UINT8     cipherMidPos  = 0 ;
      UINT8     cipherHighPos = 0 ;
      string    arrayLowContent ;
      string    arrayMidContent ;
      string    arrayHighContent ;
      UINT8     arrayLowLen = 0 ;
      UINT8     arrayMidLen  = 0 ;
      UINT8     arrayHighLen = 0 ;

      // calculate random array position in cipherText, then extract
      cipherLowPos = (UINT8)cipherText[0] ;
      if ( cipherText.size() <= cipherLowPos )
      {
         std::cerr << "random array exceeds maximum length." << std::endl ;
         goto error ;
      }
      cipherText.erase( 0, 1 ) ;
      arrayLowLen = cipherText[cipherLowPos] ;
      arrayLowContent = cipherText.substr( cipherLowPos, arrayLowLen + 2 ) ;

      cipherMidPos = (UINT8)arrayLowContent[arrayLowContent.size() - 1] ;
      if ( cipherText.size() <= cipherMidPos )
      {
         std::cerr << "random array exceeds maximum length." << std::endl ;
         goto error ;
      }
      arrayMidLen = cipherText[cipherMidPos] ;
      arrayMidContent = cipherText.substr( cipherMidPos, arrayMidLen + 2 ) ;

      cipherHighPos = (UINT8)arrayMidContent[arrayMidContent.size() - 1] ;
      if ( cipherText.size() <= cipherHighPos )
      {
         std::cerr << "random array exceeds maximum length." << std::endl ;
         goto error ;
      }
      arrayHighLen = cipherText[cipherHighPos] ;
      arrayHighContent = cipherText.substr( cipherHighPos, arrayHighLen + 1 ) ;

      // erase random array from cipherText.
      cipherText.erase( cipherHighPos, arrayHighLen + 1 ) ;
      cipherText.erase( cipherMidPos, arrayMidLen + 2 ) ;
      cipherText.erase( cipherLowPos, arrayLowLen + 2 ) ;

      array.append( arrayLowContent.substr( 1, arrayLowLen ) ) ;
      array.append( arrayMidContent.substr( 1, arrayMidLen ) ) ;
      array.append( arrayHighContent.substr( 1, arrayHighLen ) ) ;

   done:
      return rc ;
   error:
      rc = SDB_SYS ;
      goto done ;
   }

   void cipherMgr::_encrypt( const string &clearText, const string &token,
                             string &cipherText )  
   {
      DES_cblock        keyEncrypt ;
      DES_key_schedule  keySchedule ;
      const_DES_cblock  inputText ;
      DES_cblock        outputText ; 
      string            result ;
      CHAR              randArray[RANDOM_ARRAY_BYTE_LENGTH] = {'\0'} ;
      string            cipherString ;

      _generateRandomArray( randArray ) ;

      if ( !token.empty() )
      {
         cipherString.append( token ) ;
      }
      cipherString.append( randArray, RANDOM_ARRAY_BYTE_LENGTH ) ;
      _hashToKey( cipherString, ( UINT8 * )&keyEncrypt, sizeof( keyEncrypt ) ) ;

      DES_set_odd_parity( &keyEncrypt ) ;
      DES_set_key_checked( &keyEncrypt, &keySchedule ) ;

      for ( UINT32 i = 0; i < clearText.length() / BYTES_PER_TIME; i++ )  
      {  
         ossMemcpy( inputText, clearText.c_str() + i * BYTES_PER_TIME, BYTES_PER_TIME ) ;  
         DES_ecb_encrypt( &inputText, &outputText, &keySchedule, DES_ENCRYPT ) ;  
         result.append( (CHAR *)outputText, BYTES_PER_TIME ) ;
      }

      if ( clearText.length() % BYTES_PER_TIME != 0 )  
      {  
         INT32 remainTextIndex = ( clearText.length() / BYTES_PER_TIME ) *
                                   BYTES_PER_TIME ;
         INT32 remainTextLen = clearText.length() % BYTES_PER_TIME ;

         // padding using 0s
         ossMemset( inputText, 0, BYTES_PER_TIME ) ;  
         ossMemcpy( inputText, clearText.c_str() + remainTextIndex, remainTextLen ) ;  
         DES_ecb_encrypt( &inputText, &outputText, &keySchedule, DES_ENCRYPT ) ;  
         result.append( (CHAR *)outputText, BYTES_PER_TIME ) ;
      }

      _insertRandomArray( result, randArray, RANDOM_ARRAY_BYTE_LENGTH ) ;

      // serialize
      cipherText = _byteToHex( result.c_str(), result.size() ) ;
   }

   INT32 cipherMgr::_decrypt( const string &cipherText, const string &token,
                              string &clearText )
   {
      INT32                 rc = SDB_OK ;

      string                randArray ;
      string                passwdStr ;
      UINT32                passwdLen = 0 ;
      const CHAR*           passwd = NULL ;
      string                cipherString ;
      DES_cblock            keyEncrypt ; 
      DES_key_schedule      keySchedule ;
      const_DES_cblock      inputText ; 
      DES_cblock            outputText ;
      UINT32                posInClearText = 0 ;

      // deserialize
      _hexToByte( cipherText, passwdStr ) ;

      rc = _extractRandomArray( passwdStr, randArray ) ;
      if ( SDB_OK != rc )
      {
         goto error ;
      }

      if ( !token.empty() )
      {
         cipherString.append( token ) ;
      }
      cipherString.append( randArray ) ;
      _hashToKey( cipherString, ( UINT8 * )keyEncrypt, sizeof( keyEncrypt ) ) ;

      passwdLen = passwdStr.size() ;
      passwd = passwdStr.c_str() ;

      DES_set_odd_parity( &keyEncrypt ) ;
      DES_set_key_checked( &keyEncrypt, &keySchedule ) ;

      for ( UINT32 i = 0; i < passwdLen / BYTES_PER_TIME; i++ )  
      {
         ossMemcpy(inputText, passwd + i * BYTES_PER_TIME, BYTES_PER_TIME) ;
         DES_ecb_encrypt( &inputText, &outputText, &keySchedule, DES_DECRYPT ) ;  

         for ( INT32 j = 0; j < BYTES_PER_TIME; j++ )
         {
            clearText += outputText[j] ;
         }
      }

      // remove trailing padded 0s
      posInClearText = clearText.size() - 1 ;
      while ( 0 == clearText[posInClearText] )
      {
         posInClearText-- ;
      }
      if ( ( clearText.size() - 1 ) != posInClearText )
      {
         clearText.erase( posInClearText + 1, clearText.size() - posInClearText + 1 ) ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 cipherMgr::_parseLine( string line, string& usr, string& cipherText )
   {
      INT32 rc = SDB_OK ;

      string::size_type offset = line.find( ":" ) ;
      if ( string::npos == offset || line.length() -1 == offset || 0 == offset )
      {
         PD_LOG ( PDERROR, "line [%s] is in wrong foramt. ", line.c_str() ) ;
         goto error ;
      }

      usr = line.substr( 0, offset ) ;
      cipherText = line.substr( offset + 1, line.length() - offset - 1 ) ;

   done:
      return rc ;
   error:
      rc = SDB_INVALIDARG ;
      goto done ;
   }

   INT32 cipherMgr::_write( const string &fContent )
   {
      INT64 wCnt = 0 ;
      UINT64 len = fContent.length() ;
      INT32 rc = _file.seek( 0 ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG ( PDERROR, "cipher file option [seek] err [%d] ", ossGetLastError() ) ;
         goto error ;
      }

      rc = _file.truncate( 0 ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG ( PDERROR, "cipher file option [truncate] err [%d] ", ossGetLastError() ) ;
         goto error ;
      }

      rc = _file.write( fContent.c_str(), len, wCnt ) ;

   done:
      return rc ;
   error:
      goto done ;
   }

   void cipherMgr::_extractUserInfo( string &userInfo, string &userName,
                                     string &fullName )
   {
      string::size_type atPos = userInfo.find( "@" ) ;

      if ( string::npos != atPos )
      {
         userName = userInfo.substr( 0, atPos ) ;
      }
      else
      {
         userName = userInfo ;
      }
      fullName = userInfo ;

      userInfo = userName ;
   }

   INT32 cipherMgr::_findCipherText( const string &userName, const string &fullName,
                                     string &cipherText )
   {
      INT32                           rc = SDB_OK ;
      
      INT32                           foundFullNameCount = 0 ;
      INT32                           foundHalfNameCount = 0 ;
      map<string, string>::iterator   itor ;
      map<string, string>::iterator   found ;

      for ( itor = _usersCipher.begin(); itor != _usersCipher.end(); itor++ )
      {
         string lineUserName = itor->first ;
         string::size_type atPos = lineUserName.find( '@' ) ;
      
         if ( string::npos != atPos )
         {
            lineUserName = lineUserName.substr( 0, atPos ) ;
         }
      
         if ( itor->first == fullName )
         {
            foundFullNameCount++ ;
            found = itor ;
            break ;
         }
         else if ( lineUserName == userName )
         {
            foundHalfNameCount++ ;
            found = itor ;
         }
      }
      
      if ( 1 == foundFullNameCount || 1 == foundHalfNameCount )
      {
         cipherText = found->second ;
      }
      else if ( 1 < foundHalfNameCount )
      {
         PD_LOG ( PDERROR, "ambiguous user name, try providing cluster name." ) ;
         std::cerr << "ambiguous user name, try providing cluster name." << std::endl ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }
      else
      {
         PD_LOG ( PDWARNING, "no corresponding user information." ) ;
         std::cerr << "no corresponding user information." << std::endl ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 cipherMgr::initialize( const string &fileName, cipherRole role )
   {
      string usr, cipherText ;
      INT64 fileSize = 0 ;
      INT64 contentLen = 0 ;
      INT64 begin = 0 ;
      INT64 lineLen = 0 ;
      UINT32 iMode = ( ( RRole == role ) ? OSS_READONLY : ( OSS_CREATE | OSS_READWRITE ) ) ;
      CHAR* fileContent = NULL ;

      INT32 rc = _file.open( fileName, iMode, OSS_RU | OSS_WU ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG ( PDERROR, "cipher file option [open] err [%d] ", ossGetLastError()) ;
         goto error;
      }

      rc = _file.getFileSize( fileSize ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG ( PDERROR, "cipher file option [getFileSize] err [%d] ", ossGetLastError()) ;
         goto error;
      }

      if ( 0 == fileSize )
      {
         PD_LOG ( PDDEBUG, "cipher file is empty ") ;
         goto done;
      }

      fileContent = ( CHAR* )SDB_OSS_MALLOC( fileSize * sizeof( CHAR ) ) ;
      if ( NULL == fileContent )
      {
         PD_LOG ( PDERROR, "cipher file option [SDB_OSS_MALLOC] err [%d] ", 
                  ossGetLastError() ) ;
         rc = SDB_OOM ; 
         goto error ;
      }

      rc = _file.read( fileContent, fileSize, contentLen ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG ( PDERROR, "cipher file option [read] err [%d] ", ossGetLastError() ) ;
         SDB_OSS_FREE( fileContent ) ;
         goto error ;
      }

      while( begin < contentLen )
      {
         CHAR* find = ossStrstr( fileContent + begin, OSS_NEWLINE ) ;
         if ( NULL != find )
         {
            lineLen = find - ( fileContent + begin ) ;
            string line( fileContent + begin, lineLen ) ;
            begin += lineLen + ossStrlen( OSS_NEWLINE ) ;

            if ( SDB_OK == _parseLine( line, usr, cipherText ) )
            {
               _usersCipher[usr] = cipherText ;
            }
         }
         else
         {
            PD_LOG ( PDWARNING, "cipher file is not complete") ;
            break ;
         }  
      }

   done:
      if ( NULL != fileContent )
      {
         SDB_OSS_FREE(fileContent) ;
      }
      
      return rc ;
   error:
      goto done ;
   }

   INT32 cipherMgr::addUser( const string &user, const string &token,
                             const string &passwd )
   {
      INT32 rc = SDB_OK ;

      string cipherText ;
      string fileContent ;
      map<string,string>::iterator it ;

      _encrypt( passwd, token, cipherText ) ;

      _usersCipher[user] = cipherText ;

      it = _usersCipher.begin() ;

      for ( ; it != _usersCipher.end(); it++ )
      {
         fileContent += ( it->first + ":" + it->second + OSS_NEWLINE ) ;
      }
      rc = _write( fileContent ) ;

      return rc ;
   }

   INT32 cipherMgr::removeUser( const string &user )
   {
      INT32 rc = SDB_OK ;

      map<string, string>::iterator it = _usersCipher.find( user ) ;
      if ( it != _usersCipher.end() )
      {
         string fileContent ;

         _usersCipher.erase( it ) ;

         for( it = _usersCipher.begin(); it != _usersCipher.end(); it++ )
         {
            fileContent += ( user + ":" + it->second + OSS_NEWLINE ) ;
         }

         rc = _write( fileContent ) ;
      }

      return rc ;
   }



   INT32 cipherMgr::getPasswd( string &userInfo, const string &token,
                               string &passwd )
   {
      INT32 rc = SDB_OK ;

      string userName ;
      string fullName ;
      string cipherText ;

      _extractUserInfo( userInfo, userName, fullName ) ;

      rc = _findCipherText( userName, fullName, cipherText ) ;
      if ( SDB_OK != rc )
      {
         goto error ;
      }

      rc = _decrypt( cipherText, token, passwd ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG ( PDERROR, "decrypt user %s passwd failed.",
                  fullName.c_str() ) ;
         rc = SDB_SYS ;
         goto error ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   string passwordTool::interactivePasswdInput()
   {
      CHAR* line = NULL ;
      string passwd ;
      setEchoOff() ;
      if ( ( line = linenoise( "password:" ) ) != NULL )
      {
         passwd = line ;
         SDB_OSS_ORIGINAL_FREE( line ) ;
      }
      setEchoOn() ;
      return passwd ;
   }

   INT32 passwordTool::getPasswdByCipherFile( string &user,
                                              const string &token,
                                              const string &cipherFile,
                                              string &password )
   {
      string file = cipherFile ;
      if ( file.empty() )
      {
         file = "./passwd" ;
      }

      _cipherMgr.initialize( file, cipherMgr::RRole ) ;

      return _cipherMgr.getPasswd( user, token, password ) ;
   }

}
