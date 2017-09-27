/**************************************************************
 * @Description: test case for Jira questionaire Task
 *					  SEQUOIADBMAINSTREAM-2165
 *					  seqDB-11001:reloadConf
 *               修改数据组备节点的配置文件weight=20
 *					  重新选主，检查备节点升级为主节点(may not be)
 * @Modify     : Liang xuewang Init
 *			 		  2017-01-22
 ***************************************************************/
#include <gtest/gtest.h>
#include <client.h>
#include <stdio.h>
#include "testcommon.hpp"
#include "impWorker.hpp"
#include "arguments.hpp"

INT32 getInstallPath( CHAR* path )
{
   INT32 rc = 0 ;
   const CHAR* installFile = "/etc/default/sequoiadb" ;
   FILE* fp = fopen( installFile, "r" ) ;
   CHAR s[50] ;
   INT32 len ;
   if( fp == NULL )
   {
      printf( "fail to open file /etc/default/sequoiadb\n" ) ;
      goto error ;
   }
   while( fgets(s,sizeof(s),fp) != NULL  )
   {
      CHAR* idx ;
      if( ( idx = strstr( s, "INSTALL_DIR=" ) ) == NULL )
         continue ;
      strcpy( path, idx + 12 ) ;
      break ;
   }
   fclose( fp ) ;
   len = strlen( path ) ;
   path[ len-1 ] = '\0' ;
   if( strcmp( path, "" ) == 0 )
   {
      printf( "fail to get install path\n" ) ;
      goto error ;
   }
done:
   return rc ;
error:
   rc = 1 ;
   goto done ;
}

INT32 isMasterNode( sdbReplicaGroupHandle rg, const CHAR* host, const CHAR* svc, BOOLEAN* res )
{
   INT32 rc = SDB_OK ;
   sdbNodeHandle master ;
   const CHAR *host1, *svc1, *nodename1 ;
   INT32 nodeId1 ;

   rc = sdbGetNodeMaster( rg, &master ) ;
   while( rc == SDB_CLS_NODE_NOT_EXIST )
   {
      ossSleep( 1000 ) ;
      rc = sdbGetNodeMaster( rg, &master ) ;
   }
   CHECK_RC( SDB_OK, rc, "fail to get master node" ) ;
   rc = sdbGetNodeAddr( master, &host1, &svc1, &nodename1, &nodeId1 ) ;
   CHECK_RC( SDB_OK, rc, "fail to get master node addr" ) ;
   printf( "master node: %s:%s\n", host1, svc1 ) ;

   if( strcmp(host, host1) == 0 && strcmp(svc, svc1) == 0 )
      *res = TRUE ;
   else
      *res = FALSE ;

done:
   sdbReleaseNode( master ) ;
   return rc ;
error:
   goto done ;
}

// get a slave data node which is on the same machine with coord
INT32 createSlaveNode( sdbConnectionHandle db, sdbReplicaGroupHandle rg, sdbNodeHandle node, 
                       const CHAR** host, const CHAR** svc, const CHAR** nodeName, INT32* nodeId )
{
   INT32 rc = SDB_OK ;
   sdbCursorHandle cursor = SDB_INVALID_HANDLE ;
   bson obj ;
   bson_init( &obj ) ;
   CHAR dbPath[50] ;
   vector<string> groups ;

   rc = getGroups( db, groups ) ;
   CHECK_RC( SDB_OK, rc, "fail to get data groups" ) ;
   for( INT32 i = 0;i < groups.size();i++ )
   {
      const CHAR* rgName = groups[i].c_str() ;
      vector<string> nodes ;
      rc = getGroupNodes( db, rgName, nodes ) ;
      CHECK_RC( SDB_OK, rc, "fail to get rg nodes" ) ;
      // if rg has only one node, after reelect and change primary node to new add node, 
      // then stop the primary node, group can't make reelect
      if( nodes.size() == 1 )  continue ;

      rc = sdbGetReplicaGroup( db, rgName, &rg ) ;
      CHECK_RC( SDB_OK, rc, "fail to get rg %s", rgName ) ;
      break ;
   }

   CHAR hostName[100] ;
   rc = getLocalHost( hostName, 100 ) ;
   CHECK_RC( SDB_OK, rc, "fail to get local hostName" ) ;
   sprintf( dbPath, "%s%s%s", ARGS->rsrvNodeDir(), "data/", ARGS->rsrvPortBegin() ) ;
   rc = sdbCreateNode( rg, hostName, ARGS->rsrvPortBegin(), dbPath, NULL ) ;
   CHECK_RC( SDB_OK, rc, "fail to create node %s:%s dbpath: %s", hostName, ARGS->rsrvPortBegin(), dbPath ) ;
   rc = sdbGetNodeByHost( rg, hostName, ARGS->rsrvPortBegin(), &node ) ;
   CHECK_RC( SDB_OK, rc, "fail to get node %s:%s", hostName, ARGS->rsrvPortBegin() ) ;
   rc = sdbGetNodeAddr( node, host, svc, nodeName, nodeId ) ;
   CHECK_RC( SDB_OK, rc, "fail to get node addr" ) ;	

done:
   bson_destroy( &obj ) ;
   sdbReleaseCursor( cursor ) ;
   return rc ;
error:
   goto done ;
}

INT32 getLSN( sdbConnectionHandle db, SINT64* offset, INT32* version )
{
   INT32 rc = SDB_OK ;
   sdbCursorHandle cursor = SDB_INVALID_HANDLE ;
   bson sel, obj ;
   bson_init( &sel ) ;
   bson_init( &obj ) ;
   bson_iterator it, sub_it ;

   bson_append_string( &sel, "CurrentLSN", "" ) ;
   bson_finish( &sel ) ;
   rc = sdbGetSnapshot( db, SDB_SNAP_DATABASE, NULL, &sel, NULL, &cursor ) ;
   CHECK_RC( SDB_OK, rc, "fail to get snapshot database" ) ;

   rc = sdbNext( cursor, &obj ) ;
   CHECK_RC( SDB_OK, rc, "fail to get next" ) ;

   bson_find( &it, &obj, "CurrentLSN" ) ;
   bson_iterator_subiterator( &it, &sub_it ) ;
   bson_iterator_next( &sub_it ) ;
   *offset = bson_iterator_long( &sub_it ) ;
   bson_iterator_next( &sub_it ) ;
   *version = bson_iterator_int( &sub_it ) ;

done:
   bson_destroy( &sel ) ;
   bson_destroy( &obj ) ;
   sdbReleaseCursor( cursor ) ;
   return rc ;
error:
   goto done ;
} 

// wait sync finish, lsn is equal
INT32 waitSync( sdbReplicaGroupHandle rg, const CHAR* host, const CHAR* svc )
{
   INT32 rc = SDB_OK ;
   sdbConnectionHandle db, db1 ;
   sdbNodeHandle master ;
   const CHAR *host1, *svc1, *nodename1 ;
   INT32 nodeId1 ;
   SINT64 offset, offset1 ;                                                 
   INT32 version, version1 ;

   rc = sdbGetNodeMaster( rg, &master ) ;
   CHECK_RC( SDB_OK, rc, "fail to get master node" ) ;
   rc = sdbGetNodeAddr( master, &host1, &svc1, &nodename1, &nodeId1 ) ;
   CHECK_RC( SDB_OK, rc, "fail to get master node addr" ) ;

   rc = sdbConnect( host, svc, ARGS->user(), ARGS->passwd(), &db ) ;
   CHECK_RC( SDB_OK, rc, "fail to connect node %s:%s", host, svc ) ;
   rc = sdbConnect( host1, svc1, ARGS->user(), ARGS->passwd(), &db1 ) ;
   CHECK_RC( SDB_OK, rc, "fail to connect master node %s:%s", host1, svc1 ) ;

   do {
      ossSleep( 1000 ) ;
      rc = getLSN( db, &offset, &version ) ;
      CHECK_RC( SDB_OK, rc, "fail to get lsn of node" ) ;
      rc = getLSN( db1, &offset1, &version1 ) ;
      CHECK_RC( SDB_OK, rc, "fail to get lsn of master node" ) ;
   } while( offset != offset1 || version != version1 ) ;
   printf( "node offset: %lld, version: %d\n", offset, version ) ;
   printf( "master node offset: %lld, version: %d\n", offset1, version1 ) ;

done:
   sdbDisconnect( db ) ;
   sdbDisconnect( db1 ) ;
   sdbReleaseConnection( db ) ;
   sdbReleaseConnection( db1 ) ;
   return rc ;
error:
   goto done ;
}

INT32 changeNodeConf( const CHAR* svc, const CHAR* conf, INT32 value )
{
   INT32 rc = 0 ;

   CHAR installPath[20] = { 0 } ;
   CHAR confFile[100] = { 0 } ;
   FILE* fp = NULL ;
   CHAR buffer[100] = { 0 } ;
   sprintf( buffer, "%s%s%d", conf, "=", value ) ;
   CHAR s[100] ;
   INT32 len = 0 ;

   rc = getInstallPath( installPath ) ;
   CHECK_RC( 0, rc, "fail to get installPath" ) ;
   sprintf( confFile, "%s%s%s%s", installPath, "/conf/local/", svc, "/sdb.conf" ) ;
   fp = fopen( confFile, "r+" ) ;
   if( fp == NULL )
   {
      printf( "fail to open conf file: %s\n", confFile ) ;
      goto error ;
   }

   while( fgets( s, sizeof(s), fp ) != NULL )
   {
      len += strlen( s ) ;
      CHAR* idx ;
      if( ( idx = strstr( s, conf ) ) != NULL )
      {
         len -= strlen( s ) ;
         break ;
      }
   }
   if( fseek( fp, len, SEEK_SET ) != 0 )
   {
      printf( "fail to seek file,file: %s, offset: %d\n", confFile, len ) ;
      goto error ;
   }
   fprintf( fp, "%s", buffer ) ;
   fclose( fp ) ;

done:
   return rc ;
error:
   rc = 1 ;
   goto done ;
}

TEST( reloadConf, weight )
{
   INT32 rc = SDB_OK ;

   sdbConnectionHandle db = SDB_INVALID_HANDLE ;
   rc = sdbConnect( ARGS->hostName(), ARGS->svcName(), ARGS->user(), ARGS->passwd(), &db ) ;
   ASSERT_EQ( SDB_OK, rc ) << "fail to connect sdb" ;

   // create a slave node
   sdbReplicaGroupHandle rg = SDB_INVALID_HANDLE ;
   sdbNodeHandle node = SDB_INVALID_HANDLE ;
   const CHAR *host, *svc, *nodename ;
   INT32 nodeId ;
   rc = createSlaveNode( db, rg, node, &host, &svc, &nodename, &nodeId ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   printf( "node: name %s,svc %s,nodename %s,nodeId %d\n", host, svc, nodename, nodeId ) ;

   // start node and wait sync finish
   rc = sdbStartNode( node ) ;
   ASSERT_EQ( SDB_OK, rc ) << "fail to start node" ;
   rc = waitSync( rg, host, svc ) ;
   ASSERT_EQ( SDB_OK, rc ) ;

   // change slave node weight to 20
   rc = changeNodeConf( svc, "weight", 20 ) ;
   ASSERT_EQ( SDB_OK, rc ) ;

   // reload conf
   rc = sdbReloadConfig( db, NULL ) ;
   ASSERT_EQ( SDB_OK, rc ) << "fail to reload conf" ;

   // reelect and check master
   bson option ;
   bson_init( &option ) ;
   bson_append_int( &option, "Seconds", 60 ) ;
   bson_finish( &option ) ;
   rc = sdbReelect( rg, &option ) ;
   bson_destroy( &option ) ;
   ASSERT_EQ( SDB_OK, rc ) << "fail to reelect in rg" ;
   BOOLEAN isMaster = FALSE ;
   rc = isMasterNode( rg, host, svc, &isMaster ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   if( isMaster )
   {
      printf( "node %s:%s is master node.\n", host, svc ) ;
   }
   else
   {
      printf( "node %s:%s is not master node.\n", host, svc ) ;
   }
   // ASSERT_TRUE( isMaster ) << "fail to check node to be master after reelect" ;	

   // stop and remove node
   rc = sdbStopNode( node ) ;
   ASSERT_EQ( SDB_OK, rc ) << "fail to stop node" ;
   do
   {
      ossSleep( 1000 ) ;
      rc = isMasterNode( rg, host, svc, &isMaster ) ;
      ASSERT_EQ( SDB_OK, rc ) << "fail to check node is master node or not" ;
   } while( isMaster ) ;
   rc = sdbRemoveNode( rg, host, svc, NULL ) ;
   ASSERT_EQ( SDB_OK, rc ) << "fail to remove node" ;

   sdbDisconnect( db ) ;
   sdbReleaseConnection( db ) ;
   sdbReleaseReplicaGroup( rg ) ;
   sdbReleaseNode( node ) ;
}
