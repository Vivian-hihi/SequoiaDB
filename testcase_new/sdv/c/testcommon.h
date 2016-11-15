#ifndef TESTCOMMON_H__
#define TESTCOMMON_H__
#include "client.h"

#define HOST                  "localhost"
#define SERVER                "11810"
#define USER                  ""
#define PASSWD                "" 
#define RESTPORT               11814

SDB_EXTERN_C_START

INT32 createCollection ( sdbConnectionHandle *cl, CHAR *csName, CHAR *clName );
BOOLEAN isStandalone( sdbConnectionHandle db );

SDB_EXTERN_C_END

#endif
