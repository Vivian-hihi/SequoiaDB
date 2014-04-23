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

#include "utilPipe.hpp"
#include "pd.hpp"
#include "ossUtil.h"
#include "ossPrimitiveFileOp.hpp"
#include "boost/filesystem.hpp"
#include "boost/filesystem/operations.hpp"
#include "boost/filesystem/path.hpp"

namespace fs = boost::filesystem ;


INT32 getWaitPipeName ( const OSSPID & ppid , CHAR * buf , UINT32 bufSize )
{
   INT32          nWritten    = 0 ;
   INT32          rc          = SDB_OK ;
#if defined (_WINDOWS)
   const CHAR *   waitFormat  = "sdb-shell-wait-%u" ;
#else
   const CHAR *   waitFormat  = "/tmp/sdb-shell-wait-%u" ;
#endif

   SDB_ASSERT ( buf && bufSize > 0 , "invalid argument" ) ;

   nWritten = ossSnprintf ( buf , bufSize , waitFormat , ppid ) ;
   SH_VERIFY_COND ( nWritten >= 0 , SDB_SYS ) ;
   SH_VERIFY_COND ( (UINT32) nWritten < bufSize , SDB_INVALIDSIZE ) ;

done :
   return rc ;
error :
   goto done ;
}

INT32 getPipeNames( const OSSPID & ppid , CHAR * f2bName , UINT32 f2bSize ,
                    CHAR * b2fName , UINT32 b2fSize )
{
   INT32          rc          = SDB_OK ;
   INT32          nWritten    = 0 ;
#if defined (_WINDOWS)
   const CHAR *   f2bFormat   = "sdb-shell-f2b-%u" ;
   const CHAR *   b2fFormat   = "sdb-shell-b2f-%u" ;
#else
   const CHAR *   f2bFormat   = "sdb-shell-f2b-%u" ;
   const CHAR *   b2fFormat   = "sdb-shell-b2f-%u" ;
#endif

   SDB_ASSERT ( f2bName && b2fName && f2bSize > 0 && b2fSize > 0 ,
                "Invalid arguments" ) ;

   nWritten = ossSnprintf ( f2bName , f2bSize , f2bFormat , ppid ) ;
   SH_VERIFY_COND ( nWritten >= 0 , SDB_SYS ) ;
   SH_VERIFY_COND ( (UINT32) nWritten < f2bSize , SDB_INVALIDSIZE ) ;

   nWritten = ossSnprintf ( b2fName , b2fSize , b2fFormat , ppid ) ;
   SH_VERIFY_COND ( nWritten >= 0 , SDB_SYS ) ;
   SH_VERIFY_COND ( (UINT32) nWritten < b2fSize , SDB_INVALIDSIZE ) ;

done :
   return rc ;
error :
   goto done ;
}

INT32 getPipeNames2( const OSSPID & ppid , const OSSPID & pid ,
                     CHAR * f2bName , UINT32 f2bSize ,
                     CHAR * b2fName , UINT32 b2fSize )
{
   INT32          rc          = SDB_OK ;
   INT32          nWritten    = 0 ;
#if defined (_WINDOWS)
   const CHAR *   f2bFormat   = "sdb-shell-f2b-%u-%u" ;
   const CHAR *   b2fFormat   = "sdb-shell-b2f-%u-%u" ;
#else
   const CHAR *   f2bFormat   = "/tmp/sdb-shell-f2b-%u-%u" ;
   const CHAR *   b2fFormat   = "/tmp/sdb-shell-b2f-%u-%u" ;
#endif

   SDB_ASSERT ( f2bName && b2fName && f2bSize > 0 && b2fSize > 0 ,
                "Invalid arguments" ) ;

   nWritten = ossSnprintf ( f2bName , f2bSize , f2bFormat , ppid , pid ) ;
   SH_VERIFY_COND ( nWritten >= 0 , SDB_SYS ) ;
   SH_VERIFY_COND ( (UINT32) nWritten < f2bSize , SDB_INVALIDSIZE ) ;

   nWritten = ossSnprintf ( b2fName , b2fSize , b2fFormat , ppid , pid ) ;
   SH_VERIFY_COND ( nWritten >= 0 , SDB_SYS ) ;
   SH_VERIFY_COND ( (UINT32) nWritten < b2fSize , SDB_INVALIDSIZE ) ;

done :
   return rc ;
error :
   goto done ;
}

INT32 getPipeNames1( CHAR * bpf2bName , UINT32 bpf2bSize ,
                     CHAR * bpb2fName , UINT32 bpb2fSize ,
                     CHAR * f2bName , CHAR * b2fName )
{
   INT32           rc          = SDB_OK ;
   CHAR *          f2bp        = NULL ;
   CHAR *          b2fp        = NULL ;
   INT32           cnt         = 0 ;

   f2bp = bpf2bName ;
   b2fp = bpb2fName ;
#if defined (_WINDOWS)
   const CHAR *    path        = "\\\\.\\pipe\\" ;
#else
   const CHAR *    path        = "/tmp/" ;
   ossStrncpy ( bpf2bName , path , 5 ) ;
   f2bp += 5 ;
   ossStrncpy ( bpb2fName , path , 5 ) ;
   b2fp += 5 ;
#endif

   try
   {
      fs::path pipeDir( path ) ;
      fs::directory_iterator end_iter ;
      if ( fs::exists( pipeDir ) && fs::is_directory( pipeDir ) )
      {
         for ( fs::directory_iterator dir_iter( pipeDir );
               dir_iter != end_iter; ++dir_iter )
         {
            const std::string fileName =
                              dir_iter->path().filename().string() ;
            const CHAR *pFileName = fileName.c_str() ;
            if ( ossStrncmp ( pFileName , b2fName ,
                              ossStrlen ( b2fName ) ) == 0 )
            {
               ossStrncpy ( b2fp , pFileName , bpb2fSize-5 ) ;
               cnt++ ;
            }
            else if ( ossStrncmp ( pFileName , f2bName ,
                      ossStrlen ( f2bName ) ) == 0 )
            {
               ossStrncpy ( f2bp , pFileName , bpf2bSize-5 ) ;
               cnt++ ;
            }
            if ( cnt >= 2 )
               goto done ;
         }
         rc = SDB_FNE ;
         goto error ;
      }
      else
      {
         PD_RC_CHECK ( SDB_INVALIDARG, PDERROR,
                       "Given path %s is not a directory or not exist",
                       path ) ;
      }
   }
   catch ( std::exception &e )
   {
      PD_RC_CHECK ( SDB_SYS, PDERROR,
                    "Failed to iterate pipefile path %s: %s",
                    path, e.what() ) ;
   }

done :
   return rc ;
error :
   goto done ;
}

