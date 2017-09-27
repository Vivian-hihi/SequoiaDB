#ifndef TESTCOMMON_HPP__
#define TESTCOMMON_HPP__
#include "client.h"
#include <stdio.h>
#include <vector>
#include <string>
#include <stdarg.h>

using namespace std ;

#define CHECK_RC( expRc, actRc, fmt, ... ) \
do { \
   if( expRc != actRc ) \
   { \
      printMsg( fmt, ##__VA_ARGS__ ) ; \
      cout << endl ; \
      goto error ; \
   } \
} while( 0 ) ;

SDB_EXTERN_C_START

void printMsg( const CHAR* fmt, ... ) ;

// create collection
INT32 createNormalCsCl( sdbConnectionHandle db, sdbCSHandle* cs, sdbCollectionHandle* cl,
				            const CHAR* csName, const CHAR* clName ) ;

// check standalone
BOOLEAN isStandalone( sdbConnectionHandle db ) ;

// get hostname like sdbserver1
INT32 getLocalHost( CHAR hostName[], INT32 len ) ;

// get idle port between RSRVPORTBEGIN and RSRVPORTEND
void getIdlePort( CHAR* port ) ;

// get all data groups
INT32 getGroups( sdbConnectionHandle db, vector<string>& groups ) ;

// get all group nodes
INT32 getGroupNodes( sdbConnectionHandle db, const CHAR* rgName, vector<string>& nodes ) ;

SDB_EXTERN_C_END

#endif
