/*****************************************************************************
*@Description : decimal test
*@Modify List : 2016-5-12  Ting YU  Init
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

const CHAR *clName = "decimal";  
 
TEST( decimal, insert_query )
{   
   INT32 rc = SDB_OK;
         
   //create cl
   sdb db;
   sdbCollectionSpace cs;
   sdbCollection cl; 
   rc = createNormalCl( db, cs, cl, CSNAME, clName );   
   ASSERT_EQ( rc, SDB_OK ) ;
   
   //decimal
   bsonDecimal decimal;
   decimal.fromInt( 1000 );
   
   //bson
   BSONObj obj;
   BSONObjBuilder b;
   b.append( "a", decimal );
   obj = b.obj();

   //insert
   rc = cl.del();
   ASSERT_EQ( SDB_OK, rc );   
   rc = cl.insert( obj );
   ASSERT_EQ( SDB_OK, rc );
   
   //query
   INT32 i = 0;
   sdbCursor cursor;
   cl.query( cursor );
   BSONObj getOBj;
   while( !( cursor.next(getOBj) ) )
   {  
      //check value
      BSONElement e = getOBj.getField( "a" );
      bsonDecimal getDec = e.numberDecimal();     
      ASSERT_STREQ( "1000", getDec.toString().c_str() );     
      i++;
   } 
   ASSERT_EQ( 1, i );
   
   //clean
   rc = cursor.close();
   ASSERT_EQ( SDB_OK, rc );
}

TEST( decimal, append )
{   
   INT32 rc = SDB_OK;
    
   //decimal
   bsonDecimal dec1;
   dec1.fromInt( 104 );
   
   bsonDecimal dec2( dec1 );
   
   bsonDecimal dec3( dec1 );
   dec3.init();
   
   bsonDecimal dec4;
   dec4.init( 10, 3 );
   dec4.fromDouble( 0.12345 );   
   
   //bson
   BSONObj obj;
   BSONObjBuilder b;
   b.append( "a", dec1 );
   b.append( "b", dec2 );
   b.append( "c", dec3 );
   b.append( "d", dec4 );
   b.appendDecimal( "e", "12345.7" );
   b.appendDecimal( "f", "12345.12345", 10, 4 );
   obj = b.obj();
   
   //bson --> decimal
   BSONElement aEle = obj.getField( "a" );
   bsonDecimal aDec = aEle.numberDecimal();
   BSONElement bEle = obj.getField( "b" );
   bsonDecimal bDec = bEle.numberDecimal();   
   BSONElement cEle = obj.getField( "c" );
   bsonDecimal cDec = cEle.numberDecimal();
   BSONElement dEle = obj.getField( "d" );
   bsonDecimal dDec = dEle.numberDecimal();
   BSONElement eEle = obj.getField( "e" );
   bsonDecimal eDec = eEle.Decimal();
   BSONElement fEle = obj.getField( "f" );
   bsonDecimal fDec;
   fEle.Val(fDec);
               
   //check
   ASSERT_STREQ( "104",          aDec.toString().c_str() );
   ASSERT_STREQ( "104",          bDec.toString().c_str() ); 
   ASSERT_STREQ( "0",            cDec.toString().c_str() ); 
   ASSERT_STREQ( "0.123",        dDec.toString().c_str() ); 
   ASSERT_STREQ( "12345.7",      eDec.toString().c_str() ); 
   ASSERT_STREQ( "12345.1235",   fDec.toString().c_str() );      
}

TEST( decimal, getPrecision )
{   
   INT32 rc = SDB_OK;
   INT32 precision = -1;
   INT32 scale = -1; 
    
   //decimal
   bsonDecimal dec;
   dec.init( 10, 2 );
   dec.fromDouble( 100.123 );   
   
   //getPrecision
   rc = dec.getPrecision( &precision, &scale );
   ASSERT_EQ( SDB_OK, rc ); 
   ASSERT_EQ( 10, precision );
   ASSERT_EQ( 2, scale );
   
   //getPrecision
   precision = -1;
   precision = dec.getPrecision();
   ASSERT_EQ( 10, precision );       
}

TEST( decimal, compare )
{   
   INT32 rc = SDB_OK;
   int result; 
    
   //decimal
   bsonDecimal dec1;
   dec1.fromInt( 104 );   
   bsonDecimal dec2 = dec1;
   
   //compare
   result = dec1.compare( dec2 );
   ASSERT_EQ( 0, result );  
   
   //compare
   result = dec2.compare( 103 );
   ASSERT_EQ( 1, result );        
}

TEST( decimal, add )
{   
   INT32 rc = SDB_OK;
   INT32 i1 = 4;
   INT32 i2 = 104;   
          
   //construnct decimal
   bsonDecimal leftDec;     
   leftDec.fromInt( i1 );
   
   bsonDecimal rightDec;     
   rightDec.fromInt( i2 );
   
   //add, eg: c = a + b
   bsonDecimal resultDec;
   resultDec.init();
   rc = leftDec.add( rightDec , resultDec );
   ASSERT_EQ( SDB_OK, rc );
   
   //check
   INT32 resultInt;
   rc = resultDec.toInt( &resultInt );
   ASSERT_EQ( SDB_OK, rc );
   ASSERT_EQ( i1+i2, resultInt );
   
   //add, eg: a += b
   rc = leftDec.add( rightDec );
   ASSERT_EQ( SDB_OK, rc );
   
   //check
   rc = leftDec.toInt( &resultInt );
   ASSERT_EQ( SDB_OK, rc );
   ASSERT_EQ( i1+i2, resultInt );
}

TEST( decimal, sub )
{   
   INT32 rc = SDB_OK;
   INT64 l1 = 9223372036854775806;
   INT64 l2 = -1;   
          
   //construnct decimal
   bsonDecimal leftDec;     
   leftDec.fromLong( l1 );
   
   bsonDecimal rightDec;     
   rightDec.fromInt( l2 );
   
   //sub, eg: c = a - b
   bsonDecimal resultDec;
   resultDec.init();
   rc = leftDec.sub( rightDec , resultDec );
   ASSERT_EQ( SDB_OK, rc );
   
   //check
   INT64 resultLong;
   rc = resultDec.toLong( &resultLong );
   ASSERT_EQ( SDB_OK, rc );
   ASSERT_EQ( l1-l2, resultLong );   
}

TEST( decimal, mul )
{   
   INT32 rc = SDB_OK;
   FLOAT64 d = 2.5;
   INT32 i = 5; 
          
   //construnct decimal
   bsonDecimal leftDec;     
   leftDec.fromDouble( d );
   
   bsonDecimal rightDec;     
   rightDec.fromInt( i );
   
   //mul, eg: c = a * b
   bsonDecimal resultDec;
   resultDec.init();
   rc = leftDec.mul( rightDec , resultDec );
   ASSERT_EQ( SDB_OK, rc );
   
   //check
   FLOAT64 resultDouble;
   rc = resultDec.toDouble( &resultDouble );
   ASSERT_EQ( SDB_OK, rc );
   ASSERT_EQ( 12.5, resultDouble );   
}

TEST( decimal, div )
{   
   INT32 rc = SDB_OK;
   char s[] = "100";
   INT64 l = 5; 
          
   //construnct decimal
   bsonDecimal leftDec;     
   leftDec.fromString( s );
   
   bsonDecimal rightDec;     
   rightDec.fromLong( l );
   
   //div, eg: c = a / b
   bsonDecimal resultDec1;
   resultDec1.init();
   rc = leftDec.div( rightDec , resultDec1 );
   ASSERT_EQ( SDB_OK, rc );
   
   //check
   string resultStr1;
   resultStr1 = resultDec1.toString();
   cout << "str = " << resultStr1 << endl;
   ASSERT_STREQ( "20.0000000000000000", resultStr1.c_str() );   
   
   //div, eg: c = a / b
   bsonDecimal resultDec2;
   resultDec2.init();
   rc = leftDec.div( l , resultDec2 );
   ASSERT_EQ( SDB_OK, rc );
   
   //check
   string resultStr2;
   resultStr2 = resultDec2.toString();
   cout << "str = " << resultStr2 << endl;
   ASSERT_STREQ( "20.0000000000000000", resultStr2.c_str() );    
}

TEST( decimal, mod )
{   
   INT32 rc = SDB_OK;
   INT32 left = 99;
   INT32 right = 5; 
          
   //construnct decimal
   bsonDecimal leftDec;     
   leftDec.fromInt( left );
   
   bsonDecimal rightDec;     
   rightDec.fromInt( right );
   
   //mod, eg: c = a mod b
   bsonDecimal resultDec;
   resultDec.init();
   rc = leftDec.mod( rightDec , resultDec );
   ASSERT_EQ( SDB_OK, rc );
   
   //check
   string resultStr;
   resultStr = resultDec.toString();
   ASSERT_STREQ( "4", resultStr.c_str() );          
}

TEST( decimal, abs_getSign )
{   
   INT32 rc = SDB_OK;
   INT32 left = -1;
          
   //construnct decimal
   bsonDecimal leftDec;     
   leftDec.fromInt( left );  
   
   //get sign
   INT16 sign = 0;
   sign = leftDec.getSign();
   cout << "sign = " << sign << endl;
   ASSERT_NE( 0, sign );
   
   //abs
   INT32 absResult;
   rc = leftDec.abs(); 
   ASSERT_EQ( SDB_OK, rc );
   
   //check
   string resultStr;
   resultStr = leftDec.toString();
   ASSERT_STREQ( "1", resultStr.c_str() );  
}

TEST( decimal, ceil_floor )
{   
   INT32 rc = SDB_OK;
   FLOAT64 left = 9.5;
          
   //construnct decimal
   bsonDecimal leftDec;     
   leftDec.fromDouble( left );  
   
   //ceil
   bsonDecimal resultDec1;
   rc = leftDec.ceil( resultDec1 ); 
   ASSERT_EQ( SDB_OK, rc );
   
   //check
   string resultStr1;
   resultStr1 = resultDec1.toString();
   ASSERT_STREQ( "10", resultStr1.c_str() );
   
   //floor
   bsonDecimal resultDec2;
   rc = leftDec.floor( resultDec2 ); 
   ASSERT_EQ( SDB_OK, rc );
   
   //check
   string resultStr2;
   resultStr2 = resultDec2.toString();
   ASSERT_STREQ( "9", resultStr2.c_str() );   
}

TEST( decimal, zero )
{   
   INT32 rc = SDB_OK;
   FLOAT64 left = 9.5;
          
   //construnct decimal
   bsonDecimal leftDec;     
   leftDec.fromDouble( left );  
   
   //set zero
   leftDec.setZero();
   string resultStr;
   resultStr = leftDec.toString();
   ASSERT_STREQ( "0", resultStr.c_str() );  
   
   //is zero?
   BOOLEAN isZero = leftDec.isZero();
   ASSERT_TRUE( isZero );   
}

TEST( decimal, min )
{   
   INT32 rc = SDB_OK;
   FLOAT64 left = 9.5;

   //construnct decimal
   bsonDecimal leftDec;     
   leftDec.fromDouble( left ); 
   
   //is min?
   BOOLEAN isMin = leftDec.isMin();
   ASSERT_FALSE( isMin ); 
                 
   //set min
   leftDec.setMin();
   isMin = leftDec.isMin();
   ASSERT_TRUE( isMin );  
}

TEST( decimal, max )
{   
   INT32 rc = SDB_OK;
   FLOAT64 left = 9.5;

   //construnct decimal
   bsonDecimal leftDec;     
   leftDec.fromDouble( left ); 
   
   //is min?
   BOOLEAN isMax = leftDec.isMax();
   ASSERT_FALSE( isMax ); 
                 
   //set min
   leftDec.setMax();
   isMax = leftDec.isMax();
   ASSERT_TRUE( isMax );  
}
