#include "testcommon.hpp"
#include <string>
#include <iostream>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <gtest/gtest.h>

using namespace std;
using namespace sdbclient;
using namespace bson;

void createCollection( sdb &db, sdbCollection *cl, const CHAR *clName )
{
   INT32 rc = SDB_OK;
   
   //new sdb       
   const CHAR *hostName = HOST;
   const CHAR *svcPort  = SERVER;
   rc = db.connect( hostName, svcPort, "", "" );
   ASSERT_EQ( SDB_OK, rc );
   
   //create cs
   sdbCollectionSpace cs;
   const CHAR *csName = CSNAME;  
   rc = db.createCollectionSpace( csName, 4096, cs );
   if( SDB_DMS_CS_EXIST == rc )
   {
      rc = db.getCollectionSpace( csName, cs );
   }
   ASSERT_EQ( SDB_OK, rc );
   
   //create cl
   BSONObj optionObj = BSON( "ReplSize" << 0 );  
   rc = cs.createCollection( clName, optionObj, *cl );
   if( SDB_DMS_EXIST == rc )
   {
      rc = cs.dropCollection( clName );
      ASSERT_EQ( SDB_OK, rc );
      rc = cs.createCollection( clName, *cl );
   }
   ASSERT_EQ( SDB_OK, rc );
}
