#ifndef OSSVER_H__
#define OSSVER_H__

#include "core.h"
#include "ossVer_Autogen.h"

/*
 *    SequoiaDB Engine Version
 */
#define SDB_ENGINE_VERSION_0           0
#define SDB_ENGINE_VERSION_1           1

#define SDB_ENGINE_VERISON_CURRENT     SDB_ENGINE_VERSION_1

/*
 *    SequoiaDB Engine Subversion
 */
#define SDB_ENGINE_SUBVERSION_0        0
#define SDB_ENGINE_SUBVERSION_1        1
#define SDB_ENGINE_SUBVERSION_2        2
#define SDB_ENGINE_SUBVERSION_3        3
#define SDB_ENGINE_SUBVERSION_5        5
#define SDB_ENGINE_SUBVERSION_6        6
#define SDB_ENGINE_SUBVERSION_7        7
#define SDB_ENGINE_SUBVERSION_8        8
#define SDB_ENGINE_SUBVERSION_10       10


#define SDB_ENGINE_SUBVERSION_CURRENT  SDB_ENGINE_SUBVERSION_10

#ifdef _DEBUG
   #define SDB_ENGINE_BUILD_TIME    SDB_ENGINE_BUILD_CURRENT"(Debug)"
#else
   #define SDB_ENGINE_BUILD_TIME    SDB_ENGINE_BUILD_CURRENT
#endif // _DEBUG

/*
 *    Get the version, subversion and release version.
 */
void ossGetVersion ( INT32 *version,
                     INT32 *subVersion,
                     INT32 *release,
                     const CHAR **ppBuild ) ;

void ossPrintVersion( const CHAR *prompt ) ;

#endif /* OSSVER_HPP_ */

