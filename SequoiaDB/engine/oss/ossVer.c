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

   Source File Name = ossVer.cpp

   Descriptive Name = Operating System Services Version

   When/how to use: this program may be used on binary and text-formatted
   versions of OSS component. This file contains engine version information

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          01/16/2013  TW  Initial Draft

   Last Changed =

*******************************************************************************/
#include "ossVer.h"
#include <iostream>
#include <sstream>

void ossGetVersion ( INT32 *version,
                     INT32 *subVersion,
                     INT32 *fixVersion,
                     INT32 *release,
                     const CHAR **ppBuild )
{
   if ( version )
      *version = SDB_ENGINE_VERISON_CURRENT ;
   if ( subVersion )
      *subVersion = SDB_ENGINE_SUBVERSION_CURRENT ;
   if ( release )
      *release = SDB_ENGINE_RELEASE_CURRENT ;
   if ( fixVersion )
   {
#ifdef SDB_ENGINE_FIXVERSION_CURRENT
      *fixVersion = SDB_ENGINE_FIXVERSION_CURRENT ;
#else
      *fixVersion = 0 ;
#endif // SDB_ENGINE_FIXVERSION_CURRENT
   }
   if ( ppBuild )
      *ppBuild = SDB_ENGINE_BUILD_TIME ;
}

std::string ossSprintVersion( const CHAR *prompt )
{
   std::stringstream ss ;
#ifdef SDB_ENGINE_FIXVERSION_CURRENT
   ss << prompt << ": " << SDB_ENGINE_VERISON_CURRENT << "."
      << SDB_ENGINE_SUBVERSION_CURRENT << "."
      << SDB_ENGINE_FIXVERSION_CURRENT << std::endl ;
#else
   ss << prompt << ": " << SDB_ENGINE_VERISON_CURRENT << "."
      << SDB_ENGINE_SUBVERSION_CURRENT << std::endl ;
#endif //SDB_ENGINE_FIXVERSION_CURRENT
   ss << "Release: " << SDB_ENGINE_RELEASE_CURRENT << std::endl ;
   ss << SDB_ENGINE_BUILD_TIME << std::endl ;

   return ss.str() ;
}

void ossPrintVersion( const CHAR *prompt )
{
   std::cout << ossSprintVersion( prompt ) ;
}

