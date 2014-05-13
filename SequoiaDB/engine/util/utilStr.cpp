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

   Source File Name = utilStr.cpp

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
#include "utilStr.hpp"
#include "ossUtil.hpp"
#include "pd.hpp"

namespace engine
{
   INT32 utilStrTrimBegin( const CHAR *src, const CHAR *&begin )
   {
      INT32 rc = SDB_OK ;
      if ( NULL == src )
      {
         rc = SDB_INVALIDARG ;
         goto error ;
      }

      {
      UINT32 len = ossStrlen( src ) ;
      UINT32 sub = 0 ;
      while ( sub < len )
      {
         if ( ' ' == src[sub]
              || '\t' == src[sub] )
         {
            ++sub ;
         }
         else
         {
            break ;
         }
      }

      begin = &(src[sub]) ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 utilStrTrimEnd( CHAR *src )
   {
      INT32 rc = SDB_OK ;
      if ( NULL == src )
      {
         rc = SDB_INVALIDARG ;
         goto error ;
      }

      {
      UINT32 len = ossStrlen( src ) ;
      INT32 sub = len - 1 ;
      while ( -1 < sub )
      {
         if ( ' ' == src[sub]
              || '\t' == src[sub] )
         {
            src[sub--] = '\0' ;
         }
         else
         {
            break ;
         }
      }
      }
   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 utilStrTrim( CHAR *src, const CHAR *&begin )
   {
      INT32 rc = SDB_OK ;
      const CHAR *tmpBegin = NULL ;
      rc = utilStrTrimBegin( src, tmpBegin ) ;
      if ( SDB_OK != rc )
      {
         goto error ;
      }

      rc = utilStrTrimEnd( (CHAR *)tmpBegin ) ;
      if ( SDB_OK != rc )
      {
         goto error ;
      }

      begin = tmpBegin ;
   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 utilStrToUpper( const CHAR *src, CHAR *&upper )
   {
      INT32 rc = SDB_OK ;
      CHAR *tmp = NULL ;
      UINT32 size = 0 ;
      if ( NULL == src )
      {
         rc = SDB_INVALIDARG ;
         goto error ;
      }

      size = ossStrlen( src) + 1 ;
      tmp = (CHAR *)SDB_OSS_MALLOC(size) ;
      if ( NULL == tmp )
      {
         rc = SDB_OOM ;
         PD_LOG( PDERROR, "failed to allocate mem." ) ;
         goto error ;
      }

      /// '\0' is contained.
      for ( UINT32 i = 0; i < size ; i++ )
      {
         tmp[i] = ( src[i] >= 'a' && src[i] <= 'z' ) ?
                    src[i] - 32 : src[i] ;
      }

      upper = tmp ;
   done:
      return rc ;
   error:
      if ( NULL != tmp )
      {
         SDB_OSS_FREE( tmp ) ;
      }
      goto done ;
   }

   INT32 utilStrJoin( const CHAR **src,
                      UINT32 cnt,
                      CHAR *join,
                      UINT32 &joinSize )
   {
      SDB_ASSERT( NULL != join, "impossible" )
      INT32 rc = SDB_OK ;
      UINT32 len = 0 ;
      for ( UINT32 i = 0; i < cnt; i++ )
      {
         if ( NULL != src[i] )
         {
            UINT32 sLen = ossStrlen(src[i]) ;
            ossMemcpy( join + len, src[i], sLen ) ;
            len += sLen ;
         }
      }
      joinSize = len ;
      return rc ;
   }

   INT32 utilStr2TimeT( const CHAR *str,
                        time_t &tm,
                        UINT64 *usec )
   {
      INT32 rc = SDB_OK ;
      struct tm t ;
      memset ( &t, 0, sizeof(t) ) ;
      INT32 year   = 0 ;
      INT32 month  = 0 ;
      INT32 day    = 0 ;
      INT32 hour   = 0 ;
      INT32 minute = 0 ;
      INT32 second = 0 ;
      INT32 micros = 0 ;

      if ( NULL != usec )
      {
         if ( !sscanf ( str,
                     "%d-%d-%d.%d.%d.%d.%d",
                     &year   ,
                     &month  ,
                     &day    ,
                     &hour   ,
                     &minute ,
                     &second ,
                     &micros ) )
         {
            rc = SDB_INVALIDARG ;
            goto error ;
         }
      }
      else
      {
         if ( !sscanf( str, "%d-%d-%d", &year, &month, &day ) )
         {
            rc = SDB_INVALIDARG ;
            goto error ;
         }
      }

      t.tm_year  = year - 1900  ;
      t.tm_mon   = month - 1 ;
      t.tm_mday  = day    ;
      t.tm_hour  = hour   ;
      t.tm_min   = minute ;
      t.tm_sec   = second ;

      tm = mktime( &t ) ;

      if ( NULL != usec )
      {
         *usec = micros ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 utilBuildFullPath( const CHAR *path, const CHAR *name,
                            UINT32 fullSize, CHAR *full )
   {
      INT32 rc = SDB_OK ;
      if ( ossStrlen( path ) + ossStrlen( name )
           + 2 > fullSize )
      {
         rc = SDB_INVALIDARG ;
         goto error ;
      }
      ossMemset( full, 0, fullSize );
      ossStrcpy( full, path ) ;
      if ( '\0' != path[0] &&
           0 != ossStrcmp(&path[ossStrlen(path)-1], OSS_FILE_SEP ) )
      {
         ossStrncat( full, OSS_FILE_SEP, 1 ) ;
      }
      ossStrncat( full, name, ossStrlen( name ) ) ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 utilCatPath( CHAR * src, UINT32 srcSize, const CHAR * catStr )
   {
      INT32 rc = SDB_OK ;
      UINT32 srcLen = ossStrlen( src ) ;
      UINT32 catStrLen = ossStrlen( catStr ) ;

      if ( srcLen + catStrLen + 2 > srcSize )
      {
         rc = SDB_INVALIDARG ;
         goto error ;
      }
      if ( srcLen > 0 && src[srcLen-1] != OSS_FILE_SEP_CHAR )
      {
         ossStrncat( src, OSS_FILE_SEP, 1 ) ;
      }
      ossStrncat( src, catStr, catStrLen ) ;

   done:
      return rc ;
   error:
      goto done ;
   }

   const CHAR* utilAscTime( time_t tTime, CHAR * pBuff, UINT32 size )
   {
      struct tm localTm ;
      ossLocalTime( tTime, localTm ) ;

      ossMemset( pBuff, 0, size ) ;
      ossSnprintf( pBuff, size-1,
                   "%04d-%02d-%02d-%02d:%02d:%02d",
                   localTm.tm_year+1900,            // 1) Year (UINT32)
                   localTm.tm_mon+1,                // 2) Month (UINT32)
                   localTm.tm_mday,                 // 3) Day (UINT32)
                   localTm.tm_hour,                 // 4) Hour (UINT32)
                   localTm.tm_min,                  // 5) Minute (UINT32)
                   localTm.tm_sec                   // 6) Second (UINT32)
                  ) ;
      pBuff[ size - 1 ] = 0 ;
      return pBuff ;
   }

}

