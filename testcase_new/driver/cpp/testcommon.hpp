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

extern string HSOTAME ;
extern string SVCNAME ;
extern string CHANGEDPREFIX ;

void createCollection( sdb &db, sdbCollection *cl, const CHAR *clName );
void getConf() ;

void ossSleep(int milliseconds) ;
bool isStandalone() ;



#endif
