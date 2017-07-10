/*****************************************************************************
*@Description : seqDB-7549:c++_输入strict格式，查询显示
                seqDB-7550:c++_strict格式的参数校验
                seqDB-7551:c++_strict格式的边界值校验
*@Modify List : 2016-3-29  Ting YU  Init
*****************************************************************************/
#include <stdio.h>
#include <gtest/gtest.h>
#include <string>
#include <iostream>
#include <sstream>
#include "client.hpp"
#include "../testcommon.hpp"

using namespace std;
using namespace sdbclient;
using namespace bson; 

const CHAR *clName = "numberLong";  

void checkLongVal( sdbCollection &cl, const long long longVal )
{
   INT32 rc = SDB_OK;
   
   //number to string 
   stringstream ss;
   ss << longVal;
   string longValStr = ss.str();

   //json to bson
   string recJson = "{ \"a\": { \"$numberLong\": \"" + longValStr + "\" } }";   
   BSONObj recBson;
   fromjson( recJson, recBson, true );
   ASSERT_TRUE( recBson.getField("a").Long() == longVal );
        
   //insert 
   rc = cl.del();
   ASSERT_EQ( SDB_OK, rc );   
   rc = cl.insert( recBson );
   ASSERT_EQ( SDB_OK, rc );
   
   //query
   INT32 i = 0;
   sdbCursor cursor;
   cl.query( cursor );
   BSONObj qryRec;
   while( !( cursor.next(qryRec) ) )
   {  
      //check value
      long long qryVal = qryRec.getField("a").Long();
      ASSERT_TRUE( qryVal == longVal );     
      i++;
   }
   ASSERT_TRUE( i == 1 );
   
   rc = cursor.close();
   ASSERT_EQ( SDB_OK, rc );
}
  
TEST( numberLong, boundary )
{         
   //create cl
   sdb db;
   sdbCollectionSpace cs;
   sdbCollection cl;
   int rc = createNormalCl( db, cs, cl, CSNAME, clName );
   ASSERT_EQ( rc, SDB_OK ) ;   
   
   long long longMax = 9223372036854775807;   
   cout << "---begin to checkLongVal with long = " << longMax << endl;  
   checkLongVal( cl, longMax );
   
   long long longMin = -9223372036854775808;
   cout << "---begin to checkLongVal with long = " << longMin << endl;   
   checkLongVal( cl, longMin );   
}

TEST( numberLong, outofBoundary )
{
   INT32 rc = SDB_OK ;
   string recJson;
   BSONObj recBson; 

   //json to bson
   recJson = "{ \"a\": { \"$numberLong\": \"9223372036854775808\" } }";   
   rc = fromjson( recJson, recBson, true );
   ASSERT_EQ( -6, rc );

   //json to bson
   recJson = "{ \"a\": { \"$numberLong\": \"-9223372036854775809\" } }";
   rc = fromjson( recJson, recBson, true );
   ASSERT_EQ( -6, rc );  
    
}

TEST( numberLong, errorFormat )
{             
   INT32 rc = SDB_OK ;
   string recJson ;
   BSONObj recBson ;

   //1. error fomart: {"$numberLong":"123.5"}       
   recJson = "{ \"a\": { \"$numberLong\": \"123.5\" } }" ;  
   rc = fromjson( recJson, recBson, true );
   ASSERT_EQ( -6, rc );    

   //2. error fomart: {"$numberLong":123}       
   recJson = "{ \"a\": { \"$numberLong\": 123.5 } }" ;
   rc = fromjson( recJson, recBson, true );
   ASSERT_EQ( -6, rc );       
}
