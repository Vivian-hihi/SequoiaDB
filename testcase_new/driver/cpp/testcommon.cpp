#include "testcommon.hpp"
#include <cstring>
#include <vector>
#include <iostream>
#include <sys/types.h>
#include <unistd.h>
#include <gtest/gtest.h>
#include <stdarg.h>

using namespace std;
using namespace sdbclient;
using namespace bson ;
using testing::internal::g_argvs ;
extern vector<testing::internal::String> g_argvs ;

char HOSTNAME[100]         = "localhost" ;
char SVCNAME[100]          = "11810" ;
char CHANGEDPREFIX[100]    = "sdv_cpp_test" ;
char RSRVPORTBEGIN[100]    = "26000" ;
char RSRVPORTEND[100]      = "27000" ;
char RSRVNODEDIR[100]      = "/opt/sequoiadb/database/" ;
char WORKDIR[100]          = "/tmp/cpptest" ;
char COORD[100]   		   = "localhost:11810" ;
char HOST[100]    		   = "localhost" ;

void printMsg( const char* fmt, ... )
{
    va_list ap ;
    va_start( ap, fmt ) ;
    vprintf( fmt, ap ) ;
    va_end( ap ) ;
}

void getConf()
{
	static bool inited = false ;
	if( inited )
	{
		return ;
	}
	else
	{
		inited = true ;
	}

    cout << "Print command args: " << endl ;
    for( int i = 0;i < g_argvs.size();i++ )
		cout << g_argvs[i] << " " ;
    cout << endl ;
    
    for( int i = 0;i < g_argvs.size();i++ )
    {
    	string para = g_argvs[i] ;
      	if( para == "--hostname" || para == "-n" )
        	strcpy( HOSTNAME,g_argvs[i+1].c_str() ) ;
      	else if( para == "--svcname" || para == "-s" )
         	strcpy( SVCNAME,g_argvs[i+1].c_str() ) ;
      	else if( para == "--changedprefix" || para == "-c" )
         	strcpy( CHANGEDPREFIX,g_argvs[i+1].c_str() ) ;
	  	else if( para == "--rsrvportbegin" || para == "-b" )
		 	strcpy( RSRVPORTBEGIN,g_argvs[i+1].c_str() ) ;
	  	else if( para == "--rsrvportend" || para == "-e" )
		 	strcpy( RSRVPORTEND,g_argvs[i+1].c_str() ) ;
	  	else if( para == "--rsrvnodedir" || para == "-d" )
		 	strcpy( RSRVNODEDIR,g_argvs[i+1].c_str() ) ;
	  	else if( para == "--workdir" || para == "-w" )
		 	strcpy( WORKDIR,g_argvs[i+1].c_str() ) ; 
    }
	sprintf( COORD, "%s%s%s", HOSTNAME, ":", SVCNAME ) ;
   
    cout << "HostName: " << HOSTNAME << endl ;
    cout << "SvcName: " << SVCNAME << endl ;
    cout << "CHANGEDPREFIX: " << CHANGEDPREFIX << endl ;
    cout << "RSPVPORTBEGIN: " << RSRVPORTBEGIN << endl ;
	cout << "RSPVPORTEND: " << RSRVPORTEND << endl ;
	cout << "RSPVNODEDIR: " << RSRVNODEDIR << endl ;
	cout << "WORKDIR: " << WORKDIR << endl ;
	cout << "COORD: " << COORD << endl ;
}

void getUniqueName( const char* modName, char name[] )
{
	pid_t pid = getpid() ;
	sprintf( name, "%s_%d_%s", CHANGEDPREFIX, pid, modName ) ;
}

void ossSleep(int milliseconds)
{
	#if defined (_WINDOWS)
    	Sleep( milliseconds ) ;
    #else
        usleep( milliseconds * 1000 ) ;
    #endif
}

bool isStandalone( sdb& db )
{
	sdbCursor cursor ;
	int rc = db.getList( cursor, 7 ) ;
	if( rc == -159 )
		return true ;
	else
		return false ;
}

int createNormalCl( sdb& db, sdbCollectionSpace& cs, sdbCollection& cl,
                    const char* csname, const char* clname )
{
	int rc = SDB_OK ;
	// connect and create cs cl
	getConf() ;
	rc = db.connect( HOSTNAME, SVCNAME, USER, PASSWD ) ;
	CHECK_RC( rc, "fail to connect sdb, rc = %d\n", rc ) ; 
	rc = db.createCollectionSpace( csname, SDB_PAGESIZE_4K, cs ) ;
	CHECK_RC( rc, "fail to create cs %s, rc = %d\n", csname, rc ) ;
	rc = cs.createCollection( clname, cl ) ;
	CHECK_RC( rc, "fail to create cl %s, rc = %d\n", clname, rc ) ;
done:
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
int getGroups( sdb& db, vector<string>& vec )
{
    int rc = SDB_OK ;
    sdbCursor cursor ;
    BSONObj obj ;

    rc = db.listReplicaGroups( cursor ) ;
    CHECK_RC( rc, "fail to list replica groups, rc = %d\n", rc ) ;
    while( !cursor.next( obj ) )
    {
        string groupname = obj.getField( "GroupName" ).String() ;
        if( groupname != "SYSCoord" && groupname != "SYSCatalogGroup" )
        {
            // cout << groupname << endl ;
            vec.push_back( groupname ) ;
        }
    }

done:
    return rc ;
error:
    goto done ;
}

/********************************************************************
* get local hostname 
* return 0 if success, return errno if fail
*
********************************************************************/
int getLocalHost()
{
	int rc = 0 ;
	rc = gethostname( HOST, sizeof(HOST) ) ;
	CHECK_RC( rc, "fail to get local hostname, rc = %d", rc ) ;
	cout << "local hostname: " << HOST << endl ;
done:
	return rc ;
error:
	goto done ;
}
