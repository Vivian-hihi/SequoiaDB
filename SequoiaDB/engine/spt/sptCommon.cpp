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

*******************************************************************************/

#include "sptCommon.hpp"
#include "pd.hpp"
#include "ossUtil.h"
#include "ossMem.h"

namespace engine
{
   /*
      Global function
   */
   static OSS_THREAD_LOCAL CHAR *__errmsg__ = NULL ;
   static OSS_THREAD_LOCAL INT32 __errno__ = SDB_OK ;
   static OSS_THREAD_LOCAL BOOLEAN __printError__ = TRUE ;

   const CHAR *sdbGetErrMsg()
   {
      return __errmsg__ ;
   }

   void sdbSetErrmsg( const CHAR *err )
   {
      static CHAR* s_emptyMsg = "" ;
      if ( NULL != __errmsg__ && s_emptyMsg != __errmsg__ )
      {
         SDB_OSS_FREE( __errmsg__ ) ;
         __errmsg__ = s_emptyMsg ;
      }
      if ( err && 0 != *err )
      {
         __errmsg__ = ossStrdup( err ) ;
      }
   }

   BOOLEAN sdbIsErrMsgEmpty()
   {
      if ( __errmsg__ && *__errmsg__ )
      {
         return FALSE ;
      }
      return TRUE ;
   }

   INT32 sdbGetErrno()
   {
      return __errno__ ;
   }

   void sdbSetErrno( INT32 errNum )
   {
      __errno__ = errNum ;
   }

   void sdbClearErrorInfo()
   {
      sdbSetErrno( SDB_OK ) ;
      sdbSetErrmsg( NULL ) ;
   }

   BOOLEAN sdbNeedPrintError()
   {
      return __printError__ ;
   }

   void sdbSetPrintError( BOOLEAN print )
   {
      __printError__ = print ;
   }

}

