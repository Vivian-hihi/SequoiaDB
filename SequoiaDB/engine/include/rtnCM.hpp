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

   Source File Name = rtnCM.cpp

   Descriptive Name = rtnClusterManager

   When/how to use: this program may be used on binary and text-formatted
   versions of PMD component. This file contains main function for sdbcm,
   which is used to do cluster managing.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          12/17/2013  JHL  Initial Draft

   Last Changed =

*******************************************************************************/
#ifndef RTNCM_HPP__
#define RTNCM_HPP__

#include "pmdOptions.h"
#include "pmdDaemon.hpp"
#include "omagentDef.hpp"

namespace CLSMGR
{

#define SDBCM_OPTION_PREFIX         "--"

// remote operation code
#define SDBSTART              1
#define SDBSTOP               2
#define SDBADD                3
#define SDBMODIFY             4
#define SDBRM                 5

// stop status
#define STOPFAIL              1
#define STOPPART              3

   /*
      cCMService define
   */
   class cCMService : public engine::iPmdDMNChildProc
   {
   public:
      cCMService() ;
      ~cCMService(){}
      INT32 init() ;

      virtual const CHAR *getProgramName() ;

   private:
      const CHAR *getArguments();
      virtual INT32 svcMain( INT32 argc, CHAR **argv );
   } ;

}

#endif // RTNCM_HPP__

