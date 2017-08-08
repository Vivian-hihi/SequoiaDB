/**************************************************************
* @Description: test case of Jira questionaire
*					 SEQUOIADBMAINSTREAM-2593
*				    list backup and remove backup with group ID
* @Modify     : Liang xuewang Init
*           	 2017-08-02
***************************************************************/
#include <gtest/gtest.h>
#include <client.hpp>
#include "../testcommon.hpp"

using namespace sdbclient ;
using namespace std ;
using namespace bson ;

TEST( backup, option )
{
	int rc = SDB_OK ;
	sdb db ;

	// connect to sdb 
   getConf() ;
   rc = db.connect( HOSTNAME, SVCNAME, USER, PASSWD ) ;
   ASSERT_EQ( rc, SDB_OK ) << "fail to connect sdb" ;
	if( isStandalone(db) )
	{
		cout << "Run mode is standalone" << endl ;
		db.disconnect() ;
		return ;
	}
	
	// get a data group id
	sdbCursor cursor ;
	int groupID ;
	BSONObj obj ;
   rc = db.listReplicaGroups( cursor ) ;
   ASSERT_EQ( rc, SDB_OK ) << "fail to list replica groups" ;
   while( !( cursor.next( obj ) ) )
   {
      string groupname = obj.getField( "GroupName" ).String() ;
      if( groupname != "SYSCoord" && groupname != "SYSCatalogGroup" )
      {
			groupID = obj.getField( "GroupID" ).Int() ;
			break ;
      }
   }
	rc = cursor.close() ;
	ASSERT_EQ( rc, SDB_OK ) << "fail to close cursor of list rg" ;
	cout << "backup group id: " << groupID << endl ;

	// backup offline with groupID
	BSONObj option = BSON( "GroupID" << groupID ) ; 
	rc = db.backupOffline( option ) ;
	ASSERT_EQ( rc, SDB_OK ) << "fail to backup offline" ;
	
	// list backup 
	rc = db.listBackup( cursor, option ) ;
	ASSERT_EQ( rc, SDB_OK ) << "fail to list backup" ;
	rc = cursor.close() ;
	ASSERT_EQ( rc, SDB_OK ) << "fail to close cursor of list backup" ;

	// remove backup
	rc = db.removeBackup( option ) ;
	ASSERT_EQ( rc, SDB_OK ) << "fail to remove backup" ;

	// disconnect
	db.disconnect() ;
}
