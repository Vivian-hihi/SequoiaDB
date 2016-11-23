#ifndef TESTCOMMON_H__
#define TESTCOMMON_H__
#include "client.h"

#define USER                  ""
#define PASSWD                "" 
#define RESTPORT               11814

extern char HOSTNAME[100] ;
extern char SVCNAME[100] ;
extern char CHANGEDPREFIX[100] ;

SDB_EXTERN_C_START

INT32 createCollection ( sdbConnectionHandle *cl, CHAR *csName, CHAR *clName );
BOOLEAN isStandalone( sdbConnectionHandle db );
void getConf() ;
void getUniqueName(const char* modName,char name[]) ;

SDB_EXTERN_C_END

#endif
