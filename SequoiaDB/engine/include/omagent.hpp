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

using namespace std ;
using namespace bson ;

#define JS_FILE_NAME_LEN                 (512)
#define JS_ARG_LEN                       (1024)

#define ROLE_COORD                       "coord"
#define ROLE_CATA                        "catalog"
#define ROLE_DATA                        "data"
#define ROLE_STANDALONE                  "standalone"


namespace engine
{

   struct _InstallInfo
   {
      string _hostName ;
      string _svcName ;
      string _dbPath ;
      string _confPath ;
      string _dataGroupName ;
      BSONObj _conf ;
   } ;
   typedef struct _InstallInfo InstallInfo ;

   struct _InstalledNode
   {
      string _role ;
      string _dataGroupName ;
      string _hostName ;
      string _svcName ;
   } ;
   typedef struct _InstalledNode InstalledNode ;

   struct _InstallResult
   {
      INT32 _rc ;
      INT32 _totalNum ;
      INT32 _finishNum ;
      string _errMsg ;
      string _desc ;
      vector< InstalledNode > _installedNodes ;
   } ;
   typedef struct _InstallResult InstallResult ;

   struct _AddHost
   {
      std::string _ip ;
      std::string _userName ;
      std::string _passwd ;
      std::string _installPath ;
   } ;
   typedef struct _AddHost AddHost ;

   enum OMA_JOB_STATUS
   {
      OMA_JOB_STATUS_INIT         = 1 ,
      OMA_JOB_STATUS_RUNNING      = 2 ,
      OMA_JOB_STATUS_FINISH       = 3 ,
      OMA_JOB_STATUS_FAIL         = 4 ,

      OMA_JOB_STATUS_END          = 10
   } ;


}





#endif // OMAGENT_HPP_
