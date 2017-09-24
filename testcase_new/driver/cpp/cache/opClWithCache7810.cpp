/**************************************************************
 * @Description: turn on cache and operate cl
 *               seqDB-7810 : turn on cache and operate cl to 
 *                            update timestamp
 * @Modify     : Suqiang Ling
 *               2017-09-11
 ***************************************************************/
#include <gtest/gtest.h>
#include <client.hpp>
#include <iostream>
#include <vector>
#include <sys/time.h>
#include "testcommon.hpp"
#include "arguments.hpp"
#include "testBase.hpp"
#include "client.hpp"

using namespace sdbclient ;
using namespace bson ;
using namespace std ;

class turnOnCache7810 : public testBase 
{
protected:
   const CHAR *pCsName ;
   const CHAR *pClName ;
   sdbCollectionSpace cs ;
   sdbCollection cl ;

   void SetUp() 
   {
      INT32 rc = SDB_OK ;

      // init client
      sdbClientConf conf ;
      conf.enableCacheStrategy = TRUE ;
      conf.cacheTimeInterval = 0 ;
      rc = initClient( &conf );
      ASSERT_EQ( SDB_OK, rc ) << "fail to initClient" ;

      // connect and create cs, cl
      pCsName = "turnOnCache7810" ;
      pClName = "turnOnCache7810" ;
      rc = db.connect( ARGS->hostName(), ARGS->svcName(), ARGS->user(), ARGS->passwd() );
      ASSERT_EQ( SDB_OK, rc ) << "fail to connect db" ;
      rc = db.createCollectionSpace( pCsName, SDB_PAGESIZE_4K, cs ) ;
      ASSERT_EQ( SDB_OK, rc ) << "fail to create cs" ;
      rc = cs.createCollection( pClName, cl ) ;
      ASSERT_EQ( SDB_OK, rc ) << "fail to create cl" ;
   }

   void TearDown() 
   {
      if( shouldClear() )
      {
         INT32 rc = SDB_OK ;
         rc = db.dropCollectionSpace( pCsName ) ;
         ASSERT_EQ( SDB_OK, rc ) << "fail to drop cs" ;
      }
      db.disconnect() ;
   }

   INT32 getElapesdTimeOfGetCl( clock_t &elapsedUsec )
   {
      struct timeval begin, end ;
      sdbCollection cl ;
      INT32 rc ;

      gettimeofday( &begin, NULL ) ;
      rc = cs.getCollection( pClName, cl ) ;
      gettimeofday( &end, NULL ) ;
      
      elapsedUsec = ( end.tv_sec - begin.tv_sec ) * 1000000  + end.tv_usec - begin.tv_usec ;

      return rc ;
   }
} ;

TEST_F( turnOnCache7810, testUpdateTimeStamp )
{
   INT32 rc = SDB_OK ;
   clock_t outCacheTime, inCacheTime ;
   rc = getElapesdTimeOfGetCl( outCacheTime ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   //ossSleep( 600 ) ;
   BSONObj doc = BSON( "a" << 1 ) ;
   rc = cl.insert( doc ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   //ossSleep( 600 ) ;
   // if cl would be keep for 1s, 0.6 + 0.6s can make cl out the cache.
   // TODO: i don't know how long it would be keep.
   rc = getElapesdTimeOfGetCl( inCacheTime ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   ASSERT_LT( inCacheTime, outCacheTime ) ;
}
