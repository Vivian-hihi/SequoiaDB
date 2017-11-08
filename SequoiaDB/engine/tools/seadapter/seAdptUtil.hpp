/*******************************************************************************


   Copyright (C) 2011-2017 SequoiaDB Ltd.

   This program is free software: you can redistribute it and/or modify
   it under the term of the GNU Affero General Public License, version 3,
   as published by the Free Software Foundation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warrenty of
   MARCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU Affero General Public License for more details.

   You should have received a copy of the GNU Affero General Public License
   along with this program. If not, see <http://www.gnu.org/license/>.

   Source File Name = seAdptUtil.hpp

   Descriptive Name = Search Engine Adapter Util.

   When/how to use: this program may be used on binary and text-formatted
   versions of PMD component. This file contains main function for sdbcm,
   which is used to do cluster managing.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          10/01/2017  YSD  Initial Draft

   Last Changed =

*******************************************************************************/
#ifndef SEADPT_UTIL_HPP__
#define SEADPT_UTIL_HPP__

#include "core.hpp"
#include "seAdptDef.hpp"
#include <string>

namespace engine
{
   // Parse the original collection and index name into target index name.
   class _seAdptNameParser
   {
      public:
         _seAdptNameParser() ;
         ~_seAdptNameParser() ;

         INT32 parse( const CHAR *clFullName, const CHAR *idxName ) ;
         void reset() ;
         const CHAR* getTargetIdxName() ;

      private:
         std::string _targetIdxName ;
   } ;
   typedef _seAdptNameParser seAdptNameParser ;
}

#endif /* SEADPT_UTIL_HPP__ */

