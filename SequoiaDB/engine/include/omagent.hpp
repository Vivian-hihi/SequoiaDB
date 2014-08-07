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

   Source File Name = omagent.hpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          06/30/2014  TZB Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef OMAGENT_HPP_
#define OMAGENT_HPP_

#include "core.hpp"
#include "../bson/bson.h"
#include "ossUtil.hpp"
#include "sptApi.hpp"
#include "omagentMsgDef.hpp"

using namespace bson ;

#define JS_ARG_LEN 512

namespace engine
{

   struct _InstallInfo
   {
      const CHAR *_hostName ;
      const CHAR *_svcName ;
      const CHAR *_dbPath ;
      const CHAR *_confPath ;
      const CHAR *_dataGroupName ;
      BSONObj _conf ;
   } ;
   typedef struct _InstallInfo InstallInfo ;

   struct _InstallJobResult
   {
      INT32 _rc ;
      std::string _errMsg ;
      INT32 _totalNum ;
      INT32 _finishNum ;
      std::vector< InstallInfo > _finishNode ;
   } ;
   typedef struct _InstallJobResult InstallJobResult ;

}





#endif // OMAGENT_HPP_
