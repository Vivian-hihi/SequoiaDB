#include "ossPath.h"
#include "ossErr.h"
#include "ossUtil.h"
#include "ossMem.h"
#include "pd.hpp"
#include "pdTrace.hpp"
#include "ossTrace.hpp"

#if defined (_WINDOWS)

#include <string>
using std::string ;

// append .exe if it doesn't have one
PD_TRACE_DECLARE_FUNCTION ( SDB_GETEXECNM, "getExecutableName" )
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

PD_TRACE_DECLARE_FUNCTION ( SDB_OSSLCEXEC, "ossLocateExecutable" )
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

