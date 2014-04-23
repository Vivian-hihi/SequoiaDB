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

   Source File Name = utilRemoteExec.hpp

   Descriptive Name = Remote Excuting Header

   When/how to use: this program may be used on binary and text-formatted
   versions of OSS component. This file contains declares for process op.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          2/27/2013  TW  Initial Draft

   Last Changed =

*******************************************************************************/
#ifndef UTILREMOTEEXEC_HPP__
#define UTILREMOTEEXEC_HPP__

#include "core.hpp"
#include "../bson/bson.h"

INT32 utilRemoteExec ( SINT32 remoCode,
                      const CHAR * hostname,
                      SINT32 *retCode,
                      bson::BSONObj *arg1 = NULL,
                      bson::BSONObj *arg2 = NULL,
                      bson::BSONObj *arg3 = NULL,
                      bson::BSONObj *arg4 = NULL ) ;

#endif /* UTILREMOTEEXEC_HPP__ */