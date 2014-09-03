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
#include "ossMem.h"
#include "omagent.hpp"
#include "omagentCommand.hpp"
#include "omagentMsgDef.hpp"
#include "sptScope.hpp"
#include <map>
#include <string>

using namespace std ;
using namespace bson ;

namespace engine
{
   class _omaJobRunCmd : public SDBObject
   {
      public:
         _omaJobRunCmd () ;
         virtual ~_omaJobRunCmd () ;
         virtual INT32 setJSFile ( const CHAR *fileName ) ;
 
      public:
         virtual const CHAR* name () = 0 ;
         virtual INT32 init ( BSONObj &installInfo ) = 0 ;
         virtual INT32 doit ( BSONObj &result ) = 0 ;

     protected:
         _sptScope *_scope ;
         CHAR _jsFileName[ OSS_MAX_PATHSIZE + 1 ] ;
         CHAR _jsFileArgs[ JS_ARG_LEN + 1 ] ;
         CHAR *_fileBuff ;
         UINT32 _buffSize ;
         UINT32 _readSize ;
         std::string _content ;
   } ;

   class _omaJobRunInstallCatalogCmd : public _omaCommand
   {
      public:
         _omaJobRunInstallCatalogCmd ( InstallInfo &info ) ;
         virtual ~_omaJobRunInstallCatalogCmd () ;

      public:
         virtual const CHAR* name () { return "" ; }
         virtual INT32 init ( const CHAR *pInstallInfo ) ;
         virtual INT32 doit ( BSONObj &retObj ) ;

      private:
         BSONObj _installInfo ;
         InstallInfo _info ;

   } ;

   class _omaJobRunInstallCoordCmd : public _omaCommand
   {
      public:
         _omaJobRunInstallCoordCmd ( InstallInfo &info ) ;
         virtual ~_omaJobRunInstallCoordCmd () ;

      public:
         virtual const CHAR* name () { return "" ; }
         virtual INT32 init ( const CHAR *pInstallInfo ) ;
         virtual INT32 doit ( BSONObj &retObj ) ;

      public:
         BSONObj _installInfo ;
         InstallInfo _info ;

   } ;

   class _omaJobRunInstallDataCmd : public _omaCommand
   {
      public:
         _omaJobRunInstallDataCmd ( InsallInfo &info ) ;
         virtual ~_omaJobRunInstallDataCmd () ;

      public:
         virtual const CHAR* name () { return "" ; }
         virtual INT32 init ( const CHAR *pInstallInfo ) ;
         virtual INT32 doit ( BSONObj &retObj ) ;

      public:
         BSONObj _installInfo ;
         InstallInfo _info ;

   } ;


}


#endif
