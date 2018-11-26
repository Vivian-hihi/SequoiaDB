#ifndef UTILCIPHERMGR_H_
#define UTILCIPHERMGR_H_

#include "utilCipherFile.hpp"
#include "ossTypes.hpp"
#include <string>
#include <map>
#include <vector>

namespace engine
{

   class cipherMgr : public SDBObject
   {
   public:
      static const INT32  BYTES_PER_TIME = 8 ;
      static const INT32  KEY_BYTE_LENGTH = 8 ;
      static const INT32  RANDOM_ARRAY_BYTE_LENGTH = 16 ;
      static const INT32  UINT8_MAX_NUMBER = 65536 ;
      static const UINT32 INSERTABLE_MAX_LENGTH = 234 ;

      cipherMgr() {}
      ~cipherMgr() {}

      INT32 init( cipherAbstractFile *file ) ;
      INT32 addUser( const std::string &user, const std::string &token,
                     const std::string &passwd ) ;
      INT32 removeUser( const std::string &user ) ;
      INT32 getPasswd( const std::string &userInfo, const std::string &token,
                       std::string &passwd ) ;
      void  getConnectionUserName( const std::string &userInfo,
                                   std::string &connectionUserName ) ;

   private:
      INT32  _encrypt( const std::string &clearText, const std::string &token,
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
      void   _generateRandArraySplits( UINT32 cipherTextLen,
                                       CHAR *array, INT32 arrayLen,
                                       std::vector<UINT32> &insertPositions,
                                       std::vector<std::string> &arraySplits ) ;
      void   _insertRandomArray( std::string &cipherText, CHAR *array, INT32 arrayLen ) ;
      INT32  _extractRandomArray( std::string &cipherText, std::string &array ) ;

      INT32  _parseLine( std::string line, std::string& usr, std::string& cipherText ) ;
      INT32  _write( const std::string& fileContent ) ;
      void   _extractUserInfo( const std::string &userInfo, std::string &userName,
                               std::string &fullName ) ;
      INT32  _findCipherText( const std::string &userName, const std::string &fullName,
                              std::string &cipherText ) ;

   private:
      cipherAbstractFile *_cipherfile ;
      std::map<std::string, std::string> _usersCipher ;
   } ;


}

#endif // UTIL_CIPHER_HPP_