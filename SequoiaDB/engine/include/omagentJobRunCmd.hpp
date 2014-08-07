/*******************************************************************************


   Copyright (C) 2011-2014 SequoiaDB Ltd.

   This program is free software: you can redistribute it and/or modify
   it under the term of the GNU Affero General Public License, version 3,
   as published by the Free Software Foundation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warrenty of
   MARCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU Affero General Public License for more details.

   You should have received a copy of the GNU Affero General Public License
   along with this program. If not, see <http://www.gnu.org/license/>.

   Source File Name = omagentJobRunCmd.hpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          06/30/2014  TZB Initial Draft

   Last Changed =

*******************************************************************************/

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
#include "omagent.hpp"
#include <map>
#include <string>

using namespace bson ;

namespace engine
{
   class _omaJobRunCmd : public SDBObject
   {
      public:
         _omaJobRunCmd () { _scope = NULL ; }
         virtual ~_omaJobRunCmd ()
         {
            if ( _scope )
            {
              _scope->shutdown() ;
              SAFE_OSS_DELETE ( _scope ) ;
            }
         }
      public:
         virtual INT32 init ( std::vector<BSONObj> &objs ) = 0 ;
         virtual INT32 doit ( InstallJobResult &result ) = 0 ;

     protected:
         _sptScope *_scope ;
         const CHAR *_jsFileName ;
         CHAR *_fileBuff ;
         UINT32 _buffSize ;
         UINT32 _readSize ;
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
