/**************************************************************************
 * @Description:   test case for C driver
 *                 seqDB-18581: insert 重复键时覆盖记录
 * @Modify:        liuxiaoxuan Init
 *                 2019-06-18
 **************************************************************************/
#include <gtest/gtest.h>
#include <client.h>
#include <stdio.h>
#include "testcommon.hpp"
#include "testBase.hpp"
#include "arguments.hpp"

class insertReplaceOnDup18581 : public testBase
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
      csName = "insertReplaceOnDup18581Cs" ;
      clName = "insertReplaceOnDup18581Cl" ;
      rc = sdbCreateCollectionSpace( db, csName, SDB_PAGESIZE_4K, &cs ) ;
      ASSERT_EQ( SDB_OK, rc ) << "fail to create cs " << csName ;
      rc = sdbCreateCollection( cs, clName, &cl ) ;
      ASSERT_EQ( SDB_OK, rc ) << "fail to create cl " << clName ;
   }

   void TearDown()
   {
      if( shouldClear() )
      {
         INT32 rc = sdbDropCollectionSpace( db, csName ) ;
         ASSERT_EQ( SDB_OK, rc ) << "fail to drop cs " << csName ;
      } 
      sdbReleaseCS( cs ) ;
      sdbReleaseCollection( cl ) ;
      testBase::TearDown() ;
   }
} ;

TEST_F( insertReplaceOnDup18581, uniqueIndex )
{
   INT32 rc = SDB_OK ;

   bson idxDef ;
   bson_init( &idxDef ) ;
   bson_append_int( &idxDef, "a", 1 ) ;
   bson_finish( &idxDef ) ;
   rc = sdbCreateIndex( cl, &idxDef, "aIndex", true, true ) ;
   bson_destroy( &idxDef ) ;
   ASSERT_EQ( SDB_OK, rc ) << "fail to create index" ;
   
   bson doc ;
   bson ret ;
   bson_init( &doc ) ;
   bson_init( &ret );
   bson_append_int( &doc, "a", 1 ) ;
   bson_append_string( &doc, "b", "b1" ) ;
   bson_finish( &doc ) ;
   rc = sdbInsert2( cl, &doc, FLG_INSERT_REPLACEONDUP, &ret ) ;
   ASSERT_EQ( SDB_OK, rc ) << "fail to insert" ; 
   bson_destroy( &doc ) ;
   bson_destroy( &ret );
   bson_init( &doc ) ;
   bson_init( &ret );
   bson_append_int( &doc, "a", 1 ) ;
   bson_append_string( &doc, "b", "b2" ) ;
   bson_finish( &doc ) ;
   rc = sdbInsert2( cl, &doc, FLG_INSERT_REPLACEONDUP, &ret ) ;
   ASSERT_EQ( SDB_OK, rc ) << "fail to insert" ; 
   bson_destroy( &doc ) ;
   bson_destroy( &ret );
   
   // get the record num
   SINT64 count  = 0 ;
   rc = sdbGetCount ( cl, NULL, &count ) ;
   ASSERT_EQ( SDB_OK, rc ) << "fail to get count";
   ASSERT_EQ( 1, count ) ;

   // get query result
   bson cond ;
   bson_init( &cond ) ;
   bson_append_int( &cond, "a", 1 ) ;
   bson_finish( &cond ) ;
   sdbCursorHandle cursor ;
   rc = sdbQuery( cl, &cond, NULL, NULL, NULL, 0, -1, &cursor ) ;
   bson_destroy( &cond ) ;
   ASSERT_EQ( SDB_OK, rc ) << "fail to query" ;

   bson obj ;
   bson_init( &obj ) ;
   rc = sdbNext( cursor, &obj ) ;
   ASSERT_EQ( SDB_OK, rc ) << "fail to get next" ;
   bson_iterator it ;
   bson_find( &it, &obj, "b" ) ;
   ASSERT_STREQ( "b2", bson_iterator_string( &it ) ) ;
   bson_destroy( &obj ) ;
   rc = sdbCloseCursor( cursor ) ;
   ASSERT_EQ( SDB_OK, rc ) << "fail to close cursor" ;
}

TEST_F( insertReplaceOnDup18581, noIndex )
{
   INT32 rc = SDB_OK ;

   bson ret ;
   SINT64 NUM = 2;
   bson *objList [ NUM ] ;
   INT32 i ;
   bson_init( &ret );
   
   for ( i = 0; i < 2; i++ )
   {
      objList[i] = bson_create() ;
      rc = bson_append_int ( objList[i], "a", 1 ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      CHAR b[10] ;
      sprintf( b, "%s%d", "b", i);
      rc = bson_append_string ( objList[i], "b", b ) ;
      bson_finish ( objList[i] ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
   }
   rc = sdbBulkInsert2( cl, FLG_INSERT_REPLACEONDUP, objList, NUM, &ret ) ;
   ASSERT_EQ( SDB_OK, rc ) << "fail to insert" ; 
   bson_destroy( &ret );
      
   // get the record num
   SINT64 count  = 0 ;
   rc = sdbGetCount ( cl, NULL, &count ) ;
   ASSERT_EQ( SDB_OK, rc ) << "fail to get count";
   ASSERT_EQ( 2, count ) ;

   // get query result
   bson cond ;
   bson orderBy ;
   bson_init( &cond ) ;
   bson_init( &orderBy ) ;
   bson_append_int( &cond, "a", 1 ) ;
   bson_append_int( &orderBy, "_id", 1 ) ;
   bson_finish( &cond ) ;
   bson_finish( &orderBy ) ;
   sdbCursorHandle cursor ;
   rc = sdbQuery( cl, &cond, NULL, &orderBy, NULL, 0, -1, &cursor ) ;
   bson_destroy( &cond ) ;
   bson_destroy( &orderBy ) ;
   ASSERT_EQ( SDB_OK, rc ) << "fail to query" ;

   bson obj ;
   bson_init( &obj ) ;
   rc = sdbNext( cursor, &obj ) ;
   ASSERT_EQ( SDB_OK, rc ) << "fail to get next" ;
   bson_iterator it ;
   bson_find( &it, &obj, "a" ) ;
   ASSERT_EQ( 1, bson_iterator_int( &it ) ) ;
   bson_find( &it, &obj, "b" ) ;
   ASSERT_STREQ( "b0", bson_iterator_string( &it ) ) ;
   bson_destroy( &obj ) ;
   
   bson_init( &obj ) ;
   rc = sdbNext( cursor, &obj ) ;
   ASSERT_EQ( SDB_OK, rc ) << "fail to get next" ;
   bson_find( &it, &obj, "a" ) ;
   ASSERT_EQ( 1, bson_iterator_int( &it ) ) ;
   bson_find( &it, &obj, "b" ) ;
   ASSERT_STREQ( "b1", bson_iterator_string( &it ) ) ;
   bson_destroy( &obj ) ;
   rc = sdbCloseCursor( cursor ) ;
   ASSERT_EQ( SDB_OK, rc ) << "fail to close cursor" ;
}
