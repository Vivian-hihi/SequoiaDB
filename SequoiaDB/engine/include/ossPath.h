#ifndef OSS_PATH_H
#define OSS_PATH_H

#include "ossTypes.h"

#if defined (_WINDOWS)
#define OSS_PATH_SEP_CHAR '\\'
#define OSS_PATH_SEP_SET "\\"
#else
#define OSS_PATH_SEP_CHAR '/'
#define OSS_PATH_SEP_SET "/"
#endif

/**
 * locate an executable by providing the path of another executable in the
 * same directory and its own name.
 * On Windows, refPath=C:\mypath\a.exe, exeName=b.exe, then path will contain
 * C:\mypath\b.exe
 * On Linux, refPath=/home/a, exeName=b, then path will contain /home/b
 */
INT32 ossLocateExecutable ( const CHAR * refPath ,
                            const CHAR * exeName ,
                            CHAR * buf ,
                            UINT32 bufSize ) ;
#endif // OSS_PATH_H

