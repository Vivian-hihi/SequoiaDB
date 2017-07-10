#ifndef TESTCOMMON_HPP__
#define TESTCOMMON_HPP__

#include "client.hpp"
#include <iostream>
#include <stdarg.h>

using namespace sdbclient ; 

#define CHECK_RC( rc, fmt, ... ) \
do { \
	if( rc != SDB_OK ) \
	{ \
		printMsg( fmt, ##__VA_ARGS__ ) ; \
		goto error ; \
	} \
} while( 0 ) ;

#define ASSERT_RC( rc, fmt, ... ) \
do { \
    if( rc != SDB_OK ) \
    { \
        printMsg( fmt, ##__VA_ARGS__ ) ; \
        exit(1) ; \
    } \
} while( 0 ) ;

#define HOST                  "localhost"
#define SERVER                "11810" 
#define CSNAME                "story_cpp_test_cs" 

extern char HOSTNAME[100] ;
extern char SVCNAME[100] ;
extern char CHANGEDPREFIX[100] ;
extern char RSRVPORTBEGIN[100] ;
extern char RSRVPORTEND[100] ;
extern char RSRVNODEDIR[100] ;
extern char WORKDIR[100] ;
extern char COORD[100] ;

void printMsg( const char* fmt, ... ) ;
void getConf() ;
void ossSleep( int milliseconds ) ;
bool isStandalone( sdb& db ) ;
int createNormalCl( sdb& db, sdbCollectionSpace& cs, sdbCollection& cl,
					const char* csname, const char* clname ) ;

#endif
