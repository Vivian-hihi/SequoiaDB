#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <gtest/gtest.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdarg.h>
#include "testcommon.hpp"
#include "arguments.hpp"

using namespace std ;

/*****************************************************************
 * Print err msg
 *
 *****************************************************************/
void printMsg( const char* fmt, ... )
{
   va_list ap ;
   va_start( ap, fmt ) ;
   vprintf( fmt, ap ) ;
   va_end( ap ) ;
}

/*****************************************************************
 * create normal collection with csname clname
 * return SDB_OK if success, return others if error
 *
 ******************************************************************/
INT32 createNormalCsCl( sdbConnectionHandle db, sdbCSHandle* cs, sdbCollectionHandle* cl,
                        const CHAR* csName, const CHAR* clName )
{
   INT32 rc = SDB_OK ;
   // create cs
   rc = sdbCreateCollectionSpace( db, csName, SDB_PAGESIZE_4K, cs ) ;
   CHECK_RC( SDB_OK, rc, "fail to create cs %s", csName ) ;
   // create cl
   rc = sdbCreateCollection( *cs, clName, cl ) ;
   CHECK_RC( SDB_OK, rc, "fail to create cl %s", clName ) ;	
done:
   return rc ;
error:
   goto done ;
}

/*************************************************************
 * check db is standalone or not, 
 * if standalone return true, otherwise return false
 *
 *************************************************************/
BOOLEAN isStandalone( sdbConnectionHandle db )
{
   BOOLEAN ret = FALSE ;
   INT32 rc = SDB_OK ;
   sdbCursorHandle cursor = SDB_INVALID_HANDLE ;

   rc = sdbListReplicaGroups( db, &cursor ) ;
   if( SDB_RTN_COORD_ONLY == rc )
   {
      ret = TRUE ;
   }
   sdbReleaseCursor( cursor ) ;
   return ret ;
}

/**********************************************************
 * get local hostname 
 *
 **********************************************************/
INT32 getLocalHost( CHAR hostName[], INT32 len )
{
   INT32 rc = gethostname( hostName, len ) ;
   CHECK_RC( SDB_OK, rc, "fail to gethostname" ) ;
   printf( "local hostname: %s\n", hostName ) ;
done:
   return rc ;
error:
   goto done ;
}

/*******************************************************************************
 * get a idle port between RSRVPORTBEGIN and RSRVPORTEND in localhost
 *
 *******************************************************************************/
void getIdlePort( CHAR* port )
{
   INT32 start = atoi( ARGS->rsrvPortBegin() ) ;
   INT32 end = atoi( ARGS->rsrvPortEnd() ) ;
   struct sockaddr_in servaddr ;
   INT32 sock, i, serverport, ret ;

   bzero( &servaddr, sizeof( servaddr ) ) ;
   servaddr.sin_family = AF_INET ;
   inet_pton( AF_INET, "127.0.0.1", &servaddr.sin_addr ) ;
   for( i = start;i <= end;i += 5 )
   {
      servaddr.sin_port = htons( i ) ;
      ret = connect( sock, (struct sockaddr*)&servaddr, sizeof( servaddr ) ) ;
      if( EISCONN == ret )  continue ;
      close( sock ) ;
      sprintf( port, "%d", i ) ;
      break ;
   }
}

/***********************************************************************************
 * get nodes of a group to a vector
 * vector ex [ "sdbserver1:20100", "sdbserver2:20100", ... ]
 * return SDB_OK if success, return others if error
 *
 ***********************************************************************************/
INT32 getGroupNodes( sdbConnectionHandle db, const CHAR* rgName,
                     vector<string>& nodes )
{
   INT32 rc = SDB_OK ;
   sdbCursorHandle cursor = SDB_INVALID_HANDLE ;
   bson cond, obj ;
   bson_init( &cond ) ;
   bson_init( &obj ) ;

   rc = bson_append_string( &cond, "GroupName", rgName ) ;
   CHECK_RC( BSON_OK, rc, "bson fail to append GroupName:%s", rgName ) ;
   rc = bson_finish( &cond ) ;
   CHECK_RC( BSON_OK, rc, "bson fail to finish" ) ;
   rc = sdbGetList( db, SDB_LIST_GROUPS, &cond, NULL, NULL, &cursor ) ;
   CHECK_RC( SDB_OK, rc, "fail to get list groups" ) ;

   rc = sdbNext( cursor, &obj ) ;
   CHECK_RC( SDB_OK, rc, "fail to get sdb next" ) ;
   bson_iterator it, sub ;
   bson_find( &it, &obj, "Group" ) ;
   bson_iterator_subiterator( &it, &sub ) ;
   bson_iterator_next( &sub ) ;
   while( bson_iterator_more( &sub ) )
   {
      bson tmp1 ;
      bson_init( &tmp1 ) ;
      bson_iterator_subobject( &sub, &tmp1 ) ;
      bson_iterator i1 ;
      bson_find( &i1, &tmp1, "HostName" ) ;
      string hostName = bson_iterator_string( &i1 ) ;

      bson_find( &i1, &tmp1, "Service" ) ;
      bson tmp2 ;
      bson_init( &tmp2 ) ;
      bson_iterator_subobject( &i1, &tmp2 ) ;
      bson_iterator i2 ;
      bson_iterator_init( &i2, &tmp2 ) ;
      bson_iterator_next( &i2 ) ;
      bson tmp3 ;
      bson_init( &tmp3 ) ;
      bson_iterator_subobject( &i2, &tmp3 ) ;
      bson_iterator i3 ;
      bson_find( &i3, &tmp3, "Name" ) ;
      string svcName = bson_iterator_string( &i3 ) ;

      string nodeName = hostName + ":" + svcName ;
      nodes.push_back( nodeName ) ;

      bson_destroy( &tmp3 ) ;
      bson_destroy( &tmp2 ) ;
      bson_destroy( &tmp1 ) ;
      bson_iterator_next( &sub ) ;
   }

done:
   bson_destroy( &cond ) ;
   bson_destroy( &obj ) ;
   sdbReleaseCursor( cursor ) ;
   return rc ;
error:
   goto done ;
}

/*****************************************************************
 * get all data groups to a vector, 
 * vector ex [ "group1", "group2", ... ]
 * return SDB_OK if success, return others if error
 *
 *****************************************************************/
INT32 getGroups( sdbConnectionHandle db, vector<string>& groups )
{
   INT32 rc = SDB_OK ;
   sdbCursorHandle cursor = SDB_INVALID_HANDLE ;
   bson obj ;
   bson_init( &obj ) ;

   rc = sdbListReplicaGroups( db, &cursor ) ;
   CHECK_RC( SDB_OK, rc, "fail to list replica groups", rc ) ;
   while( !sdbNext( cursor, &obj ) )
   {
      bson_iterator it ;
      bson_find( &it, &obj, "GroupName" ) ;
      string rgName = bson_iterator_string( &it ) ;
      if( rgName != "SYSCoord" && rgName != "SYSCatalogGroup" )
      {
         groups.push_back( rgName ) ;
      }
      bson_destroy( &obj ) ;
      bson_init( &obj ) ;
   }

done:
   bson_destroy( &obj ) ;
   sdbReleaseCursor( cursor ) ;
   return rc ;
error:
   goto done ;
}
