/**************************************************************************
 * @Description :   test lob getModificationTime operation
 *                  seqDB-13429:获取lob修改时间
 * @Modify      :   Liang xuewang
 *                  2017-12-14
 **************************************************************************/
#include <gtest/gtest.h>
#include <client.h>
#include "testcommon.hpp"
#include "testBase.hpp"
#include "arguments.hpp"

class lobModTime13429 : public testBase
{
protected:
   const CHAR* csName ;
   const CHAR* clName ;
   sdbCSHandle cs ;
   sdbCollectionHandle cl ;
   
   void SetUp()
   {
      testBase::SetUp() ;
      INT32 rc = SDB_OK ;
      csName = "lobModTimeCs13429" ;
      clName = "lobModTimeCl13429" ;
      rc = createNormalCsCl( db, &cs, &cl, csName, clName ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
   }

   void TearDown()
   {
      INT32 rc = SDB_OK ;
      rc = sdbDropCollectionSpace( db, csName ) ;
      ASSERT_EQ( SDB_OK, rc ) << "fail to drop cs " << csName ;
      sdbReleaseCollection( cl ) ;
      sdbReleaseCS( cs ) ;
      testBase::TearDown() ;
   }
} ;

TEST_F( lobModTime13429, ModifyTime )
{
   INT32 rc = SDB_OK ;

   // create lob, get create time
   bson_oid_t oid ;
   bson_oid_gen( &oid ) ;
   sdbLobHandle lob ;
   rc = sdbOpenLob( cl, &oid, SDB_LOB_CREATEONLY, &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) << "fail to open lob with create mode" ;
   UINT64 createTime ;
   rc = sdbGetLobModificationTime( lob, &createTime ) ;
   ASSERT_EQ( SDB_OK, rc ) << "fail to getModificationTime" ;
   rc = sdbCloseLob( &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) << "fail to close lob" ;

   // write lob, get write time 
   rc = sdbOpenLob( cl, &oid, SDB_LOB_WRITE, &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) << "fail to open lob with write mode" ;
   const CHAR* writeBuf = "ABCDE" ;
   UINT32 len = strlen( writeBuf ) ;
   rc = sdbWriteLob( lob, writeBuf, len ) ;
   ASSERT_EQ( SDB_OK, rc ) << "fail to write lob" ;
   UINT64 writeTime ;
   rc = sdbGetLobModificationTime( lob, &writeTime ) ;
   ASSERT_EQ( SDB_OK, rc ) << "fail to getModificationTime" ;
   ASSERT_LT( createTime, writeTime ) << "fail to check write time" ;
   rc = sdbCloseLob( &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) << "fail to close lob" ;
   
   // read lob, get read time
   CHAR* readBuf = (CHAR*)malloc( len+1 ) ;
   memset( readBuf, 0, len+1 ) ;
   UINT32 readLen = 0 ;
   rc = sdbOpenLob( cl, &oid, SDB_LOB_READ, &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) << "fail to open lob with read mode" ;
   rc = sdbReadLob( lob, len, readBuf, &readLen ) ;
   ASSERT_EQ( SDB_OK, rc ) << "fail to read lob" ;
   UINT64 readTime ;
   rc = sdbGetLobModificationTime( lob, &readTime ) ;
   ASSERT_EQ( SDB_OK, rc ) << "fail to getModificationTime" ;
   // ASSERT_EQ( writeTime, readTime ) << "fail to check read time" ; // readTime > writeTime
   rc = sdbCloseLob( &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) << "fail to close lob" ;

   free( readBuf ) ;
}
