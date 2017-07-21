/************************************************************
* @Description: test case for Jira questionaire
*				SEQUOIADBMAINSTREAM-1068
* @Modify:      Liang xuewang Init
*				2017-07-20
*************************************************************/
#include <client.hpp>
#include <gtest/gtest.h>
#include <iostream>
#include "testcommon.hpp" 

using namespace sdbclient ;
using namespace bson ;
using namespace std ;

TEST( bson, date )
{
	long long mills[] = {
		-62167248352000,   // 0000-01-01 00:00:00
        253402271999000,   // 9999-12-31 23:59:59
        -62167248353000,   // < 0000-01-01 00:00:00 
        253402272000000,   // > 9999-12-31 23:59:59
		-9223372036854775808,  // -2^63
		9223372036854775807    // 2^63-1
	} ;
	int size = sizeof(mills) / sizeof(mills[0]) ;
	int i ;
	BSONObjBuilder builder ;
	for( i = 0;i < size;i++ )
	{
		char key[10] ;
		sprintf( key, "%s%d", "date", i ) ;
		Date_t dt( mills[i] ) ;
		builder.appendDate( key, dt ) ;
	}
	BSONObj obj = builder.obj() ;
	const char* expect = "{ \"date0\": {\"$date\": \"0000-01-01\"}, "
						   "\"date1\": {\"$date\": \"9999-12-31\"}, "
						   "\"date2\": { \"$date\": -62167248353000 }, "
						   "\"date3\": { \"$date\": 253402272000000 }, "
						   "\"date4\": { \"$date\": -9223372036854775808 }, "
						   "\"date5\": { \"$date\": 9223372036854775807 } }" ;
	string real = obj.toString() ;
	ASSERT_STREQ( expect, real.c_str() ) << "fail to check date" ;
}

TEST( bson, timestamp )
{
	int secs[] = {
       -2145945600,   	// 1902-01-01 00:00:00.000000
	   -1325491200,     // 1928-01-01 00:00:00.000000
        2145887999     	// 2037-12-31 23:59:59.999999
    } ;
	int micros[] = {
		0,  			// micro seconds for 1902-01-01 00:00:00.000000
		0,              // micro seconds for 1928-01-01 00:00:00.000000
		999999 			// micro seconds for 2037-12-31 23:59:59.999999
	} ;
	int size = sizeof(secs) / sizeof(secs[0]) ;
	int i ;
	BSONObjBuilder builder ;
	for( i = 0;i < size;i++ )
	{
		char key[10] ;
		sprintf( key, "%s%d", "time", i ) ;
        unsigned long long tmp = 0 ;
        memcpy( (char*)&tmp, &micros[i], sizeof(int)) ;
        memcpy( (char*)&tmp + 4, &secs[i], sizeof(int)) ;
		builder.appendTimestamp( key, tmp ) ;
	}
	BSONObj obj = builder.obj() ;
	string real = obj.toString() ;
	const char* expect = "{ \"time0\": {\"$timestamp\": \"1902-01-01-00.05.52.000000\"}, "
						   "\"time1\": {\"$timestamp\": \"1928-01-01-00.00.00.000000\"}, "
						   "\"time2\": {\"$timestamp\": \"2037-12-31-23.59.59.999999\"} }" ;
	ASSERT_STREQ( expect, real.c_str() ) << "fail to check timestamp" ;
}
