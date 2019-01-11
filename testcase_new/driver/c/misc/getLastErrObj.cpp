/************************************************************

 * @Description: test case for Jira questionaire 
 *				     SEQUOIADBMAINSTREAM-3920
 * @author:      liuxiaoxuan
 *				     2019-01-04
 *************************************************************/
#include <gtest/gtest.h>
#include <client.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "testcommon.hpp"
#include "testBase.hpp"
#include "arguments.hpp"
#include <string>

class getLastErrorObjTest17055 : public testBase
{
protected:
   sdbCSHandle cs ;
   sdbCollectionHandle cl ;
   sdbCollectionHandle cl2 ;
   const CHAR* csName ;
   const CHAR* clName ;

   void SetUp() 
   {
      INT32 rc = SDB_OK ;
      cs = SDB_INVALID_HANDLE ;
      cl = SDB_INVALID_HANDLE ;
      cl2 = SDB_INVALID_HANDLE ;
      csName = "csname_17055";
      clName = "clname_17055" ;
      testBase::SetUp() ;
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
      sdbReleaseCollection( cl2 ) ;
      testBase::TearDown() ; 
   }  
} ;

TEST_F( getLastErrorObjTest17055, returnErrorObj )
{
   INT32 rc = SDB_OK ;

   rc = sdbCreateCollectionSpace( db, csName, SDB_PAGESIZE_4K, &cs ) ;
   ASSERT_EQ( SDB_OK, rc ) << "fail to create cs " << csName ;
   rc = sdbCreateCollection( cs, clName, &cl ) ;
   ASSERT_EQ( SDB_OK, rc ) << "fail to create cl " << clName ;
 
   // create existed cl
   rc = sdbCreateCollection( cs, clName, &cl2 ) ;
   ASSERT_EQ( -22, rc ) << "fail to create cl " << clName ;
   
   // get last errorobj from catalog
   bson errObj ;
   bson_init( &errObj ) ;
   rc = sdbGetLastErrorObj( db, &errObj ) ; 
   ASSERT_EQ( SDB_OK, rc ) << "fail to get last error obj " << rc ;
   bson_iterator it, sub ;
   bson_find( &it, &errObj, "ErrNodes" ) ;
   bson_iterator_subiterator( &it, &sub ) ;

   while( bson_iterator_more( &sub ) )
   {
      bson errNodesObj ;
      bson_init( &errNodesObj ) ;
      bson_iterator_subobject( &sub, &errNodesObj ) ;
      bson_iterator i1 ;
      bson_find( &i1, &errNodesObj, "ErrInfo" ) ;
      bson errInfoObj ;
      bson_init( &errInfoObj ) ;
      bson_iterator_subobject( &i1, &errInfoObj ) ;
       
      bson_iterator i2 ;
      bson_find( &i2, &errInfoObj, "errno" ) ;
      INT32 errno ;
      errno = bson_iterator_int( &i2 ) ;
      ASSERT_EQ( -22, errno ) << "errno: " << errno ;      
      
      bson_destroy( &errNodesObj ) ;
      bson_destroy( &errInfoObj ) ;
      bson_iterator_next( &sub ) ;
  }
  bson_destroy( &errObj );

  // clean error obj
  sdbCleanLastErrorObj( db );

  // get null last error obj 
  bson_init( &errObj ) ;
  rc = sdbGetLastErrorObj(db, &errObj) ;
  ASSERT_EQ( SDB_DMS_EOC, rc ) << "fail to get last error obj " << rc ;
  bson_destroy( &errObj );

  // update with invalid param
  bson rule;
  bson_init ( &rule ) ;
  bson_append_start_object( &rule, "$est" ) ;
  bson_append_int ( &rule, "a", 1 ) ;
  bson_append_finish_object( &rule ) ;
  bson_finish ( &rule ) ;
  rc = sdbUpdate( cl, &rule, NULL, NULL ) ;
  ASSERT_EQ( -6, rc ) << "fail to update" ;
  bson_destroy( &rule );
 
  // get last errorobj from data
  bson_init( &errObj ) ;
  rc = sdbGetLastErrorObj(db, &errObj) ;
  ASSERT_EQ( SDB_OK, rc ) << "fail to get last error obj " << rc ;
  bson_find( &it, &errObj, "ErrNodes" ) ;
  bson_iterator_subiterator( &it, &sub ) ;

  while( bson_iterator_more( &sub ) )
  {
     bson errNodesObj ;
     bson_init( &errNodesObj ) ;
     bson_iterator_subobject( &sub, &errNodesObj ) ;
     bson_iterator i1 ;
     bson_find( &i1, &errNodesObj, "ErrInfo" ) ;
     bson errInfoObj ;
     bson_init( &errInfoObj ) ;
     bson_iterator_subobject( &i1, &errInfoObj ) ;

     bson_iterator i2 ;
     bson_find( &i2, &errInfoObj, "errno" ) ;
     INT32 errno ;
     errno = bson_iterator_int( &i2 ) ;
     ASSERT_EQ( -6, errno ) << "errno: " << errno ;

     bson_destroy( &errNodesObj ) ;
     bson_destroy( &errInfoObj ) ;
     bson_iterator_next( &sub ) ;
  }
  bson_destroy( &errObj ); 

  // clean error obj
  sdbCleanLastErrorObj( db );

  // get null last error obj
  bson_init( &errObj ) ;
  rc = sdbGetLastErrorObj(db, &errObj) ;
  ASSERT_EQ( SDB_DMS_EOC, rc ) << "fail to get last error obj " << rc ;
  bson_destroy( &errObj );
}

TEST_F( getLastErrorObjTest17055, errorObjNULL )
{
   INT32 rc = SDB_OK ;

   rc = sdbCreateCollectionSpace( db, csName, SDB_PAGESIZE_4K, &cs ) ;
   ASSERT_EQ( SDB_OK, rc ) << "fail to create cs " << csName ;
   rc = sdbCreateCollection( cs, clName, &cl ) ;
   ASSERT_EQ( SDB_OK, rc ) << "fail to create cl " << clName ;

   // insert record
   bson doc ;
   bson_init( &doc ) ;
   bson_append_int( &doc, "a", 1 ) ;
   bson_finish( &doc ) ;
   rc = sdbInsert( cl, &doc ) ;
   bson_destroy( &doc ) ;
   ASSERT_EQ( SDB_OK, rc ) << "fail to insert doc" ;  

   // get last error
   bson errObj;
   bson_init( &errObj ) ;
   rc = sdbGetLastErrorObj(db, &errObj) ;
   ASSERT_EQ( SDB_DMS_EOC, rc ) << "fail to get last error obj " << rc ;
   bson_destroy( &errObj );
}
