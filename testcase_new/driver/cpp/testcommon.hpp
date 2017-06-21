#ifndef TESTCOMMON_HPP__
#define TESTCOMMON_HPP__

#include "client.hpp"
#include <iostream>

using namespace sdbclient ; 

#define CHECK_RC( rc, msg ) \
do { \
	if( rc != SDB_OK ) \
	{ \
		std::cout << msg << ", rc = " << rc << std::endl ; \
		goto error ; \
	} \
} while( 0 ) ;

#define COORD                 "192.168.31.19:11810"
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

void createCollection( sdb &db, sdbCollection *cl, const CHAR *clName );
void getConf() ;
void ossSleep(int milliseconds) ;
bool isStandalone() ;
bool IsStandalone( sdb& db ) ;
INT32 createNormalCl( sdb& db, sdbCollectionSpace& cs, sdbCollection& cl,
					  const char* csname, const char* clname ) ;

#endif
