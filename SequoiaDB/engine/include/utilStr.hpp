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

   Source File Name = utilStr.hpp

   Descriptive Name =

   When/how to use: str util

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          09/14/2012  YW  Initial Draft

   Last Changed =

******************************************************************************/

#ifndef UTILSTR_HPP_
#define UTILSTR_HPP_

#include "core.hpp"

namespace engine
{
   /// skip spaces at begin.
   INT32 utilStrTrimBegin( const CHAR *src, const CHAR *&begin ) ;

   /// remove spaces at end.
   INT32 utilStrTrimEnd( CHAR *src ) ;

   /// trim spaces at begin or end .
   INT32 utilStrTrim( CHAR *src, const CHAR *&begin ) ;

   INT32 utilStrToUpper( const CHAR *src, CHAR *&upper ) ;

   INT32 utilStrJoin( const CHAR **src,
                      UINT32 cnt,
                      CHAR *join,
                      UINT32 &joinSize ) ;

   INT32 utilStr2TimeT( const CHAR *str,
                        time_t &tm,
                        UINT64 *usec = NULL ) ;

   INT32 utilBuildFullPath( const CHAR *path, const CHAR *name,
                            UINT32 fullSize, CHAR *full ) ;

   INT32 utilCatPath( CHAR *src, UINT32 srcSize, const CHAR *catStr ) ;

   const CHAR* utilAscTime( time_t tTime, CHAR *pBuff, UINT32 size ) ;

}

#endif // UTILSTR_HPP_

