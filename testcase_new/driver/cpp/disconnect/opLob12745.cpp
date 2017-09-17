/**************************************************************
 * @Description: opreate lob object after disconnect
 *               seqDB-12745 : opreate lob object after disconnect
 * @Modify     : Suqiang Ling
 *               2017-09-11
 ***************************************************************/
#include <gtest/gtest.h>
#include <client.hpp>
#include <iostream>
#include <vector>
#include "testcommon.hpp"
#include "arguments.hpp"
#include "testBase.hpp"

#define BUF_LEN 128

using namespace sdbclient ;
using namespace bson ;
using namespace std ;

class opLob12745 : public testBase
{
protected:
   const char *pCsName ;
   const char *pClName ;
   sdbLob wLob ;
   sdbLob rLob ;

   void SetUp()
   {
      testBase::SetUp() ;

      pCsName = "opLob12745" ;
      pClName = "opLob12745" ;
      sdbCollectionSpace cs ;
      sdbCollection cl ;

      INT32 rc = SDB_OK ;
      rc = db.createCollectionSpace( pCsName, SDB_PAGESIZE_4K, cs ) ;
      ASSERT_EQ( SDB_OK, rc ) << "fail to create cs" ;
      rc = cs.createCollection( pClName, cl ) ;
      ASSERT_EQ( SDB_OK, rc ) << "fail to create cl" ;
      rc = cl.createLob( wLob ) ;
      ASSERT_EQ( SDB_OK, rc ) << "fail to create lob" ;
      db.disconnect() ;
   }

   void TearDown()
   {
      if( shouldClear() )
      {
         INT32 rc = SDB_OK ;
         rc = db.connect( ARGS->hostName(), ARGS->svcName(), ARGS->user(), ARGS->passwd() ) ;
         ASSERT_EQ( SDB_OK, rc ) << "fail to connect db" ;
         rc = db.dropCollectionSpace( pCsName ) ;
         ASSERT_EQ( SDB_OK, rc ) << "fail to drop cs" ;
      }
      testBase::TearDown() ;
   }
} ;

TEST_F( opLob12745, opLob )
{
   // test all interfaces of class sdbLob except close(), isClosed(), getOid(), getSize(), getCreateTime()
   // in the order of c++ api doc

   INT32 rc = SDB_OK ;
   CHAR buf[BUF_LEN] ;
   UINT32 read ;
   //rc = lob.read( BUF_LEN, buf, &read ) ; // TODO: rc = -6, not reasonable
   //EXPECT_EQ( SDB_NOT_CONNECTED, rc ) << "read lob shouldn't succeed" ;
   rc = wLob.write( buf, BUF_LEN ) ;
   EXPECT_EQ( SDB_NOT_CONNECTED, rc ) << "write lob shouldn't succeed" ;
   //rc = lob.seek( 0, SDB_LOB_SEEK_CUR ) ; // TODO: no doc to know what SDB_LOB_SEEK is!// TODO: rc = -6, not reasonable
   //EXPECT_EQ( SDB_NOT_CONNECTED, rc ) << "seek lob shouldn't succeed" ;
}
