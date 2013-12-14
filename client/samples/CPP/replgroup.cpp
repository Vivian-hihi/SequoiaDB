/******************************************************************************
 *
 * Name: replgroup.cpp
 * Description: This program demostrates how to connect to SequoiaDB database,
 *              list/get replica set/node, and create new node
 * Parameters:
 *              HostName: The hostname for database server
 *              ServiceName: The service name or port number for the database
 *                           service
 *              Username: The user name for database server
 *              Password: The password  for user
 * Auto Compile:
 * Linux: ./buildApp.sh replgroup
 * Win: buildApp.bat replgroup
 * Manual Compile:
 * Linux: 
 *       g++ replgroup.cpp common.cpp -o replgroup -I../../include \
 *       -L../../lib -lsdbcpp
 * Win:
 *    cl /Foreplgroup.obj /c replgroup.cpp /I..\..\include /wd4047
 *    cl /Focommon.obj /c common.cpp /I..\..\include /wd4047
 *    link /OUT:replgroup.exe /LIBPATH:..\..\lib sdbcpp.lib replgroup.obj common.obj
 *    copy ..\..\lib\sdbcpp.dll .
 * Run:
 * Linux: LD_LIBRARY_PATH=<path for libsdbcpp.so> ./replgroup <hostname> \
 *        <servicename> <username> <password>
 * Win: replgroup.exe <hostname> <servicename> <username> <password>
 *
 ******************************************************************************/
#include <iostream>
#include <stdio.h>
#include "common.hpp"

#define COLLECTION_NAME "SAMPLE.employee"
#define NUM_RECORD 100
#define INDEX_NAME "employee_id"
/* Display Syntax Error */
void displaySyntax ( CHAR *pCommand )
{
   printf ( "Syntax: %s <hostname> <servicename>\
 <username> <password>" OSS_NEWLINE, pCommand ) ;
}

INT32 addGroup ( sdb *connection )
{
   INT32 rc = SDB_OK ;
   CHAR newGroupName [ 128 ] = {0} ;
   sdbReplicaGroup rg ;
   printf ( "Please input the new group name: " ) ;
   scanf ( "%s", newGroupName ) ;
   rc = connection->createReplicaGroup ( newGroupName, rg ) ;
   if ( rc )
   {
      printf ( "Failed to create group, rc = %d" OSS_NEWLINE, rc ) ;
      goto error ;
   }
done :
   return rc ;
error :
   goto done ;
}

INT32 listGroups ( sdb *connection )
{
   INT32 rc = SDB_OK ;
   sdbCursor cursor ;
   INT32 count = 0 ;
   BSONObj obj ;
   rc = connection->listReplicaGroups ( cursor ) ;
   if ( rc )
   {
      printf ( "Failed to list replicagroups, rc = %d"
               OSS_NEWLINE, rc ) ;
      exit ( 0 ) ;
   }

   while ( TRUE )
   {
      rc = cursor.next ( obj ) ;
      if ( rc )
      {
         if ( SDB_DMS_EOC != rc )
         {
            printf ( "Failed to fetch next record from collection, rc = %d"
                     OSS_NEWLINE, rc ) ;
         }
         break ;
      }
      printf ( "ReplicaGroup [ %d ]: " OSS_NEWLINE, count ) ;
      cout << obj.toString() << endl ;
      ++ count ;
   }
   rc = SDB_OK ;
done :
   return rc ;
error :
   goto done ;
}

INT32 main ( INT32 argc, CHAR **argv )
{
   /* initialize local variables */
   CHAR *pHostName                   = NULL ;
   CHAR *pServiceName                = NULL ;
   CHAR *pUser                       = NULL ;
   CHAR *pPasswd                     = NULL ;
   BSONObj detail ;
   sdb connection ;
   sdbReplicaGroup replicagroup ;
   sdbReplicaNode replicamasternode ;
   sdbReplicaNode replicaslavenode ;
   INT32 nodeNum = 0 ;
   INT32 rc                          = 0 ;
   /* verify syntax */
   if ( 5 != argc )
   {
      displaySyntax ( (CHAR*)argv[0] ) ;
      exit ( 0 ) ;
   }

   /* read argument */
   pHostName    = (CHAR*)argv[1] ;
   pServiceName = (CHAR*)argv[2] ;
   pUser        = (CHAR*)argv[3] ;
   pPasswd      = (CHAR*)argv[4] ;

   /* connect to database */
   rc = connectTo ( pHostName, pServiceName,
                    pUser, pPasswd, connection ) ;
   if ( rc )
   {
      printf ( "Failed to connect to database at %s:%s, rc = %d"
               OSS_NEWLINE,
               pHostName, pServiceName, rc ) ;
      exit ( 0 ) ;
   }

   rc = listGroups ( &connection ) ;
   if ( rc )
   {
      printf ( "Failed to list replicagroups, rc = %d"
               OSS_NEWLINE, rc ) ;
      exit ( 0 ) ;
   }
   // get catalog replica set by name
   rc = connection.getReplicaGroup ( "SYSCatalogGroup", replicagroup ) ;
   if ( rc )
   {
      printf ( "Failed to get catalog group, rc = %d" OSS_NEWLINE, rc ) ;
      exit ( 0 ) ;
   }
   // find how many nodes in the set
   rc = replicagroup.getNodeNum ( SDB_NODE_ALL, &nodeNum ) ;
   if ( rc )
   {
      printf ( "Failed to get number of node, rc = %d" OSS_NEWLINE,
               rc ) ;
      exit ( 0 ) ;
   }
   printf ( "There are totally %d nodes in the set" OSS_NEWLINE,
            nodeNum ) ;
   // get detailed information about the set
   rc = replicagroup.getDetail ( detail ) ;
   if ( rc )
   {
      printf ( "Failed to get details, rc = %d" OSS_NEWLINE,
               rc ) ;
      exit ( 0 ) ;
   }
   cout << detail.toString() << endl ;
   // get the master node
   rc = replicagroup.getMaster ( replicamasternode ) ;
   if ( rc )
   {
      printf ( "Failed to get replicamasternode, rc = %d" OSS_NEWLINE, rc ) ;
      exit ( 0 ) ;
   }
   printf ( "master host name: %s" OSS_NEWLINE,
            replicamasternode.getHostName () ) ;
   printf ( "master service name: %s" OSS_NEWLINE,
            replicamasternode.getServiceName () ) ;
   // get one of slave
   rc = replicagroup.getSlave ( replicaslavenode ) ;
   if ( rc )
   {
      printf ( "Failed to get replicaslavenode, rc = %d" OSS_NEWLINE, rc ) ;
      exit ( 0 ) ;
   }
   printf ( "slave host name: %s" OSS_NEWLINE,
            replicaslavenode.getHostName () ) ;
   printf ( "slave service name: %s" OSS_NEWLINE,
            replicaslavenode.getServiceName () ) ;

   // add a new group
   rc = addGroup ( &connection ) ;
   if ( rc )
   {
      printf ( "Failed to add group, rc = %d" OSS_NEWLINE, rc ) ;
      exit ( 0 ) ;
   }

   // list groups again
   rc = listGroups ( &connection ) ;
   if ( rc )
   {
      printf ( "Failed to list replicagroups, rc = %d"
               OSS_NEWLINE, rc ) ;
      exit ( 0 ) ;
   }
   /* disconnect from server */
   connection.disconnect() ;
   return 0 ;
}
