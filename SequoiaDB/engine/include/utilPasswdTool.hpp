#ifndef UTILPASSWDTOOL_H_
#define UTILPASSWDTOOL_H_

#include "utilCipherMgr.hpp"

namespace engine
{

   class passwordTool : public SDBObject
   {
   public:
      passwordTool() {}
      ~passwordTool() {}
      static std::string interactivePasswdInput() ;
      INT32              getPasswdByCipherFile( const std::string &user,
                                                const std::string &token,
                                                const std::string &cipherFile,
                                                std::string &connectionUser,
                                                std::string &password ) ;
   private:
      cipherMgr    _cipherMgr ;
      cipherFile   _cipherfile ;
   } ;

}

#endif