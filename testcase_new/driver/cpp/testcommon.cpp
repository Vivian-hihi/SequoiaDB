#include "testcommon.hpp"
#include <string>
#include <vector>
#include <iostream>
#include <sys/types.h>
#include <unistd.h>
#include <gtest/gtest.h>

using namespace std;
using namespace sdbclient;
using namespace bson ;
using testing::internal::g_argvs ;
extern vector<testing::internal::String> g_argvs ;

string HOSTNAME         = "localhost" ;
string SVCNAME          = "11810" ;
string CHANGEDPREFIX    = "sdv_cpp_test" ;
string RSRVPORTBEGIN    = "26000" ;
string RSRVPORTEND      = "27000" ;
string RSRVNODEDIR      = "/opt/sequoiadb/database/" ;
string WORKDIR          = "/tmp/cpptest" ;

void createCollection( sdb &db, sdbCollection *cl, const CHAR *clName )
{
   INT32 rc = SDB_OK;
   
   //new sdb       
   const CHAR *hostName = HOST;
   const CHAR *svcPort  = SERVER;
   rc = db.connect( hostName, svcPort, "", "" );
   ASSERT_EQ( SDB_OK, rc );
   
   //create cs
   sdbCollectionSpace cs;
   const CHAR *csName = CSNAME;  
   rc = db.createCollectionSpace( csName, 4096, cs );
   if( SDB_DMS_CS_EXIST == rc )
   {
      rc = db.getCollectionSpace( csName, cs );
   }
   ASSERT_EQ( SDB_OK, rc );
   
   //create cl
   BSONObj optionObj = BSON( "ReplSize" << 0 );  
   rc = cs.createCollection( clName, optionObj, *cl );
   if( SDB_DMS_EXIST == rc )
   {
      rc = cs.dropCollection( clName );
      ASSERT_EQ( SDB_OK, rc );
      rc = cs.createCollection( clName, *cl );
   }
   ASSERT_EQ( SDB_OK, rc );
}

void getConf()
{
    cout << "Print command args: " << endl ;
    for( int i = 0;i < g_argvs.size();i++ )
      cout << g_argvs[i] << " " ;
    cout << endl ;
    
    for( int i = 0;i < g_argvs.size();i++ )
    {
      string para = g_argvs[i] ;
      if( para == "--hostname" || para == "-n" )
         HOSTNAME = g_argvs[i+1] ;
      else if( para == "--svcname" || para == "-s" )
         SVCNAME = g_argvs[i+1] ;
      else if( para == "--changedprefix" || para == "-c" )
         CHANGEDPREFIX = g_argvs[i+1] ;
	  else if( para == "--rsrvportbegin" || para == "-b" )
		   RSRVPORTBEGIN = g_argvs[i+1] ;
	  else if( para == "--rsrvportend" || para == "-e" )
		   RSRVPORTEND = g_argvs[i+1] ;
	  else if( para == "--rsrvnodedir" || para == "-d" )
		   RSRVNODEDIR = g_argvs[i+1] ;
	  else if( para == "--workdir" || para == "-w" )
		   WORKDIR = g_argvs[i+1] ; 
    }
   
   cout << "HostName: " << HOSTNAME << endl ;
   cout << "SvcName: " << SVCNAME << endl ;
   cout << "CHANGEDPREFIX: " << CHANGEDPREFIX << endl ;
   cout << "RSPVPORTBEGIN: " << RSRVPORTBEGIN << endl ;
	cout << "RSPVPORTEND: " << RSRVPORTEND << endl ;
	cout << "RSPVNODEDIR: " << RSRVNODEDIR << endl ;
	cout << "WORKDIR: " << WORKDIR << endl ;
}

void ossSleep(int milliseconds)
{
        #if defined (_WINDOWS)
                Sleep(milliseconds) ;
        #else
                usleep(milliseconds*1000) ;
        #endif
}

bool isStandalone()
{
        sdb db ;
        db.connect(HOST, SERVER) ;
        sdbCursor cursor ;
        int rc = db.getList(cursor,7) ;
        if(rc == -159)
                return true ;
        else
                return false ;
}

