/*****************************************************************************
*@Description : getLastAliveTime test
*@Modify List : 2016-8-3  Ting YU  Init
*****************************************************************************/
#include <stdio.h>
#include <gtest/gtest.h>
#include <string>
#include <iostream>
#include <time.h>
#include "client.hpp"
#include "../testcommon.hpp"

using namespace std;
using namespace sdbclient;
using namespace bson; 

const CHAR *clName = "time";  
 
TEST( time, getLastAliveTime )
{   
   INT32 rc = SDB_OK;
   sdb db;
   sdbCollection cl; 
   sdbCollectionSpace cs;
   time_t endtime;
   time_t timediff;
         
   // connect to database   
   getConf() ;
   rc = db.connect( HOSTNAME, SVCNAME, USER, PASSWD );                                
   ASSERT_EQ( SDB_OK, rc ); 
   
   usleep(2050000);;
   time( &endtime );
   timediff = difftime( endtime, db.getLastAliveTime() );
   ASSERT_TRUE( timediff  == 2 || timediff == 3 ) <<  "timediff = " << timediff;
   
   // drop cs in ready
   db.dropCollectionSpace( CSNAME );
   
   // create cs
   rc = db.createCollectionSpace( CSNAME, 4096, cs );                                                                                                               
   ASSERT_EQ( SDB_OK, rc ); 
   
   usleep(2050000);;
   time( &endtime );
   timediff = difftime( endtime, db.getLastAliveTime() );
   ASSERT_TRUE( timediff  == 2 || timediff == 3 ) <<  "timediff = " << timediff;   
   
   // create cl
   rc = cs.createCollection( clName, cl ); 
   ASSERT_EQ( SDB_OK, rc ); 
   
   usleep(2050000);;
   time( &endtime );
   timediff = difftime( endtime, db.getLastAliveTime() );
   ASSERT_TRUE( timediff  == 2 || timediff == 3 ) <<  "timediff = " << timediff;  //can be more than 2s if machine seize up
   
   // create cl again, throw error   
   rc = cs.createCollection( clName, cl ); 
   ASSERT_EQ( SDB_DMS_EXIST, rc ); 
   
   usleep(2050000);;
   time( &endtime );
   timediff = difftime( endtime, db.getLastAliveTime() );
   ASSERT_TRUE( timediff  == 2 || timediff == 3 ) <<  "timediff = " << timediff; 
   
   // insert
   BSONObj obj;
   obj = BSON ( "name" << "tom" << "age" << 24 );
   rc = cl.insert( obj );                                                   
   ASSERT_EQ( SDB_OK, rc ); 
   
   usleep(2050000);;
   time( &endtime );
   timediff = difftime( endtime, db.getLastAliveTime() );
   ASSERT_TRUE( timediff  == 2 || timediff == 3 ) <<  "timediff = " << timediff;  
   
   // query 
   sdbCursor cursor;
   cl.query ( cursor ) ;
   
   usleep(2050000);;
   time( &endtime );
   timediff = difftime( endtime, db.getLastAliveTime() );
   ASSERT_TRUE( timediff  == 2 || timediff == 3 ) <<  "timediff = " << timediff;  
   
   // drop cs, clean environment   
   db.dropCollectionSpace( CSNAME );
   
   usleep(2050000);;
   time( &endtime );
   timediff = difftime( endtime, db.getLastAliveTime() );
   ASSERT_TRUE( timediff  == 2 || timediff == 3 ) <<  "timediff = " << timediff;   
}

