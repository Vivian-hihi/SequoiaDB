#ifndef TESTCOMMON_HPP__
#define TESTCOMMON_HPP__

#include "client.hpp"

using namespace sdbclient ; 

#define HOST                  "localhost"
#define SERVER                "11810" 
#define CSNAME                "story_cpp_test_cs" 


void createCollection( sdb &db, sdbCollection *cl, const CHAR *clName );

#endif
