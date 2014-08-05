#ifndef OMAGENT_JOB_RUN_COMMAND_HPP_
#define OMAGENT_JOB_RUN_COMMAND_HPP_

#include "core.hpp"
#include "oss.hpp"
#include "ossTypes.h"
#include "ossUtil.h"
#include "../bson/bson.h"
#include "sptApi.hpp"
#include "ossMem.h"
#include "omagent.hpp"
#include "omagentMsgDef.hpp"
//#include "omagentJob.hpp"
#include "omagent.hpp"
#include <map>
#include <string>

using namespace bson ;

namespace engine
{
   class _omaJobRunCmd : public SDBObject
   {
      public:
         _omaJobRunCmd () {}
         virtual ~_omaJobRunCmd () {}
      public:
         virtual INT32 init ( std::vector<BSONObj> &objs ) = 0 ;
         virtual INT32 doit ( InstallJobResult &result ) = 0 ;

     protected:
         _sptScope *_scope ;
         const CHAR *_jsFileName ;
         CHAR *_fileBuff ;
         UINT32 _buffSize ;
         UINT32 _readSize ;
//         std::vector<BSONObj> &_hosts ;
         std::string _content ;
   } ;

   class _omaJobRunInstallCatalogCmd : public _omaJobRunCmd
   {
      public:
         _omaJobRunInstallCatalogCmd () ;
         virtual ~_omaJobRunInstallCatalogCmd () ;

      public:
         virtual INT32 init ( std::vector<BSONObj> &objs ) ;
         virtual INT32 doit ( InstallJobResult &result ) ;

      private:
         std::vector<InstallInfo> _installInfos ;
   } ;

   class _omaJobRunInstallCoordCmd : public _omaJobRunCmd
   {
      public:
         _omaJobRunInstallCoordCmd () ;
         virtual ~_omaJobRunInstallCoordCmd () ;

      public:
         virtual INT32 init ( std::vector<BSONObj> &objs ) ;
         virtual INT32 doit ( InstallJobResult &result ) ;

      private:
         std::vector<InstallInfo> _installInfos ;
   } ;

   class _omaJobRunInstallDataCmd : public _omaJobRunCmd
   {
      public:
         _omaJobRunInstallDataCmd () ;
         virtual ~_omaJobRunInstallDataCmd () ;

      public:
         virtual INT32 init ( std::vector<BSONObj> &objs ) ;
         virtual INT32 doit ( InstallJobResult &result ) ;

      private:
         std::vector<InstallInfo> _installInfos ;
   } ;


}


#endif
