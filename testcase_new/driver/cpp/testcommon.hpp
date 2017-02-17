#ifndef TESTCOMMON_HPP__
#define TESTCOMMON_HPP__

#include "client.hpp"

using namespace sdbclient ; 

//#define COORD                 "192.168.20.166:11810"
//#define HOST                  "192.168.20.166"
//#define SERVER                "11810" 

#define COORD                 "localhost:11810"
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



#endif
