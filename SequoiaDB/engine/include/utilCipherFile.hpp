#ifndef UTILCIPHERFILE_H_
#define UTILCIPHERFILE_H_

#include "ossFile.hpp"

namespace engine
{

   class cipherAbstractFile : public SDBObject
   {
   public:
      enum cipherRole
      {
         RRole,
         WRole
      } ;

      cipherAbstractFile() {}
      virtual ~cipherAbstractFile() {}

      virtual INT32 initFile( const std::string &fileName, 
                              cipherRole role) = 0 ;
      virtual INT32 readFromFile( const CHAR **fileContent,
                                  INT64 *contentLen ) = 0 ;
      virtual INT32 writeToFile( const std::string& fileContent ) = 0 ;
   } ;


   class cipherFile : public cipherAbstractFile
   {
   public:
      cipherFile() : _fileContent( NULL ) {}
      ~cipherFile() ;

      INT32 initFile( const std::string &fileName, 
                      cipherRole role) ;
      INT32 readFromFile( const CHAR **fileContent,
                          INT64 *contentLen ) ;
      INT32 writeToFile( const std::string& fileContent ) ;
   private:
      ossFile  _file ;
      CHAR    *_fileContent ;
   } ;

}

#endif