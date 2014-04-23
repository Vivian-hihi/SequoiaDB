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

   Source File Name = pmdSignalHandler.hpp

   Descriptive Name = Process MoDel Signal Handler Header

   When/how to use: this program may be used on binary and text-formatted
   versions of PMD component. This file contains function used for signal
   handler.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          09/14/2012  TW  Initial Draft

   Last Changed =

*******************************************************************************/
#ifndef PMDSIGNALHANDLER_HPP_
#define PMDSIGNALHANDLER_HPP_
#include "core.hpp"
#include "ossSignal.hpp"
#include "ossStackDump.hpp"
#include "ossPrimitiveFileOp.hpp"
namespace engine
{
   void ossSignalSigsegv ( OSS_HANDPARMS ) ;
}

#endif
