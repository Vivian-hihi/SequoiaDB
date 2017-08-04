/**************************************************************
* @Description: test case of Jira questionaire
*					 SEQUOIADBMAINSTREAM-2593
*				    list backup and remove backup with group ID
* @Modify     : Liang xuewang Init
*           	 2017-08-02
***************************************************************/
#include <gtest/gtest.h>
#include <client.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../common/testcommon.hpp"

TEST( backup, option )
{
	int rc = SDB_OK ;
	sdbConnectionHandle db = SDB_INVALID_HANDLE ;

	// connect to sdb 
   getConf() ;
   rc = sdbConnect( HOSTNAME, SVCNAME, USER, PASSWD, &db ) ;
   ASSERT_EQ( rc, SDB_OK ) << "fail to connect sdb" ;
	
	// get a data group id
	sdbCursorHandle cursor = SDB_INVALID_HANDLE ;
	int groupID ;
	bson obj ;
   bson_init( &obj ) ;
   rc = sdbListReplicaGroups( db, &cursor ) ;
   ASSERT_EQ( rc, SDB_OK ) << "fail to list replica groups" ;
   while( !sdbNext( cursor, &obj ) )
   {
		bson_iterator it ;
      bson_find( &it, &obj, "GroupName" ) ;
      string groupname = bson_iterator_string( &it ) ;
      if( groupname != "SYSCoord" && groupname != "SYSCatalogGroup" )
      {
      	bson_find( &it, &obj, "GroupID" ) ;
			groupID = bson_iterator_int( &it ) ;
			break ;
      }
      bson_destroy( &obj ) ;
      bson_init( &obj ) ;
   }
	bson_destroy( &obj ) ;
	rc = sdbCloseCursor( cursor ) ;
	ASSERT_EQ( rc, SDB_OK ) << "fail to close cursor of list rg" ;
	printf( "backup group id: %d\n", groupID ) ;

	// backup offline with groupID
	bson option ;
	bson_init( &option ) ;
	bson_append_int( &option, "GroupID", groupID ) ;
	bson_finish( &option ) ;
	rc = sdbBackupOffline( db, &option ) ;
	ASSERT_EQ( rc, SDB_OK ) << "fail to backup offline" ;
	
	// list backup 
	rc = sdbListBackup( db, &option, NULL, NULL, NULL, &cursor ) ;
	ASSERT_EQ( rc, SDB_OK ) << "fail to list backup" ;
	rc = sdbCloseCursor( cursor ) ;
	ASSERT_EQ( rc, SDB_OK ) << "fail to close cursor of list backup" ;

	// remove backup
	rc = sdbRemoveBackup( db, &option ) ;
	ASSERT_EQ( rc, SDB_OK ) << "fail to remove backup" ;

	// disconnect
	sdbDisconnect( db ) ;
	sdbReleaseCursor( cursor ) ;
	sdbReleaseConnection( db ) ;
}
