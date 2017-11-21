/**************************************************************************
 * @Description:   seqDB-13416: 获取lob修改时间
 * @Modify:        Suqiang Ling
 *                 2017-11-17
 **************************************************************************/
#include <gtest/gtest.h>
#include <client.hpp>
#include <iostream>
#include "testcommon.hpp"
#include "testBase.hpp"
#include "arguments.hpp"

using namespace sdbclient ;
using namespace bson ;
using namespace std ;

class getLobModTime13416 : public testBase
{
protected:
   const CHAR *pCsName ;
   const CHAR *pClName ;
   sdbCollectionSpace cs ;
   sdbCollection cl ;
   
   void SetUp()
   {
      testBase::SetUp() ;

      INT32 rc = SDB_OK ;
      pCsName = "getLobModTime13416" ;
      pClName = "getLobModTime13416" ;

      // create cs, cl
      rc = db.createCollectionSpace( pCsName, SDB_PAGESIZE_4K, cs ) ;
      ASSERT_EQ( SDB_OK, rc ) << "fail to create cs" ;
      rc = cs.createCollection( pClName, cl ) ;
      ASSERT_EQ( SDB_OK, rc ) << "fail to create cl" ;
   }

   void TearDown()
   {
      if( shouldClear() )
      {
         INT32 rc = db.dropCollectionSpace( pCsName ) ;
         ASSERT_EQ( SDB_OK, rc ) << "fail to drop cs " << pCsName ;
      } 
      testBase::TearDown() ;
   }
} ;

TEST_F( getLobModTime13416, lobWrite )
{
   INT32 rc = SDB_OK ;
   sdbLob lob ;
   OID oid ;
   UINT64 createTime ;
   UINT64 modificationTime1 ;
   UINT64 modificationTime2 ;
   UINT64 modificationTime3 ;
   const CHAR *writeBuf = "0123456789ABCDEabcde" ;
   const INT32 wBufSize = strlen( writeBuf ) ;
   const UINT32 rBufSize = wBufSize ;
   CHAR readBuf[ rBufSize ] ;
   UINT32 read ;

   // create lob
   rc = cl.createLob( lob ) ;
   ASSERT_EQ( SDB_OK, rc ) << "fail to create lob" ;
   rc = lob.close() ;
   ASSERT_EQ( SDB_OK, rc ) << "fail to close lob" ;
   createTime = lob.getCreateTime() ;
   modificationTime1 = lob.getModificationTime() ;
   ASSERT_EQ( createTime, modificationTime1 ) 
         << "wrong modification time after lob creation" ;

   // modify lob
   rc = lob.getOid( oid ) ;
   ASSERT_EQ( SDB_OK, rc ) << "fail to get oid" ;
   rc = cl.openLob( lob, oid, SDB_LOB_WRITE ) ;
   ASSERT_EQ( SDB_OK, rc ) << "fail to open lob" ;
   rc = lob.write( writeBuf, wBufSize ) ;
   ASSERT_EQ( SDB_OK, rc ) << "fail to write lob" ;
   modificationTime2 = lob.getModificationTime() ;
   rc = lob.close() ;
   ASSERT_EQ( SDB_OK, rc ) << "fail to close lob" ;
   ASSERT_LT( modificationTime1, modificationTime2 ) 
         << "wrong modification time after modifying" ;

   // not modify lob
   // TODO: open and read lob can also update modification time. is it reasonable?
   //rc = cl.openLob( lob, oid, SDB_LOB_READ ) ;
   //ASSERT_EQ( SDB_OK, rc ) << "fail to open lob" ;
   //rc = lob.read( rBufSize, readBuf, &read ) ;
   //ASSERT_EQ( rBufSize, read ) << "long read length is wrong" ;
   //ASSERT_EQ( SDB_OK, rc ) << "fail to read lob" ;
   //rc = lob.close() ;
   //ASSERT_EQ( SDB_OK, rc ) << "fail to close lob" ;
   modificationTime3 = lob.getModificationTime() ;
   ASSERT_EQ( modificationTime2, modificationTime3 ) 
         << "wrong modification time when no modifying" ;
}
