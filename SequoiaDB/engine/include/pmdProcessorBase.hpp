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

   Source File Name = pmdProcessorBase.hpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          01/12/2014  Lin Youbin  Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef PMD_PROCESSORBASE_HPP_
#define PMD_PROCESSORBASE_HPP_

#include "oss.hpp"
#include "msg.h"
#include "sdbInterface.hpp"
#include "rtnContext.hpp"

namespace engine
{
   class _IProcessor : public SDBObject
   {
      public:
         _IProcessor() {}
         virtual ~_IProcessor() {}

      public:

         virtual INT32           processMsg( MsgHeader *msg, 
                                             SDB_DPSCB *dpsCB,
                                             rtnContextBuf &contextBuff, 
                                             INT64 &contextID, 
                                             INT32 &startPos ) = 0 ;

         virtual const CHAR *    getName() = 0 ;
   } ;
}

#endif /*PMD_PROCESSORBASE_HPP_*/

