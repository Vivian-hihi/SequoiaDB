#ifndef UTIL_SECLT_MGR_HPP_
#define UTIL_SECLT_MGR_HPP_

#include "utilList.hpp"
#include "utilESClt.hpp"

namespace engine
{
   // Management of search engine client.
   class _utilESCltMgr
   {
   public:
      _utilESCltMgr() ;
      ~_utilESCltMgr() ;

      INT32 init( const std::string &url ) ;
      INT32 getSeClt( utilESClt **seClt ) ;
      INT32 releaseClt( utilESClt **seClt ) ;

   private:
      // Search engine information
      std::string _url ;
      _utilList<utilESClt *> _seCltList ;
   } ;
   typedef _utilESCltMgr utilESCltMgr ;
}

#endif /* UTIL_SECLT_MGR_HPP_ */

