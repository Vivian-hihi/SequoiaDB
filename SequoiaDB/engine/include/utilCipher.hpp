#ifndef UTILCIPHER_H_
#define UTILCIPHER_H_

#include "ossFile.hpp"
#include "ossTypes.h"
#include <string>
#include <map>
#include <vector>

namespace engine
{
   class cipherMgr : public SDBObject
   {
   public:
      enum cipherRole
      {
         RRole,
         WRole
      } ;
      static const INT32  BYTES_PER_TIME = 8 ;
      static const INT32  KEY_BYTE_LENGTH = 8 ;
      static const INT32  RANDOM_ARRAY_BYTE_LENGTH = 16 ;
      static const UINT32 INSERTABLE_MAX_LENGTH = 234 ;

      cipherMgr() ;
      ~cipherMgr() ;

      INT32 initialize( const std::string &fileName, cipherRole role ) ;
      INT32 addUser( const std::string &user, const std::string &token,
                     const std::string &passwd ) ;
      INT32 removeUser( const std::string &user ) ;
      INT32 getPasswd( string &userInfo, const string &token,
                       string &passwd ) ;

   private:
      void   _encrypt( const std::string &clearText, const std::string &token,
                       std::string &cipherText ) ;
      INT32  _decrypt( const std::string &cipherText, const std::string &token,
                       std::string &clearText ) ;

      void   _hashToKey( const std::string &cipherString,
                         UINT8 *cipherKey, UINT32 desiredLength ) ;
      INT16  _hexChar2dec( CHAR c ) ;
      void   _hexToByte( const std::string &hex, std::string &bytes ) ;
      std::string _byteToHex( const CHAR* in, INT32 len ) ;

      INT32  _randBetween( INT32 begin, INT32 end ) ;
      void   _generateRandomArray( CHAR* array ) ;
      void   _generateRandomArraySplits( UINT32 cipherTextLen,
                                         CHAR *array, INT32 arrayLen,
                                         std::vector<UINT32> &insertPositions,
                                         std::vector<string> &arraySplits );
      void   _insertRandomArray( string &cipherText, CHAR *array, INT32 arrayLen ) ;
      INT32  _extractRandomArray( std::string &cipherText, std::string &array ) ;

      INT32  _parseLine( std::string line, std::string& usr, std::string& cipherText ) ;
      INT32  _write( const std::string& fileContent ) ;
      void   _extractUserInfo( string &userInfo, string &userName,
                               string &fullName ) ;
      INT32  _findCipherText( const string &userName, const string &fullName,
                              string &cipherText ) ;

   private:
      ossFile _file;        
      std::map<std::string, std::string> _usersCipher;
   } ;

   class passwordTool : public SDBObject
   {
   public:
      passwordTool() {}
      ~passwordTool() {}
      static std::string interactivePasswdInput() ;
      INT32              getPasswdByCipherFile( string &user,
                                                const string &token,
                                                const string &cipherFile,
                                                string &password ) ;
   private:
      cipherMgr   _cipherMgr ;
   } ;
}

#endif // UTIL_CIPHER_HPP_