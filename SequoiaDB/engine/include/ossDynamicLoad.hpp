/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = ossDynamicLoad.hpp

   Descriptive Name =

   When/how to use: this program may be used on binary and text-motionatted
   versions of OSS component. This file contains declare of oss dynamic loading
   functions.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          08/26/2013  TW  Initial Draft

   Last Changed =

*******************************************************************************/
#ifndef OSSDYNAMICLOAD_HPP__
#define OSSDYNAMICLOAD_HPP__

#include "core.hpp"
#include "oss.hpp"
typedef INT32 ( OSS_MODULE_FUNCTION ) () ;
typedef OSS_MODULE_FUNCTION *OSS_MODULE_PFUNCTION ;
#if defined (_WINDOWS)
typedef HMODULE OSS_MHANDLE ;
#else
typedef void* OSS_MHANDLE ;
#endif
#define OSS_MINVALIDHANDLE NULL

#if defined (_WINDOWS)
#define OSSMODULEHANDLE_ERR(rc)                               \
switch(rc)                                                    \
{                                                             \
case ERROR_INVALID_ACCESS:                                    \
case ERROR_ACCESS_DENIED:                                     \
   rc = SDB_PERM ;                                            \
   break ;                                                    \
case ERROR_GEN_FAILURE :                                      \
case ERROR_MOD_NOT_FOUND :                                    \
case ERROR_FILE_NOT_FOUND :                                   \
case ERROR_INVALID_NAME :                                     \
case ERROR_PATH_NOT_FOUND :                                   \
   rc = SDB_FNE ;                                             \
   break ;                                                    \
case ERROR_NOT_ENOUGH_MEMORY :                                \
   rc = SDB_OOM ;                                             \
   break ;                                                    \
default :                                                     \
   rc = SDB_SYS ;                                             \
   break ;                                                    \
}                                                             \


#endif

class _ossModuleHandle : public SDBObject
{
public :
   BOOLEAN           _isInitialized ;
   CHAR              _moduleName [ OSS_MAX_PATHSIZE + 1 ] ;
   OSS_MHANDLE       _moduleHandle ;
   CHAR              _libPath [ OSS_MAX_PATHSIZE + 1 ] ;
   UINT32            _flags ;
   _ossModuleHandle ( const CHAR *pModuleName,
                      const CHAR *pLibraryPath,
                      UINT32 dlOpenMode = 0 ) ;
   ~_ossModuleHandle ()
   {
      unload () ;
   }
   INT32 init () ;
   INT32 unload () ;
   INT32 resolveAddress ( const CHAR *pFunctionName,
                          OSS_MODULE_PFUNCTION *pFunctionAddress ) ;
} ;
typedef class _ossModuleHandle ossModuleHandle ;

INT32 ossLoadModule ( ossModuleHandle *mHandle, const CHAR *pModuleName,
                      const CHAR *pLibraryPath,
                      UINT32 dlOpenMode, UINT32 options = 0 ) ;

INT32 ossUnloadModule ( ossModuleHandle *mHandle ) ;

INT32 ossResolveAddress ( ossModuleHandle *mHandle,
                          const CHAR *pFunctionName,
                          OSS_MODULE_PFUNCTION *pFunctionAddress ) ;
#endif
