/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

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

#include "util2Bsonobj.hpp"
#include "ossUtil.hpp"


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
      date.millis = timep ;
   done:
      return rc ;
   error:
      goto done ;
   }
}
