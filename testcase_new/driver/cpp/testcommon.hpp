#ifndef TESTCOMMON_HPP__
#define TESTCOMMON_HPP__

#include "client.hpp"

using namespace sdbclient ; 

#define HOST                  "localhost"
#define SERVER                "11810" 
#define CSNAME                "story_cpp_test_cs" 

extern string HSOTAME ;
extern string SVCNAME ;
extern string CHANGEDPREFIX ;

void createCollection( sdb &db, sdbCollection *cl, const CHAR *clName );
void getConf() ;

#endif
