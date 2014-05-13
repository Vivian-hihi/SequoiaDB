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

   Source File Name = util2Bsonobj.cpp

   Descriptive Name =

   When/how to use: this program may be used on binary and text-formatted
   versions of PMD component. This file contains functions for agent processing.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          09/14/2012  YW  Initial Draft

   Last Changed =

*******************************************************************************/

#include "ossUtil.hpp"
#include "util2Bsonobj.hpp"

namespace engine
{
   INT32 utilStr2Datet( const CHAR *str, Date_t &date)
   {
      INT32 rc = SDB_OK ;

      time_t timep ;
      struct tm t ;
      ossMemset( &t, 0, sizeof( t ) ) ;
      INT32 year = 0 ;
      INT32 mon = 0 ;
      INT32 day = 0 ;
      if ( !sscanf( str, "%d-%d-%d", &year, &mon, &day ) )
      {
         rc = SDB_INVALIDARG ;
         goto error ;
      }

      if( year >= 2038 ||
          mon > 12 ||
          day >= 31 )
      {
         rc = SDB_INVALIDARG ;
         goto error ;
      }

      t.tm_year = year - 1900 ;
      t.tm_mon = mon - 1 ;
      t.tm_mday = day ;

      timep = mktime( &t ) ;
      date.millis = timep * 1000 ;
   done:
      return rc ;
   error:
      goto done ;
   }
}
