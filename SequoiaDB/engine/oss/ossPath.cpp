
#include "ossPath.hpp"
#include "ossErr.h"
#include "ossUtil.h"
#include "ossMem.h"
#include "pd.hpp"
#include "pdTrace.hpp"
#include "ossTrace.hpp"

#include <boost/filesystem.hpp>
#include <boost/filesystem/path.hpp>

namespace fs = boost::filesystem ;

#if defined (_WINDOWS)

// append .exe if it doesn't have one
// PD_TRACE_DECLARE_FUNCTION ( SDB_GETEXECNM, "getExecutableName" )
static INT32 getExecutableName ( const CHAR * exeName ,
                                 CHAR * buf ,
                                 UINT32 bufSize )
{
   INT32 rc = SDB_OK ;
   PD_TRACE_ENTRY ( SDB_GETEXECNM );

   SDB_ASSERT ( exeName && buf && bufSize > 0 , "invalid argument" ) ;

   try
   {
      string strName = exeName ;
      string strEnd  = ".exe" ;
      if ( strName.length() <= strEnd.length() ||
           0 != strName.compare ( strName.length() - strEnd.length() ,
                                  strEnd.length() , strEnd ) )
      {
         strName += strEnd ;
      }

      if ( strName.length() >= bufSize )
      {
         rc = SDB_INVALIDSIZE ;
         goto error ;
      }
      ossStrcpy ( buf , strName.c_str() ) ;
   }
   catch ( std::bad_alloc & )
   {
      rc = SDB_OOM ;
      goto error ;
   }

done :
   PD_TRACE_EXITRC ( SDB_GETEXECNM, rc );
   return rc ;
error :
   goto done ;
}

#endif

// PD_TRACE_DECLARE_FUNCTION ( SDB_OSSLCEXEC, "ossLocateExecutable" )
INT32 ossLocateExecutable ( const CHAR * refPath ,
                            const CHAR * exeName ,
                            CHAR * buf ,
                            UINT32 bufSize )
{
   INT32          rc          = SDB_OK ;
   PD_TRACE_ENTRY ( SDB_OSSLCEXEC );
   INT32          dirLen      = 0 ;
   UINT32         exeLen      = 0 ;
   const CHAR *   separator   = NULL ;
   CHAR newExeName[ OSS_MAX_PATHSIZE + 1 ] = {0} ;

   ossMemset ( newExeName , 0 , sizeof ( newExeName ) ) ;

   if ( ! ( refPath && exeName && buf && bufSize > 0 ) )
   {
      rc = SDB_INVALIDARG ;
      goto error ;
   }

#ifdef _WINDOWS
   rc = getExecutableName ( exeName , newExeName , sizeof ( newExeName ) ) ;
   if ( rc != SDB_OK )
   {
      goto error ;
   }
#else
   if ( ossStrlen ( exeName ) >= sizeof ( newExeName ) )
   {
      rc = SDB_INVALIDSIZE ;
      goto error ;
   }
   ossStrncpy ( newExeName , exeName, sizeof ( newExeName ) ) ;
#endif

   exeLen = ossStrlen ( newExeName ) ;

   separator = ossStrrchr ( refPath , OSS_PATH_SEP_CHAR ) ;
   if ( ! separator )
   {
      // refPath is resolved using system's PATH variable.
      // Addtionally, on Windows, it may be in the current working diretory.
      // so we can just copy newExeName to buf.
      if ( exeLen >= bufSize )
      {
         rc = SDB_INVALIDSIZE ;
         goto error ;
      }
      ossStrcpy ( buf , newExeName ) ;
      goto done ;
   }

   dirLen = separator - refPath ; // length without separator

   if ( dirLen + exeLen + 1 >= bufSize )
   {
      rc = SDB_INVALIDSIZE ;
      goto error ;
   }

   ossStrncpy ( buf , refPath , dirLen + 1 ) ; // 1 for separator
   buf[dirLen + 1] = '\0' ;
   ossStrncat ( buf , newExeName , exeLen ) ;

done :
   PD_TRACE_EXITRC ( SDB_OSSLCEXEC, rc );
   return rc ;
error :
   goto done ;
}

enum OSS_MATCH_TYPE
{
   OSS_MATCH_LEFT,
   OSS_MATCH_MID,
   OSS_MATCH_RIGHT,
   OSS_MATCH_ALL,
   OSS_MATCH_NULL
} ;

static INT32 _ossEnumFiles( const string &dirPath,
                            map<string, string> &mapFiles,
                            const CHAR *filter, UINT32 filterLen,
                            OSS_MATCH_TYPE type, UINT32 deep )
{
   INT32 rc = SDB_OK ;
   const CHAR *pFind = NULL ;

   fs::path dbDir ( dirPath ) ;
   fs::directory_iterator end_iter ;

   if ( 0 == deep )
   {
      goto done ;
   }

   if ( fs::exists ( dbDir ) && fs::is_directory ( dbDir ) )
   {
      for ( fs::directory_iterator dir_iter ( dbDir );
            dir_iter != end_iter; ++dir_iter )
      {
         if ( fs::is_regular_file ( dir_iter->status() ) )
         {
            const std::string fileName =
               dir_iter->path().filename().string() ;

            if ( ( OSS_MATCH_NULL == type ) ||
                 ( OSS_MATCH_LEFT == type &&
                   0 == ossStrncmp( fileName.c_str(), filter, filterLen ) ) ||
                 ( OSS_MATCH_MID == type &&
                   ossStrstr( fileName.c_str(), filter ) ) ||
                 ( OSS_MATCH_RIGHT == type &&
                   ( pFind = ossStrstr( fileName.c_str(), filter ) ) &&
                   pFind[filterLen] == 0 ) ||
                 ( OSS_MATCH_ALL == type &&
                   0 == ossStrcmp( fileName.c_str(), filter ) )
               )
            {
               mapFiles[ fileName ] = dir_iter->path().string() ;
            }
         }
         else if ( fs::is_directory( dir_iter->path() ) && deep > 1 )
         {
            _ossEnumFiles( dir_iter->path().string(), mapFiles,
                           filter, filterLen, type, deep - 1 ) ;
         }
      }
   }
   else
   {
      rc = SDB_INVALIDARG ;
      goto error ;
   }

done:
   return rc ;
error:
   goto done ;
}

INT32 ossEnumFiles( const string &dirPath,
                    map< string, string > &mapFiles,
                    const CHAR *filter,
                    UINT32 deep )
{
   INT32 rc = SDB_OK ;
   string newFilter ;
   OSS_MATCH_TYPE type = OSS_MATCH_NULL ;

   if ( !filter || filter[0] == 0 || 0 == ossStrcmp( filter, "*" ) )
   {
      type = OSS_MATCH_NULL ;
   }
   else if ( filter[0] != '*' && filter[ ossStrlen( filter ) - 1 ] != '*' )
   {
      type = OSS_MATCH_ALL ;
      newFilter = filter ;
   }
   else if ( filter[0] == '*' && filter[ ossStrlen( filter ) - 1 ] == '*' )
   {
      type = OSS_MATCH_MID ;
      newFilter.assign( &filter[1], ossStrlen( filter ) - 2 ) ;
   }
   else if ( filter[0] == '*' )
   {
      type = OSS_MATCH_RIGHT ;
      newFilter.assign( &filter[1], ossStrlen( filter ) - 1 ) ;
   }
   else
   {
      type = OSS_MATCH_LEFT ;
      newFilter.assign( filter, ossStrlen( filter ) -1 ) ;
   }

   return _ossEnumFiles( dirPath, mapFiles, newFilter.c_str(),
                         newFilter.length(), type, deep ) ;
}

