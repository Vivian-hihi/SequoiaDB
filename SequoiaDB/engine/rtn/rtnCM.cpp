/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = rtnCM.cpp

   Descriptive Name = rtnClusterManager

   When/how to use: this program may be used on binary and text-formatted
   versions of PMD component. This file contains main function for sdbcm,
   which is used to do cluster managing.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          12/17/2013  JHL  Initial Draft

   Last Changed =

*******************************************************************************/

#include "rtnCM.hpp"
#include "core.h"
#include "ossSocket.hpp"
#include "ossProc.hpp"
#include "ossUtil.hpp"
#include "ossMem.hpp"
#include "ossLatch.hpp"
#include "pd.hpp"
#include "pmd.hpp"
#include "pmdStartup.hpp"
#include "pmdOptionsMgr.hpp"
#include "msgMessage.hpp"
#include "sdbcm.hpp"
#include "../bson/bson.h"
#include "pdTrace.hpp"
#include "pmdTrace.hpp"
#include "rtnTrace.hpp"
#include "sdbcommon.hpp"

#include <stdlib.h>
#include <time.h>
#include <map>
#include <vector>
#include <queue>
#include <string>
#include <sstream>
#include <boost/program_options.hpp>
#include <boost/program_options/parsers.hpp>
#include "../bson/bson.h"

using namespace engine;
using namespace std;
using namespace bson;
namespace CLSMGR
{
   // socket options
#define CM_TCPLISTENER_RETRY        5

   // buffer size
#define CM_NPIPE_SIZE               64

   // sleep time
#define SLEEPTIME                   10
#if defined (_LINUX)
      #include <dirent.h>
      #define SDBCM_NAME_BUF_LEN       255
      #define SDBCM_NAME_PATTERN       "sdbcm(%d)"
      #define SDBSTARTPROG             "sdbstart"
      #define SDBSTOPPROG              "sdbstop"
#elif defined (_WINDOWS)
      #include <windows.h>
      #include <tchar.h>
      #define SDBSTARTPROG             "sdbstart.exe"
      #define SDBSTOPPROG              "sdbstop.exe"
#endif

   // bit operation
   // service is starting, lock other sdbstart request, only setted by sdbStart2()
#define BIT_STARTING                0x01
   // service is running
#define BIT_RUNNING                 0x02
   // service is restarting, only setted by pidMonitor()
#define BIT_RESTARTING              0x04
#define OSS_BIT_CLR_SET(x,y)        ((x) = (y))

   // sdbStart type
#define TYPE_CLIENT                 1     // start by client
#define TYPE_MONITOR                2     // startby pidMonitor()

   namespace po = boost::program_options;

   struct Process
   {
      OSSPID pid;
      INT32 status;
      queue<time_t> startTime;
   };

   // restart count, always restart if set "-1", never restart if set "0"
   static INT32 resCount = -1 ;
   // restart time interval ( minute ), ignore this if resInterval <= 0
   static INT32 resInterval = 0 ;

   // TRUE: start all nodes while CM start.
   static BOOLEAN autoStart = FALSE ;

   // SVC list: mapped svcname to exec process
   static map<string, struct Process> svcList ;
   // mutex locker of svcList
   static ossSpinXLatch listLocker ;

   // parent path of node's config file, i.e. "conf/local"
   static CHAR pmdConf[OSS_MAX_PATHSIZE + 1] ;
   // path of binary file "sdbstart"
   static CHAR sdbstartExecName[OSS_MAX_PATHSIZE + 1] ;
   // path of binary file "sdbstop"
   static CHAR sdbstopExecName[OSS_MAX_PATHSIZE + 1] ;

   // indicates whether to exit main loop
   static BOOLEAN gSDBCMSTOP = FALSE ;

   // functions declare
   INT32 writeConfigureFile ( OSSFILE *pFile, const CHAR *pBufferWrite,
                              SINT64 iLenToWrite ) ;
   INT32 readConfigureFile( const CHAR *conf,
                            po::options_description &desc,
                            po::variables_map &vm ) ;
   INT32 buildFullPath( const CHAR *path, const CHAR *name,
                        UINT32 fullSize, CHAR *full ) ;
   // INT32 buildConfPath ( const CHAR *srvname,
                         // UINT32 fullSize, CHAR *full ) ;
   void cmTcpListener ( INT32 port ) ;
   INT32 startRemoteCommand ( SOCKET s ) ;
   void recvRemoteCommand ( SOCKET s ) ;
   INT32 buildArguments ( CHAR **pArgumentBuffer, list<const CHAR*> &argv ) ;
   INT32 sdbStart ( const CHAR *arg ) ;
   INT32 sdbStart2 ( string svcname, INT32 type ) ;
   INT32 startNode ( const CHAR *confpath, OSSPID *pid ) ;
   INT32 sdbStop ( const CHAR *arg ) ;
   INT32 sdbAdd ( const CHAR *arg1, const CHAR *arg2 ) ;
   INT32 sdbRm ( const CHAR *arg1 ) ;
   INT32 sdbModify ( const CHAR *arg1, const CHAR *arg2, const CHAR *arg3 ) ;
   INT32 sendRetCode ( SINT32 retCode, ossSocket *sock ) ;
   INT32 cmRecv ( CHAR *pBuffer, INT32 recvSize, ossSocket *sock ) ;
   INT32 cmSend ( const CHAR *pBuffer, INT32 sendSize, ossSocket *sock ) ;
   void pidMonitor () ;
   INT32 initEnv( UINT16 &port ) ;

   PD_TRACE_DECLARE_FUNCTION ( SDB_SDBCM_RDCFGFILE, "readConfigureFile" )
   INT32 readConfigureFile( const CHAR *conf,
                            po::options_description &desc,
                            po::variables_map &vm )
   {
      INT32 rc = SDB_OK;
      PD_TRACE_ENTRY ( SDB_SDBCM_RDCFGFILE );

      try
      {
         po::store ( po::parse_config_file<CHAR> ( conf, desc, TRUE ), vm ) ;
         po::notify ( vm ) ;
      }
      catch( po::reading_file )
      {
         rc = SDB_IO ;
         goto error ;
      }
      catch ( po::unknown_option)
      {
         rc = SDB_INVALIDARG ;
         goto error ;
      }
      catch ( po::invalid_option_value )
      {
         rc = SDB_INVALIDARG ;
         goto error ;
      }
      catch( po::error )
      {
         rc = SDB_INVALIDARG ;
         goto error ;
      }

   done:
      PD_TRACE_EXITRC ( SDB_SDBCM_RDCFGFILE, rc );
      return rc;
   error:
      goto done;
   }

   PD_TRACE_DECLARE_FUNCTION ( SDB_SDBCM_WTCFGFILE, "writeConfigureFile" )
   INT32 writeConfigureFile ( OSSFILE *pFile, const CHAR *pBufferWrite,
                              SINT64 iLenToWrite )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB_SDBCM_WTCFGFILE );
      SINT64 lenWritten = 0 ;
      SINT64 len = iLenToWrite ;
      while ( lenWritten < len )
      {
         rc = ossWrite ( pFile, pBufferWrite, len - lenWritten, &lenWritten ) ;
         if ( rc && rc != SDB_INTERRUPT )
            goto error ;
         len -= lenWritten ;
      }

   done:
      PD_TRACE_EXITRC ( SDB_SDBCM_WTCFGFILE, rc );
      return rc ;
   error:
      goto done ;
   }

   PD_TRACE_DECLARE_FUNCTION ( SDB_SDBCM_BLDFULLPATH, "buildFullPath" )
   INT32 buildFullPath ( const CHAR *path, const CHAR *name,
                         UINT32 fullSize, CHAR *full )
   {
      INT32 rc = SDB_OK;
      PD_TRACE_ENTRY ( SDB_SDBCM_BLDFULLPATH );
      if ( ossStrlen( path ) + ossStrlen( name )
           + 2 > fullSize )
      {
         rc = SDB_INVALIDARG;
         goto error;
      }
      ossMemset( full, 0, fullSize );
      ossMemcpy( full, path, ossStrlen( path ) );
      ossStrncat( full, OSS_FILE_SEP, 1 );
      ossStrncat( full, name, ossStrlen( name ) );
      PD_TRACE1 ( SDB_SDBCM_BLDFULLPATH, PD_PACK_STRING(full) );
   done:
      PD_TRACE_EXITRC ( SDB_SDBCM_BLDFULLPATH, rc );
      return rc;
   error:
      goto done;
   }

   // tcp listener to handle new connection request
   PD_TRACE_DECLARE_FUNCTION ( SDB_CMTCPLISTENER, "cmTcpListener" )
   void cmTcpListener ( INT32 port )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB_CMTCPLISTENER );
      UINT32 retry = 0 ;
      if ( port <= 0 )
      {
         rc = SDB_INVALIDARG;
         PD_LOG( PDERROR, "invalid port:%d", port );
         goto error;
      }

      while ( retry <= CM_TCPLISTENER_RETRY )
      {
         retry ++;
         // here we read from TCP socket
         PD_LOG ( PDEVENT, "Listening on port %d\n", port ) ;

         // main thread only accept, it never recv or send, so no need timeout
         ossSocket sock ( port ) ;
         rc = sock.initSocket () ;
         SDB_VALIDATE_GOTOERROR ( SDB_OK == rc, rc, "Failed initialize socket" ) ;

         rc = sock.bind_listen ();
         SDB_VALIDATE_GOTOERROR ( SDB_OK == rc, rc, "Failed to bind/listen socket" ) ;

         while ( TRUE )
         {
            SOCKET s ;
            rc = sock.accept ( &s, NULL, NULL ) ;
            // if we don't get anything for a period of time, let's loop
            if ( SDB_TIMEOUT == rc )
            {
               rc = SDB_OK ;
               continue ;
            }
             else if ( rc )
            {
               // if we fail due to error, let's restart socket
               PD_LOG ( PDERROR, "Failed to accept socket in TcpListener" );
               PD_LOG ( PDEVENT, "Restarting socket to listen" );
               break ;
            }

            // now we have a tcp socket for a new connection
            rc = startRemoteCommand ( s ) ;
            if ( rc )
            {
               PD_LOG ( PDERROR, "Failed to execute remote command" ) ;
               // close remote connection if we can't create new thread
               ossSocket newsock ( &s ) ;
               newsock.close () ;
               continue ;
            }
         }
      }

   error:
      switch ( rc )
      {
      case SDB_SYS:
         PD_LOG ( PDSEVERE,  "System error occured" ) ;
         break ;
      default:
         PD_LOG ( PDSEVERE, "Internal error" ) ;
      }
      PD_TRACE_EXITRC ( SDB_CMTCPLISTENER, rc ) ;
      iPmdProc::stop();
   }

   PD_TRACE_DECLARE_FUNCTION ( SDB_STARTREMOTECMD, "startRemoteCommand" )
   INT32 startRemoteCommand ( SOCKET s )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB_STARTREMOTECMD );
      try
      {
         boost::thread exeThread ( recvRemoteCommand, s ) ;
         exeThread.detach () ;
      }
      catch ( boost::exception& )
      {
         rc = SDB_SYS ;
         PD_LOG ( PDERROR, "Failed to create thread" );
         goto error ;
      }

   done:
      PD_TRACE_EXITRC ( SDB_STARTREMOTECMD, rc );
      return rc ;
   error:
      goto done ;
   }

   PD_TRACE_DECLARE_FUNCTION ( SDB_RECVREMOTECMD, "recvRemoteCommand" )
   void recvRemoteCommand ( SOCKET s )
   {
      INT32 rc      = SDB_OK ;
      PD_TRACE_ENTRY ( SDB_RECVREMOTECMD );
      SINT32 remoCode ;
      CHAR *arg1 = NULL, *arg2 = NULL, *arg3 = NULL, *arg4 = NULL ;

      CHAR *pReceiveBuffer = NULL ;
      SINT32 packetLength = 0 ;

      // create socket
      ossSocket sock ( &s ) ;

      // read the receiving message length
      rc = cmRecv ( (CHAR*)&packetLength, sizeof (SINT32), &sock ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to extract message length" ) ;
         goto error ;
      }
      PD_TRACE1 ( SDB_RECVREMOTECMD, PD_PACK_INT(packetLength) );
      if ( packetLength < (SINT32)sizeof (MsgCMRequest) )
      {
         PD_LOG ( PDERROR, "Invalid message length" ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }
      // free at the end of this function
      pReceiveBuffer = (CHAR*)SDB_OSS_MALLOC ( sizeof(CHAR) * (packetLength) ) ;
      if ( !pReceiveBuffer )
      {
         PD_LOG ( PDERROR, "Failed to allocate memory" ) ;
         rc = SDB_OOM ;
         goto error ;
      }
      *(SINT32*)(pReceiveBuffer) = packetLength ;

      // now let's read the real package
      rc = cmRecv ( &pReceiveBuffer[sizeof (SINT32)],
                     packetLength-sizeof (SINT32),
                     &sock ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to receive cm request" ) ;
         goto error ;
      }

      rc = msgExtractCMRequest ( pReceiveBuffer, &remoCode, &arg1, &arg2, &arg3, &arg4 ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to extract cm request" ) ;
         goto error ;
      }
      PD_TRACE1 ( SDB_RECVREMOTECMD, PD_PACK_INT(remoCode) );

      switch ( remoCode )
      {
      case SDBSTART:
         rc = sdbStart ( arg1 ) ;
         break ;

      case SDBSTOP:
         rc = sdbStop ( arg1 ) ;
         break ;

      case SDBADD:
         rc = sdbAdd ( arg1, arg2 ) ;
         break;

      case SDBMODIFY:
         rc = sdbModify ( arg1, arg2, arg3 ) ;
         break ;
      case SDBRM:
         rc = sdbRm( arg1 ) ;
         break ;

      default:
         rc = SDB_INVALIDARG ;
         goto error ;
      }

   done:
      rc = sendRetCode ( (SINT32) rc, &sock ) ;
      if ( rc )
         PD_LOG ( PDERROR, "Failed to send return code, rc=%d", rc ) ;
      if ( pReceiveBuffer )
         SDB_OSS_FREE ( pReceiveBuffer ) ;
      PD_TRACE_EXIT ( SDB_RECVREMOTECMD );
      return ;
   error:
      goto done ;
   }

   PD_TRACE_DECLARE_FUNCTION ( SDB_SDBCM_BLDARGS, "buildArguments" )
   INT32 buildArguments ( CHAR **pArgumentBuffer, list<const CHAR*> &argv )
   {
      SDB_ASSERT ( pArgumentBuffer, "Invalid input" ) ;
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB_SDBCM_BLDARGS );
      INT32 bufferSize = 0 ;
      INT32 pos = 0 ;
      // estimate the size of final buffer
      for ( list<const CHAR*>::iterator it = argv.begin(); it != argv.end(); ++it )
         bufferSize += ( ossStrlen ( *it ) + 1 ) ;
      if ( bufferSize <= 0 )
      {
         PD_LOG ( PDERROR, "Failed to calculate buffer size" ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }
      // allocate buffer, free at calling function
      *pArgumentBuffer = (CHAR*)SDB_OSS_MALLOC ( bufferSize ) ;
      if ( !(*pArgumentBuffer) )
      {
         PD_LOG ( PDERROR, "Failed to allocate buffer for %d bytes",
                  bufferSize ) ;
         rc = SDB_OOM ;
         goto error ;
      }
      ossMemset ( *pArgumentBuffer, 0, bufferSize ) ;
      // copy arguments into buffer
      for ( list<const CHAR*>::iterator it = argv.begin(); it != argv.end(); ++it )
      {
         ossStrncpy ( &(*pArgumentBuffer)[pos], *it, bufferSize - pos ) ;
         pos += ossStrlen ( *it ) ;
         // each arguments are separated by '\0'
         (*pArgumentBuffer)[pos] = '\0' ;
         ++pos ;
      }

   done:
      PD_TRACE_EXITRC ( SDB_SDBCM_BLDARGS, rc );
      return rc ;
   error:
      goto done ;
   }

   PD_TRACE_DECLARE_FUNCTION ( SDB_SDBSTART, "sdbStart" )
   INT32 sdbStart ( const CHAR *arg )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB_SDBSTART );
      string svcname;
      try
      {
         BSONObj bsonArg ( arg ) ;
         BSONObjIterator j ( bsonArg ) ;
         while ( j.more () )
         {
            BSONElement e = j.next() ;
            const CHAR *pKey = e.fieldName () ;
            if ( !ossStrcmp ( pKey, PMD_OPTION_SVCNAME ) )
            {
               if ( e.type() == String )
               {
                  svcname = e.str() ;
               }
               else
               {
                  // bad type
                  PD_LOG ( PDERROR, "Unexpected type" ) ;
                  rc = SDB_INVALIDARG ;
                  goto error ;
               }
               // stop iterate
               break ;
            }
         }
         if ( !svcname.empty() )
         {
            PD_TRACE1 ( SDB_SDBSTART, PD_PACK_STRING(svcname.c_str()) );
            rc = sdbStart2 ( svcname, TYPE_CLIENT ) ;
            if ( rc )
            {
               if ( SDBCM_SVC_STARTED == rc )
               {
                  PD_LOG( PDEVENT, "Node has already been started, svnname = %s",
                          svcname.c_str() ) ;
                  rc = SDB_OK ;
                  goto done ;
               }
               else
               {
                  PD_LOG ( PDERROR, "Failed to start node, svcname = %s",
                           svcname.c_str() ) ;
                  goto error ;
               }
            }
            else
               PD_LOG ( PDEVENT, "Start SequoiaDB node, svcname = %s",
                        svcname.c_str() ) ;
         }
         else
         {
            PD_LOG ( PDERROR, "Can not find svcname" ) ;
            rc = SDB_INVALIDARG ;
            goto error ;
         }
      }
      catch ( std::exception &e )
      {
         PD_LOG ( PDERROR, "Exception detected during extracting cm request: %s",
                  e.what() ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }
   done:
      PD_TRACE_EXITRC ( SDB_SDBSTART, rc );
      return rc ;
   error:
      goto done ;
   }

   PD_TRACE_DECLARE_FUNCTION ( SDB_SDBSTART2, "sdbStart2" )
   INT32 sdbStart2 ( const string svcname, INT32 type )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB_SDBSTART2 );
      PD_TRACE1 ( SDB_SDBSTART2, PD_PACK_STRING(svcname.c_str()) );

      po::options_description desc ( "Command options" ) ;
      po::variables_map vm ;
      PMD_ADD_PARAM_OPTIONS_BEGIN( desc )
         PMD_COMMANDS_OPTIONS
      PMD_ADD_PARAM_OPTIONS_END
      string confpath = pmdConf ;
      confpath += OSS_FILE_SEP + svcname ;
      PD_TRACE1 ( SDB_SDBSTART2, PD_PACK_STRING(confpath.c_str()) );
      string conf = confpath + OSS_FILE_SEP PMD_DFT_CONF ;
      rc = readConfigureFile ( conf.c_str(), desc, vm ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Can not read configure file: %s", conf.c_str() ) ;
         goto error ;
      }

      if ( vm.count( PMD_OPTION_DBPATH ) )
      { // if exist "dbpath" in config file
         OSSPID pid ;
         time_t now;
         time ( &now ) ;
         ossLatch ( &listLocker ) ;
         if ( svcList.count(svcname) == 0 )
         { // the first starting of this service name by client
            Process proc;
            OSS_BIT_CLR_SET ( proc.status, BIT_STARTING ) ;
            proc.startTime.push ( now ) ;
            svcList[svcname] = proc ;
            ossUnlatch ( &listLocker ) ;
            rc = startNode ( confpath.c_str(), &pid ) ;
            if ( SDB_OK == rc )
            { // started successfully
               ossLatch ( &listLocker ) ;
               svcList[svcname].pid = pid ;
               OSS_BIT_CLR_SET ( svcList[svcname].status, BIT_RUNNING ) ;
               ossUnlatch ( &listLocker ) ;
            }
            else
            {
               ossLatch ( &listLocker ) ;
               svcList.erase ( svcname ) ;
               ossUnlatch ( &listLocker ) ;
               PD_LOG ( PDERROR, "Failed to start sequoiadb, svcname = %s",
                        svcname.c_str() ) ;
               goto error ;
            }
         } // if ( svcList.count(svcname) == 0 )
         else
         {
            Process &proc = svcList[svcname] ;
            if ( OSS_BIT_TEST ( proc.status, BIT_STARTING ) )
            { // service is starting, locked by other sdbStart2() thread
               ossUnlatch ( &listLocker ) ;
               rc = SDBCM_SVC_STARTING ;
               PD_LOG ( PDEVENT, "sequoiadb is starting, svcname = %s",
                        svcname.c_str() ) ;
               goto error ;
            }
            else if ( OSS_BIT_TEST ( proc.status, BIT_RESTARTING ) )
            { // status setted by pidMonitor
               if ( TYPE_MONITOR == type )
               { // only can be restarted by thread that called by pidMonitor()
                  // lock current svcname
                  OSS_BIT_SET ( proc.status, BIT_STARTING ) ;
                  proc.startTime.push ( now ) ;
                  ossUnlatch ( &listLocker ) ;
                  rc = startNode ( confpath.c_str(), &pid ) ;
                  if ( SDB_OK == rc )
                  {
                     ossLatch ( &listLocker ) ;
                     svcList[svcname].pid = pid ;
                     OSS_BIT_CLR_SET ( svcList[svcname].status, BIT_RUNNING ) ;
                     ossUnlatch ( &listLocker ) ;
                     PD_LOG ( PDEVENT, "Successfully to restart SequoiaDB node, \
   svcname = %s", svcname.c_str() ) ;
                  }
                  else
                  { // restart failed
                     ossLatch ( &listLocker ) ;
                     // restore status to "RUNNING", detected the failure by pidMonitor() later
                     OSS_BIT_CLR_SET ( svcList[svcname].status, BIT_RUNNING ) ;
                     ossUnlatch ( &listLocker ) ;
                     PD_LOG ( PDERROR, "Failed to restart sequoiadb, svcname = %s",
                              svcname.c_str() ) ;
                     goto error ;
                  }
               } // if ( TYPE_MONITOR == type )
               else
               {
                  ossUnlatch ( &listLocker ) ;
                  rc = SDBCM_SVC_RESTARTING ;
                  PD_LOG ( PDEVENT, "sequoiadb is restarting, svcname = %s",
                           svcname.c_str() ) ;
                  goto error ;
               }
            }
            else if ( OSS_BIT_TEST ( proc.status, BIT_RUNNING ) )
            {
               if ( ossIsProcessRunning ( proc.pid ) )
               { // process is running
                  ossUnlatch ( &listLocker ) ;
                  rc = SDBCM_SVC_STARTED ;
                  PD_LOG ( PDEVENT, "sequoiadb is started, svcname = %s, pid = %d",
                           svcname.c_str(), proc.pid ) ;
                  goto error ;
               }
               else
               { // process is not running
                  const CHAR *dbpath = vm[PMD_OPTION_DBPATH].as<string>().c_str() ;
                  CHAR startupFile[OSS_MAX_PATHSIZE + 1];
                  if ( NULL == dbpath )
                  { // "dbpath" in config file has not value
                     proc.startTime.push ( now ) ;
                     ossUnlatch ( &listLocker ) ;
                     PD_LOG ( PDERROR, "dbpath is empty in configure file: %s",
                              confpath.c_str() ) ;
                     rc = SDB_INVALIDPATH ;
                     goto error ;
                  }
                  rc = buildFullPath( dbpath, PMD_STARTUP_FILE_NAME,
                          OSS_MAX_PATHSIZE + 1, startupFile );
                  if ( rc )
                  {
                     proc.startTime.push ( now ) ;
                     ossUnlatch ( &listLocker ) ;
                     PD_LOG ( PDERROR, "Invalid arguments: %s",
                              confpath.c_str() ) ;
                     goto error ;
                  }
                  rc = ossAccess ( startupFile ) ;
                  if ( SDB_OK == rc )
                  { // engine crash but pidMonitor not yet detect, restart it by client manually

                     // lock current svcname
                     OSS_BIT_SET ( proc.status, BIT_STARTING ) ;
                     proc.startTime.push ( now ) ;
                     ossUnlatch ( &listLocker ) ;
                     rc = startNode ( confpath.c_str(), &pid ) ;
                     if ( SDB_OK == rc )
                     {
                        ossLatch ( &listLocker ) ;
                        svcList[svcname].pid = pid ;
                        OSS_BIT_CLR_SET ( svcList[svcname].status, BIT_RUNNING ) ;
                        ossUnlatch ( &listLocker ) ;
                     }
                     else
                     { // restart failed
                        ossLatch ( &listLocker ) ;
                        // restore status to "RUNNING", detected the failure by pidMonitor() later
                        OSS_BIT_CLR_SET ( svcList[svcname].status, BIT_RUNNING ) ;
                        ossUnlatch ( &listLocker ) ;
                        PD_LOG ( PDERROR, "Failed to restart sequoiadb, svcname = %s",
                                 svcname.c_str() ) ;
                        goto error ;
                     }
                  }
                  else if ( SDB_FNE == rc )
                  { // engine stopped normally, but pidMonitor not yet detect, start again by client
                     OSS_BIT_SET ( proc.status, BIT_STARTING ) ;
                     ossUnlatch ( &listLocker ) ;
                     rc = startNode ( confpath.c_str(), &pid ) ;
                     if ( SDB_OK == rc )
                     {
                        ossLatch ( &listLocker ) ;
                        Process &proc2 = svcList[svcname] ;
                        proc2.pid = pid ;
                        // reset starting time
                        queue<time_t> empty ;
                        swap ( proc2.startTime, empty ) ;
                        proc2.startTime.push ( now ) ;
                        OSS_BIT_CLR_SET ( proc2.status, BIT_RUNNING ) ;
                        ossUnlatch ( &listLocker ) ;
                     }
                     else
                     { // start failed, erase it
                        ossLatch ( &listLocker ) ;
                        svcList.erase ( svcname ) ;
                        ossUnlatch ( &listLocker ) ;
                        PD_LOG ( PDERROR, "Sequoiadb has been stopped, Failed to \
   start sequoiadb, svcname = %s", svcname.c_str() ) ;
                        goto error ;
                     }
                  }
                  else
                  { // permission error or system error, we can't continue
                     proc.startTime.push ( now ) ;
                     ossUnlatch ( &listLocker ) ;
                     PD_LOG ( PDERROR, "Error: rc = %d, svcname = %s",
                              rc, svcname.c_str() ) ;
                     goto error ;
                  }
               } // if ( !ossIsProcessRunning ( proc.pid ) )
            } // else if ( OSS_BIT_TEST ( proc.status, BIT_RUNNING ) )
         } // if ( svcList.count(svcname) != 0 )
      } // if ( vm.count( PMD_OPTION_DBPATH ) )
      else
      {
         PD_LOG ( PDERROR, "Can not read dbpath from configure file: %s",
                  conf.c_str() ) ;
         rc = SDB_INVALIDPATH ;
         goto error ;
      }

   done:
      PD_TRACE_EXITRC ( SDB_SDBSTART2, rc );
      return rc ;
   error:
      goto done ;
   }

   PD_TRACE_DECLARE_FUNCTION ( SDB_STARTNODE, "startNode" )
   INT32 startNode ( const CHAR *confpath, OSSPID *pid )
   {
      SDB_ASSERT ( confpath && pid, "NULL pointer exception" ) ;
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB_STARTNODE );
      PD_TRACE1 ( SDB_STARTNODE, PD_PACK_STRING(confpath) );
      CHAR *pArgumentBuffer = NULL ;
      list<const CHAR*> argv ;
      CHAR pNPipeBuf[CM_NPIPE_SIZE] = {0} ;
      OSSNPIPE outNPipe ;
      ossResultCode result ;
      OSSPID cpid ;

      // verify the configuration file
      rc = ossAccess ( confpath ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Can not access the configure file: %s", confpath ) ;
         goto error ;
      }

      argv.push_back ( sdbstartExecName ) ;
      argv.push_back ( SDBCM_OPTION_PREFIX  PMD_OPTION_CONFPATH ) ;
      argv.push_back ( confpath ) ;
      rc = buildArguments ( &pArgumentBuffer, argv ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to build sdbstart arguments") ;
         goto error ;
      }

      // call exec to run the command with arguments, do NOT wait until program
      // finish
      rc = ossExec ( pArgumentBuffer, pArgumentBuffer, NULL,
                     OSS_EXEC_SSAVE, cpid, result, NULL, &outNPipe ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to execute %s, rc = %d", sdbstartExecName, rc ) ;
         rc = SDBCM_FAIL ;
         goto error ;
      }
      // verify the executing result
      if ( result.termcode == OSS_EXIT_NORMAL && result.exitcode == SDB_OK  )
      {
         // if execute sdbStart succeed
         INT64 bufRead ;
         rc = ossReadNamedPipe ( outNPipe, pNPipeBuf, CM_NPIPE_SIZE, &bufRead, 0 ) ;
         if ( rc )
         {
            PD_LOG ( PDERROR, "Failed to trace the PID, rc = %d", rc ) ;
            goto error ;
         }
         CHAR *p = ossStrrchr ( pNPipeBuf, '(' ) ;
         if ( p )
         {
            // start successfully
            *pid = (OSSPID) ossAtoi ( p + 1 ) ;
         }
         else
         {
            rc = SDBCM_FAIL ;
            goto error ;
         }
      }
      else
         rc = SDBCM_FAIL ;

   done:
      if ( pArgumentBuffer )
         SDB_OSS_FREE ( pArgumentBuffer ) ;
      PD_TRACE_EXITRC ( SDB_STARTNODE, rc );
      return rc ;
   error:
      goto done ;
   }

   PD_TRACE_DECLARE_FUNCTION ( SDB_SDBSTOP, "sdbStop" )
   INT32 sdbStop ( const CHAR *arg )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB_SDBSTOP );
      CHAR *pArgumentBuffer = NULL ;
      list<const CHAR*> argv ;
      ossResultCode result ;
      OSSPID pid ;

      try
      {
         BSONObj bsonArg ( arg ) ;
         if ( !bsonArg.isEmpty() )
         {
            BSONObjIterator j ( bsonArg ) ;
            while ( j.more () )
            {
               BSONElement e = j.next() ;
               const CHAR *pKey = e.fieldName () ;
               if ( !ossStrcmp ( pKey, PMD_OPTION_SVCNAME ) )
               {
                  if ( e.type() == String )
                  {
                     const CHAR *svcname = e.valuestrsafe () ;
                     PD_TRACE1 ( SDB_SDBSTOP, PD_PACK_STRING(svcname) );
                     argv.push_back ( SDBCM_OPTION_PREFIX  PMD_OPTION_SVCNAME ) ;
                     argv.push_back ( svcname ) ;
                  }
                  else
                  {
                     // bad type
                     PD_LOG ( PDERROR, "Unexpected type" ) ;
                     rc = SDB_INVALIDARG ;
                     goto error ;
                  }
                  // stop iterate
                  break ;
               }
            }
         }
      }
      catch ( std::exception &e )
      {
         PD_LOG ( PDERROR, "Exception detected during extracting cm request: %s",
                  e.what() ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }

      argv.push_front ( sdbstopExecName ) ;
      rc = buildArguments ( &pArgumentBuffer, argv ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to build sdbstop arguments") ;
         goto error ;
      }
      // call exec to run the command with arguments, do NOT wait until program
      // finish
      rc = ossExec ( pArgumentBuffer, pArgumentBuffer, NULL,
                     OSS_EXEC_SSAVE, pid, result, NULL, NULL ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to execute %s, rc = %d", sdbstopExecName, rc ) ;
         goto error ;
      }

      // verify the executing result
      if ( result.termcode != OSS_EXIT_NORMAL )
         rc = SDBCM_FAIL ;
      else
      {
         switch ( result.exitcode )
         {
            case SDB_OK:
               rc = SDB_OK ;
               break ;
            case STOPPART:
               rc = SDBCM_STOP_PART ;
               break ;
            default:
               rc = SDBCM_FAIL ;
         }
      }
   done:
      if ( pArgumentBuffer )
         SDB_OSS_FREE ( pArgumentBuffer ) ;
      PD_TRACE_EXITRC ( SDB_SDBSTOP, rc );
      return rc ;
   error:
      goto done ;
   }

#define W_OK   2
   PD_TRACE_DECLARE_FUNCTION ( SDB_SDBADD, "sdbAdd" )
   INT32 sdbAdd ( const CHAR *arg1, const CHAR *arg2 )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB_SDBADD );
      CHAR path[OSS_MAX_PATHSIZE + 1] = { 0 } ;
      CHAR conf[OSS_MAX_PATHSIZE + 1] = { 0 } ;
      CHAR cata[OSS_MAX_PATHSIZE + 1] = { 0 } ;
      CHAR dbpath[OSS_MAX_PATHSIZE + 1] = { 0 } ;
      const CHAR *svcname = NULL ;
      const CHAR *dbpath_r = NULL ;
      OSSFILE file ;

      try
      {
         BSONObj bsonArg ( arg1 ) ;
         BSONObjIterator j ( bsonArg ) ;
         while ( j.more () )
         {
            BSONElement e = j.next() ;
            const CHAR *pKey = e.fieldName () ;
            // find option "svcname"
            if ( !ossStrcmp ( pKey, PMD_OPTION_SVCNAME ) )
            {
               if ( e.type() == String )
               {
                  svcname = e.valuestrsafe () ;
                  PD_TRACE1 ( SDB_SDBADD, PD_PACK_STRING(svcname) );
               }
               else
               {
                  // bad type
                  PD_LOG ( PDERROR, "Unexpected type" ) ;
                  rc = SDB_INVALIDARG ;
                  goto error ;
               }
            }
            // find option "dbpath"
            else if ( !ossStrcmp ( pKey, PMD_OPTION_DBPATH ) )
            {
               if ( e.type() == String )
               {
                  dbpath_r = e.valuestrsafe () ;
               }
               else
               {
                  // bad type
                  PD_LOG ( PDERROR, "Unexpected type" ) ;
                  rc = SDB_INVALIDARG ;
                  goto error ;
               }
            }
         }
      }
      catch ( std::exception &e )
      {
         PD_LOG ( PDERROR, "Exception detected during extracting cm request: %s",
                  e.what() ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }

      if ( svcname && dbpath_r )
      {
         if ( !ossGetRealPath(dbpath_r, dbpath, OSS_MAX_PATHSIZE) )
         {
            rc = SDB_INVALIDARG ;
            PD_LOG ( PDERROR, "Invalid dbpath" ) ;
            goto error;
         }
         // create path: "$dbpath"
         rc = ossAccess ( dbpath, W_OK ) ;
         // if we get permission, we can't continue
         if ( SDB_PERM == rc )
         {
            PD_LOG ( PDERROR, "Permission error" ) ;
            goto error ;
         }
         // if we can not find the file, then create one
         else if ( SDB_FNE == rc )
         {
            rc = ossMkdir ( dbpath, 0 ) ;
            if ( rc )
            {
               PD_LOG ( PDERROR, "Failed to create config file in path: %s",
                        dbpath ) ;
               goto error ;
            }
         }
         else if ( rc )
         {
            PD_LOG ( PDERROR, "System error, rc = %d", rc ) ;
            goto error ;
         }

         rc = buildFullPath( pmdConf, svcname,
                                OSS_MAX_PATHSIZE + 1, path );
         if ( SDB_OK != rc )
         {
            PD_LOG ( PDERROR, "configure file path is too long!" ) ;
            goto error;
         }

         // create path: "conf/local/$svcname"
         rc = ossAccess ( path, W_OK ) ;
         // if we get permission, we can't continue
         if ( SDB_PERM == rc )
         {
            PD_LOG ( PDERROR, "Permission error" ) ;
            goto error ;
         }
         // if we can not find the file, then create one
         else if ( SDB_FNE == rc )
         {
            rc = ossMkdir ( path, 0 ) ;
            if ( rc )
            {
               PD_LOG ( PDERROR, "Failed to create config file in path: %s",
                        path ) ;
               goto conferror ;
            }
         }
         else if ( rc )
         {
            PD_LOG ( PDERROR, "System error, rc = %d", rc ) ;
            goto error ;
         }
         // node existed
         else
         {
            PD_LOG ( PDERROR, "service node existed" ) ;
            rc = SDBCM_NODE_EXISTED ;
            goto error;
         }
      }
      else
      {
         rc = SDB_INVALIDARG ;
         PD_LOG ( PDERROR, "Never give the valid svcname and dbpath" ) ;
         goto error;
      }

      rc = buildFullPath( path, PMD_DFT_CONF,
                                OSS_MAX_PATHSIZE + 1, conf );
      if ( rc )
      {
         PD_LOG ( PDERROR, "configure file path is too long!" ) ;
         goto conferror;
      }
      // verify and create the configure file
      rc = ossOpen ( conf, OSS_CREATEONLY | OSS_WRITEONLY,
                     OSS_RU | OSS_WU | OSS_RG, file ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to create configure file, rc = %d", rc ) ;
         goto conferror ;
      }
      // create the configure file
      else
      {
         try
         {
            stringstream ss ;
            BSONObj bsonArg ( arg1 ) ;
            BSONObjIterator j ( bsonArg ) ;
            while ( j.more () )
            {
               BSONElement e = j.next() ;
               
               const CHAR *pKey = e.fieldName () ;
               ss<<pKey<<"=" ;
               if ( !ossStrcmp ( pKey, PMD_OPTION_DBPATH ) )
                  ss<<dbpath;
               else
               {
                  switch ( e.type() )
                  {
                  case NumberDouble :
                     ss<<e.numberDouble () ;
                     break ;
                  case NumberInt :
                     ss<<e.numberLong () ;
                     break ;
                  case NumberLong :
                     ss<<e.numberInt () ;
                     break ;
                  case String :
                     ss<<e.valuestrsafe () ;
                     break ;
                  default :
                     // bad type
                     PD_LOG ( PDERROR, "Unexpected type" ) ;
                     rc = SDB_INVALIDARG ;
                     goto conferror ;
                  }
               }
               ss<<endl ;
            }
            rc = writeConfigureFile ( &file, ss.str().c_str(), ss.str().size() ) ;
            if ( rc )
            {
               PD_LOG ( PDERROR, "Can not create file: %s", conf ) ;
               goto conferror ;
            }
         }
         catch ( std::exception &e )
         {
            PD_LOG ( PDERROR, "Exception detected during extracting BSONObj: %s",
                     e.what() ) ;
            rc = SDB_INVALIDARG ;
            goto conferror ;
         }
      }
      ossClose ( file ) ;
      // create catalog file
      try
      {
         BSONObj bsonArg ( arg2 ) ;
         if ( bsonArg.isEmpty() )
            goto done ;
         rc = buildFullPath( path, PMD_DFT_CAT,
                                OSS_MAX_PATHSIZE + 1, cata );
         if ( rc )
         {
            PD_LOG ( PDERROR, "catalog file path is too long!" ) ;
            goto conferror;
         }
         // verify and create the catalog file
         rc = ossOpen ( cata, OSS_CREATEONLY | OSS_WRITEONLY,
                        OSS_RU | OSS_WU | OSS_RG, file ) ;
         if ( rc )
         {
            PD_LOG ( PDERROR, "Failed to create catalog file, rc = %d", rc ) ;
            goto conferror ;
         }
         else
         {
            stringstream ss ;
            ss<<bsonArg<<endl;
            rc = writeConfigureFile ( &file, ss.str().c_str(), ss.str().size() ) ;
            if ( rc )
            {
               PD_LOG ( PDERROR, "Can not create file: %s", conf ) ;
               goto conferror ;
            }
         }
      }
      catch ( std::exception &e )
      {
         PD_LOG ( PDERROR, "Exception detected during extracting BSONObj: %s",
                  e.what() ) ;
         rc = SDB_INVALIDARG ;
         goto conferror ;
      }
      PD_LOG ( PDEVENT, "Successfully add node: %s", svcname) ;
      ossClose ( file ) ;
   done:
      PD_TRACE_EXITRC ( SDB_SDBADD, rc );
      return rc ;
   // error because of config file
   conferror:
      ossClose ( file ) ;
      ossDelete ( path ) ;
      goto done ;
   error:
      goto done ;
   }

   PD_TRACE_DECLARE_FUNCTION ( SDB_SDBRM, "sdbRm" )
   INT32 sdbRm ( const CHAR *arg1 )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB_SDBRM );

      CHAR confPath[OSS_MAX_PATHSIZE + 1] = { 0 } ;
      CHAR rmPath[OSS_MAX_PATHSIZE + 1] = { 0 } ;
      std::string confFile ;
      po::options_description desc ( "Command options" ) ;
      po::variables_map vm ;
      PMD_ADD_PARAM_OPTIONS_BEGIN( desc )
         PMD_COMMANDS_OPTIONS
      PMD_ADD_PARAM_OPTIONS_END

      const CHAR *svcname = NULL ;
      try
      {
         BSONObj obj( arg1 ) ;
         BSONElement e = obj.getField( PMD_OPTION_SVCNAME ) ;
         if ( e.eoo() || String != e.type() )
         {
            PD_LOG( PDERROR, "failed to get srv name from[%s]",
                    obj.toString().c_str() ) ;
            rc = SDB_INVALIDARG ;
            goto error ;

         }

         svcname = e.valuestrsafe () ;
         PD_TRACE1 ( SDB_SDBRM, PD_PACK_STRING(svcname) );
      }
      catch ( std::exception &e )
      {
         PD_LOG( PDERROR, "unexpected err happened:%s",e.what() ) ;
         rc = SDB_SYS ;
         goto error ;
      }

      rc = buildFullPath( pmdConf, svcname,
                          OSS_MAX_PATHSIZE + 1, confPath );
      if ( SDB_OK != rc )
      {
         PD_LOG ( PDERROR, "failed to build full path:%d", rc ) ;
         goto error;
      }

      confFile = string(confPath) + OSS_FILE_SEP PMD_DFT_CONF ;
      rc = readConfigureFile ( confFile.c_str(), desc, vm ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "Can not read configure file: %s",
                 confFile.c_str()) ;
         goto error ;
      }

      if ( vm.count( PMD_OPTION_LOGPATH ) )
      {
         ossMemset( rmPath, 0, OSS_MAX_PATHSIZE+1 );
         CHAR *p = ossGetRealPath( vm[PMD_OPTION_LOGPATH].as<string>().c_str(),
                     rmPath, OSS_MAX_PATHSIZE );
         if ( NULL == p )
         {
            PD_LOG( PDERROR, "Failed to get real path for: %s",
                    vm[PMD_OPTION_LOGPATH].as<string>().c_str()) ;
            rc = SDB_SYS ;
            goto error ;
         }

         rc = ossDelete( rmPath ) ;
         if ( SDB_OK != rc )
         {
            PD_LOG( PDERROR, "failed to rm log path;%s", rmPath ) ;
            goto error ;
         }
      }

      if ( vm.count( PMD_OPTION_BKUPPATH ) )
      {
         ossMemset( rmPath, 0, OSS_MAX_PATHSIZE+1 );
         CHAR *p = ossGetRealPath( vm[PMD_OPTION_BKUPPATH].as<string>().c_str(),
                     rmPath, OSS_MAX_PATHSIZE );
         if ( NULL == p )
         {
            PD_LOG( PDERROR, "Failed to get real path for: %s",
                    vm[PMD_OPTION_BKUPPATH].as<string>().c_str()) ;
            rc = SDB_SYS ;
            goto error ;
         }

         rc = ossDelete( rmPath ) ;
         if ( SDB_OK != rc )
         {
            PD_LOG( PDERROR, "failed to rm backup path;%s", rmPath ) ;
            goto error ;
         }
      }

      if ( vm.count( PMD_OPTION_DIAGLOGPATH ) )
      {
         ossMemset( rmPath, 0, OSS_MAX_PATHSIZE+1 );
         CHAR *p = ossGetRealPath( vm[PMD_OPTION_DIAGLOGPATH].as<string>().c_str(),
                     rmPath, OSS_MAX_PATHSIZE );
         if ( NULL == p )
         {
            PD_LOG( PDERROR, "Failed to get real path for: %s",
                    vm[PMD_OPTION_DIAGLOGPATH].as<string>().c_str()) ;
            rc = SDB_SYS ;
            goto error ;
         }

         rc = ossDelete( rmPath ) ;
         if ( SDB_OK != rc )
         {
            PD_LOG( PDERROR, "failed to rm diaglog path;%s", rmPath ) ;
            goto error ;
         }
      }

       /// we rm db path at last.
      if ( vm.count( PMD_OPTION_DBPATH ) )
      {
         ossMemset( rmPath, 0, OSS_MAX_PATHSIZE+1 );
         CHAR *p = ossGetRealPath( vm[PMD_OPTION_DBPATH].as<string>().c_str(),
                     rmPath, OSS_MAX_PATHSIZE );
         if ( NULL == p )
         {
            PD_LOG( PDERROR, "Failed to get real path for: %s",
                    vm[PMD_OPTION_DBPATH].as<string>().c_str()) ;
            rc = SDB_SYS ;
            goto error ;
         }

         rc = ossDelete( rmPath ) ;
         if ( SDB_OK != rc )
         {
            PD_LOG( PDERROR, "failed to rm db path;%s", rmPath ) ;
            goto error ;
         }
      }

      rc = ossDelete( confPath ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to rm conf path:%s", confPath ) ;
         goto error ;
      }

   done:
      PD_TRACE_EXITRC ( SDB_SDBRM, rc );
      return rc ;
   error:
      goto done ;
   }

#define SDBCM_RESERVED ".RESERVED"
   PD_TRACE_DECLARE_FUNCTION ( SDB_SDBMODIFY, "sdbModify" )
   INT32 sdbModify ( const CHAR *arg1, const CHAR *arg2, const CHAR *arg3 )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB_SDBMODIFY );
      CHAR confpath[OSS_MAX_PATHSIZE + 1] = { 0 } ;
      CHAR confReserved[OSS_MAX_PATHSIZE + 1] = { 0 } ;
      rc = buildFullPath ( pmdConf, SDBCM_RESERVED,
                           OSS_MAX_PATHSIZE + 1, confReserved ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to build reserved path" ) ;
         goto error ;
      }

      try
      {
         BSONObj bsonArg ( arg1 ) ;
         BSONObjIterator j ( bsonArg ) ;
         while ( j.more () )
         {
            BSONElement e = j.next() ;
            const CHAR *pKey = e.fieldName () ;
            if ( !ossStrcmp ( pKey, PMD_OPTION_SVCNAME ) )
            {
               if ( e.type() == String )
               {   
                  
                  rc = buildFullPath( pmdConf, e.valuestrsafe(),
                                      OSS_MAX_PATHSIZE + 1, confpath );
                  if ( rc )
                  {
                     PD_LOG ( PDERROR, "Failed to build config path") ;
                     goto error ;
                  }
                  PD_TRACE1 ( SDB_SDBMODIFY, PD_PACK_STRING(confpath) );
               }
               else
               {
                  // bad type
                  PD_LOG ( PDERROR, "Unexpected type" ) ;
                  rc = SDB_INVALIDARG ;
                  goto error ;
               }
               // stop iterate
               break ;
            }
         }
      }
      catch ( std::exception &e )
      {
         PD_LOG ( PDERROR, "Exception detected during extracting cm request: %s",
                  e.what() ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }

      // if never give old service name
      if ( confpath[0] == '\0' )
      {
         PD_LOG ( PDERROR, "Never give the old service name" ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }
      // rename old configure file
      rc = ossRenamePath ( confpath, confReserved ) ;
      if ( SDB_FNE == rc )
      {
         PD_LOG ( PDERROR, "service node not existed" ) ;
         rc = SDBCM_NODE_NOTEXISTED ;
         goto error ;
      }
      else if ( rc )
      {
         PD_LOG ( PDERROR, "Can not find old service config file" ) ;
         goto error ;
      }

      rc = sdbAdd ( arg2, arg3 ) ;
      if ( SDB_OK == rc )
         ossDelete ( confReserved ) ;
      else
      {
         PD_LOG ( PDERROR, "Failed to modify specified node" ) ;
         ossRenamePath ( confReserved, confpath ) ;
         goto error ;
      }

   done:
      PD_TRACE_EXITRC( SDB_SDBMODIFY, rc );
      return rc ;
   error:
      goto done ;
   }

   PD_TRACE_DECLARE_FUNCTION ( SDB_SENDRETCODE, "sendRetCode" )
   INT32 sendRetCode ( SINT32 retCode, ossSocket *sock )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB_SENDRETCODE );
      PD_TRACE1 ( SDB_SENDRETCODE, PD_PACK_INT(retCode) );
      CHAR* pBuffer = NULL ;
      INT32 pBufferSize = 0 ;

      rc = msgBuildReplyMsg ( &pBuffer, &pBufferSize, OP_CM,
                               retCode, 0, 0, 0, 0, (BSONObj*)NULL ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to build reply message") ;
         goto error ;
      }

      rc = cmSend ( pBuffer, pBufferSize, sock ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to send reply message") ;
         goto error ;
      }

   done:
      if ( pBuffer )
         SDB_OSS_FREE ( pBuffer ) ;
      PD_TRACE_EXITRC ( SDB_SENDRETCODE, rc );
      return rc ;
   error:
      goto done ;
   }

   PD_TRACE_DECLARE_FUNCTION ( SDB_CMRECV, "cmRecv" )
   INT32 cmRecv ( CHAR *pBuffer, INT32 recvSize,
                  ossSocket *sock )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB_CMRECV );
      SDB_ASSERT ( sock, "Socket is NULL" ) ;
      INT32 receivedSize = 0 ;
      INT32 totalReceivedSize = 0 ;
      while ( TRUE )
      {
         rc = sock->recv ( &pBuffer[totalReceivedSize],
                           recvSize-totalReceivedSize,
                           receivedSize ) ;
         if ( SDB_TIMEOUT == rc )
         {
            totalReceivedSize += receivedSize ;
            continue ;
         }
         PD_TRACE_EXITRC ( SDB_CMRECV, rc );
         return rc ;
      }
   }

   PD_TRACE_DECLARE_FUNCTION ( SDB_CMSEND, "cmSend" )
   INT32 cmSend ( const CHAR *pBuffer, INT32 sendSize,
                      ossSocket *sock )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB_CMSEND );
      SDB_ASSERT ( sock, "Socket is NULL" ) ;
      INT32 sentSize = 0 ;
      INT32 totalSentSize = 0 ;
      while ( TRUE )
      {
         rc = sock->send ( &pBuffer[totalSentSize],
                           sendSize-totalSentSize,
                           sentSize ) ;
         totalSentSize += sentSize ;
         if ( SDB_TIMEOUT == rc )
         {
            continue ;
         }
         PD_TRACE_EXITRC ( SDB_CMSEND, rc );
         return rc ;
      }
   }

   PD_TRACE_DECLARE_FUNCTION ( SDB_PIDMONITOR, "pidMonitor" )
   void pidMonitor ()
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB_PIDMONITOR );

      ossLatch ( &listLocker ) ;
      map<string, struct Process>::iterator it = svcList.begin();
      while ( it != svcList.end())
      {
         const string &svcname = it->first ;
         PD_TRACE1 ( SDB_PIDMONITOR, PD_PACK_STRING(svcname.c_str()) );
         struct Process &proc = it->second ;
         if ( OSS_BIT_TEST ( proc.status, BIT_STARTING ) ||
              OSS_BIT_TEST ( proc.status, BIT_RESTARTING ) )
         { // if status starting or restarting, do nothing
            ++it ;
            continue ;
         }
         else if ( !ossIsProcessRunning ( proc.pid ) )
         { // if status running( or restarting failed ) && not exist pid
            // check restarting count
            if ( resCount > 0 )
            {
               // get the starting time of this node
               queue<time_t> &startTime = proc.startTime ;
               INT32 size = startTime.size() ;
               if ( size >= ( resCount + 1 ) )
               { // if count of starting greater than count of restarting plus first starting
                  // only save the recent ( resCount + 1 ) records
                  for ( INT32 i = 0; i < size - ( resCount + 1 ); ++i )
                     startTime.pop() ;
                  if ( resInterval <= 0 ||
                       ( ( startTime.back() - startTime.front() )/60 ) <= resInterval )
                  { // if resInterval <= 0 or time of interval <= resInterval
                     svcList.erase ( it++ ) ;
                     PD_LOG ( PDEVENT, "Sequoiadb restart time out, svcname = %s",
                              svcname.c_str() ) ;
                     continue ;
                  }
               } // if ( size >= ( resCount + 1 ) )
            } // if ( resCount > 0 )
            else if ( resCount == 0 )
            { // never restart
               svcList.erase ( it++ ) ;
               PD_LOG ( PDEVENT, "Sequoiadb restart time out, svcname = %s",
                        svcname.c_str() ) ;
               continue ;
            }

            po::options_description desc ( "Command options" ) ;
            po::variables_map vm ;
            PMD_ADD_PARAM_OPTIONS_BEGIN( desc )
               PMD_COMMANDS_OPTIONS
            PMD_ADD_PARAM_OPTIONS_END
            string conf = pmdConf ;
            conf += OSS_FILE_SEP + svcname + OSS_FILE_SEP PMD_DFT_CONF ;
            rc = readConfigureFile ( conf.c_str(), desc, vm ) ;
            if ( rc )
            {
               PD_LOG ( PDERROR, "Can not read configure file: %s",
                        conf.c_str() ) ;
               svcList.erase ( it++ ) ;
               continue ;
            }
            else if ( vm.count( PMD_OPTION_DBPATH ) )
            { // if exist "dbpath" in config file
               const CHAR *dbpath = vm[PMD_OPTION_DBPATH].as<string>().c_str() ;
               CHAR startupFile[OSS_MAX_PATHSIZE + 1];
               if ( NULL == dbpath )
               {
                  PD_LOG ( PDERROR, "Can not read dbpath from configure file: %s",
                           conf.c_str() ) ;
                  svcList.erase ( it++ ) ;
                  continue ;
               }
               rc = buildFullPath( dbpath, PMD_STARTUP_FILE_NAME,
                                OSS_MAX_PATHSIZE + 1, startupFile );
               if ( rc )
               {
                  PD_LOG ( PDERROR, "Invalid arguments", conf.c_str() ) ;
                  svcList.erase ( it++ ) ;
                  continue ;
               }
               rc = ossAccess ( startupFile ) ;
               if ( SDB_OK == rc )
               { // if we can find startup file, that means the engine was unexpectedly stopped
                  OSS_BIT_CLR_SET ( proc.status, BIT_RESTARTING ) ;
                  try
                  {
                     boost::thread sdbstart ( sdbStart2, svcname, TYPE_MONITOR ) ;
                     sdbstart.detach() ;
                  }
                  catch ( boost::exception& )
                  {
                     PD_LOG ( PDERROR, "Unknown thread exception" ) ;
                     // restore status to "RUNNING", detected the failure later
                     OSS_BIT_CLR_SET ( proc.status, BIT_RUNNING ) ;
                  }
               }
               else if ( SDB_FNE == rc )
               { // if we can't find the file, that means the engine was normally stopped
                  svcList.erase ( it++ ) ;
                  PD_LOG ( PDEVENT, "Sequoiadb stopped normally, svcname = %s",
                           svcname.c_str() ) ;
               }
               else
               { // permission error or system error
                  PD_LOG ( PDERROR, "Permission error or system error: rc = %d", rc ) ;
                  svcList.erase ( it++ ) ;
               }
            } // else if ( vm.count( PMD_OPTION_DBPATH ) )
            else
               ++it ;
         }// else if ( !ossIsProcessRunning ( proc.pid ) )
         else // if status running && exitst pid
            ++it ;
      }// while ( it != svcList.end())
      ossUnlatch ( &listLocker ) ;
      PD_TRACE_EXITRC ( SDB_PIDMONITOR, rc );
   }
   
   //PD_TRACE_DECLARE_FUNCTION ( SDB_RTNCM_INITENV, "initEnv" )
   INT32 initEnv( UINT16 &port )
   {
      SDB_ASSERT( port, "Invalid input" ) ;
      INT32 rc = SDB_OK ;
      //PD_TRACE_ENTRY ( SDB_RTNCM_INITENV );
      const CHAR *svcname = NULL;
      CHAR conf[OSS_MAX_PATHSIZE + 1] = { 0 } ;
      CHAR hostname[OSS_MAX_HOSTNAME + 6] = { 0 } ;
      po::options_description desc ( "Config options" ) ;
      po::variables_map vm ;

      rc = ossGetEWD ( conf, OSS_MAX_PATHSIZE ) ;
      if ( rc )
      {
         ossPrintf ( "Failed to get excutable file's working directory"OSS_NEWLINE ) ;
         goto error ;
      }

      rc = ossSocket::getHostName ( hostname, OSS_MAX_HOSTNAME ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to get hostname, rc=%d", rc ) ;
         goto error ;
      }
      ossStrncat ( hostname, SDBCM_CONF_PORT, sizeof(SDBCM_CONF_PORT) ) ;
      desc.add_options()
         (SDBCM_CONF_DFTPORT, po::value<string>(), "sdbcm default listening port")
         (hostname, po::value<string>(), "sdbcm specified listening port")
         (SDBCM_RESTART_COUNT, po::value<INT32>(), "sequoiadb node restart max count")
         (SDBCM_RESTART_INTERVAL, po::value<INT32>(), "sequoiadb node restart time interval")
         (SDBCM_AUTO_START, po::value<string>(), "start sequoiadb node automatically when CM start")
      ;

      // build pmd config file path
      rc = buildFullPath ( conf, PMD_DFT_CONF_PATH, OSS_MAX_PATHSIZE + 1, pmdConf ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Working directory too long" ) ;
         goto error ;
      }
      // build sdbstart program file path
      rc = buildFullPath ( conf, SDBSTARTPROG, OSS_MAX_PATHSIZE + 1, sdbstartExecName ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Working directory too long" ) ;
         goto error ;
      }
      // build sdbstop program file path
      rc = buildFullPath ( conf, SDBSTOPPROG, OSS_MAX_PATHSIZE + 1, sdbstopExecName ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Working directory too long" ) ;
         goto error ;
      }
      // build sdbcm config file path
      if ( ( ossStrlen ( conf ) + sizeof ( SDBCM_CONF_PATH ) + 1 ) >
                       OSS_MAX_PATHSIZE )
      {
         PD_LOG ( PDERROR, "Working directory too long" ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }
      ossStrncat( conf, OSS_FILE_SEP, 1 );
      ossStrncat( conf, SDBCM_CONF_PATH, sizeof( SDBCM_CONF_PATH ) );
      rc = readConfigureFile ( conf, desc, vm ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to read configure file, rc = %d", rc ) ;
         goto error ;
      }
      try
      {
         if ( vm.count(hostname) )
            svcname = vm[hostname].as<string>().c_str() ;
         else if ( vm.count(SDBCM_CONF_DFTPORT) )
            svcname = vm[SDBCM_CONF_DFTPORT].as<string>().c_str() ;
         if ( vm.count(SDBCM_RESTART_COUNT) )
            resCount = vm[SDBCM_RESTART_COUNT].as<INT32>();
         if ( vm.count(SDBCM_RESTART_INTERVAL) )
            resInterval = vm[SDBCM_RESTART_INTERVAL].as<INT32>();

         if ( vm.count( SDBCM_AUTO_START ))
         {
            const CHAR *pAutoStart = vm[ SDBCM_AUTO_START ].as<string>().c_str();
            ossStrToBoolean( pAutoStart, &autoStart );
         }
      }
      catch ( std::exception&)
      {
         PD_LOG ( PDERROR, "Failed to read configure file" ) ;
         rc = SDB_IO ;
         goto error ;
      }
      if ( svcname != NULL )
      {
         rc = ossSocket::getPort( svcname, port ) ;
         if ( rc )
         {
            PD_LOG ( PDERROR, "Bad sdbcm listening service name: %s", svcname ) ;
            goto error ;
         }
      }
      PD_TRACE4 ( SDB_RTNCM_INITENV, PD_PACK_STRING(pmdConf), PD_PACK_STRING(_pdDiagLogPath),
                  PD_PACK_STRING(sdbstartExecName), PD_PACK_STRING(sdbstopExecName) );

   done:
      return rc ;
   error:
      goto done ;   
   }


#if defined (_WINDOWS)
   PD_TRACE_DECLARE_FUNCTION ( SDB_SDBCM_WINGETPROCESSINFOBYSVCNAME, "getProcessInfoBySvcname" )
   INT32 getProcessInfoBySvcname( const string &svcName,
                              struct Process &processInfo )
   {
      INT32 rc = SDB_OK;
      PD_TRACE_ENTRY ( SDB_SDBCM_WINGETPROCESSINFOBYSVCNAME );
      vector<string> names;
      OSSNPIPE handle;
      OSSPID pid;
      INT64 readSize = 0;
      CHAR enginePipeName [ PROC_PIPE_NAME_LEN + 1 ] = {0};
      BOOLEAN isOpen = FALSE;
      ossSnprintf ( enginePipeName, PROC_PIPE_NAME_LEN, ENGINE_NPIPE_PATTERN,
                    svcName.c_str() ) ;
      rc = ossEnumNamedPipes ( names, enginePipeName );
      PD_RC_CHECK( rc, PDERROR,
                  "failed to get the process name(rc=%d)",
                  rc );
      if ( names.size() != 1 )
      {
         if ( names.size() == 0 )
         {
            rc = SDBCM_NODE_NOTEXISTED;
            goto done;
         }
         PD_LOG( PDWARNING, "named pipe conflict(name:%s)",
               enginePipeName );
      }

      rc = ossOpenNamedPipe( enginePipeName, OSS_NPIPE_DUPLEX | OSS_NPIPE_BLOCK,
                           OSS_NPIPE_BLOCK_WITH_TIMEOUT, handle );
      if ( rc && SDB_FE != rc )
      {
         PD_LOG ( PDERROR, "failed to create named pipe: %s(rc=%d)",
               enginePipeName, rc );
         goto error;
      }
      isOpen = TRUE;
      rc = ossWriteNamedPipe( handle, ENGINE_NPIPE_MSG_PID,
                              sizeof(ENGINE_NPIPE_MSG_PID),
                              NULL );
      if ( rc )
      {
         PD_LOG ( PDERROR, "failed to send "ENGINE_NPIPE_MSG_PID" to %s, rc=%d",
                  enginePipeName, rc );
      }
      rc = ossReadNamedPipe( handle, (CHAR *)&pid, sizeof(pid), &readSize,
                           LIST_TIMEOUT );
      PD_RC_CHECK( rc, PDERROR, "Failed to read pid from pipe %s",
                  enginePipeName );
      PD_CHECK( sizeof( pid ) == readSize, SDB_SYS, error, PDERROR,
               "Failed to read pid from pipe %s", enginePipeName );
      processInfo.pid = pid;
      processInfo.status = BIT_RUNNING;
   done:
      if ( isOpen )
      {
         ossCloseNamedPipe( handle );
      }
      PD_TRACE_EXITRC ( SDB_SDBCM_WINGETPROCESSINFOBYSVCNAME, rc );
      return rc;
   error:
      goto done;
   }

   PD_TRACE_DECLARE_FUNCTION ( SDB_SDBCM_WINGETSVCLSTFROMCFG, "getSvcLstFromCfg" )
   INT32 getSvcLstFromCfg( vector< string > &svcnameLst )
   {
      INT32 rc = SDB_OK;
      PD_TRACE_ENTRY ( SDB_SDBCM_WINGETSVCLSTFROMCFG );
      BOOLEAN isOpen = FALSE;
      HANDLE hFile = NULL;
      WIN32_FIND_DATAA fd = {0};
      CHAR searchPath[OSS_MAX_PATHSIZE + 3] ;
      INT32 len = ossStrlen( pmdConf );
      ossStrncpy( searchPath, pmdConf, OSS_MAX_PATHSIZE );
      len = len < OSS_MAX_PATHSIZE ? len : OSS_MAX_PATHSIZE ;
      if ( searchPath[ len - 1 ] != '//' )
      {
         strcat( searchPath, "//" );
      }
      strcat( searchPath, "*" );
      try
      {
         hFile = FindFirstFileA( searchPath, &fd );
         if ( hFile != INVALID_HANDLE_VALUE )
         {
            isOpen = TRUE;
            do
            {
               // ignore the normal file, "." and ".."
               if ( !( fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
                  || ossStrcmp( fd.cFileName, "." ) == 0
                  || ossStrcmp( fd.cFileName, ".." ) == 0 )
               {
                  continue;
               }
               svcnameLst.push_back( fd.cFileName );
            }while( FindNextFileA( hFile, &fd ));
         }
      }
      catch ( std::exception & e)
      {
         rc = SDB_IO;
         PD_RC_CHECK( rc, PDERROR,
                     "occured unexpected error(%s)",
                     e.what() );
      }
   done:
      if ( isOpen )
      {
         FindClose( hFile );
         hFile = NULL;
      }
      PD_TRACE_EXITRC ( SDB_SDBCM_WINGETSVCLSTFROMCFG, rc );
      return rc;
   error:
      goto done;
   }

#elif defined (_LINUX)
   PD_TRACE_DECLARE_FUNCTION ( SDB_SDBCM_LINUXGETPROCESSINFOBYSVCNAME, "getProcessInfoBySvcname" )
   INT32 getProcessInfoBySvcname( const string &svcName,
                              struct Process &processInfo )
   {
      INT32 rc = SDB_OK;
      PD_TRACE_ENTRY ( SDB_SDBCM_LINUXGETPROCESSINFOBYSVCNAME );
      DIR *pDir;
      struct dirent *pDirent;
      CHAR engineName [ PROC_PATH_LEN_MAX + 1 ] = {0};
      BOOLEAN isOpen = FALSE;
      pDir = opendir( PROC_PATH );
      PD_CHECK( pDir != NULL, SDB_IO, error, PDERROR,
               "failed to open the directory:%s, errno=%d",
               PROC_PATH, ossGetLastError() );
      isOpen = TRUE;
      ossSnprintf ( engineName, PROC_PATH_LEN_MAX, ENGINE_NAME_PATTERN,
                    svcName.c_str() ) ;
      while( (pDirent = readdir( pDir )) != NULL )
      {
         CHAR pathName[ PROC_PATH_LEN_MAX + 1 ] = {0};
         ossSnprintf( pathName, PROC_PATH_LEN_MAX, PROC_CMDLINE_PATH_FORMAT,
                     pDirent->d_name );
         FILE *fp = NULL;
         fp = fopen( pathName, "r" );
         if ( !fp )
         {
            continue;
         }
         CHAR commandLine[ PROC_PATH_LEN_MAX + 1 ] = {0};
         CHAR *pTmp = fgets ( commandLine, PROC_PATH_LEN_MAX, fp );
         fclose(fp);
         if ( NULL == pTmp )
         {
            continue;
         }
         if ( 0 != ossStrcmp( commandLine, engineName ))
         {
            continue;
         }
         processInfo.pid = ossAtoi( pDirent->d_name );
         processInfo.status = BIT_RUNNING;
         break;
      }
      if ( NULL == pDirent )
      {
         rc = SDBCM_NODE_NOTEXISTED;
      }
   done:
      if ( isOpen )
      {
         closedir( pDir );
      }
      PD_TRACE_EXITRC ( SDB_SDBCM_LINUXGETPROCESSINFOBYSVCNAME, rc );
      return rc;
   error:
      goto done;
   }

   PD_TRACE_DECLARE_FUNCTION ( SDB_SDBCM_LINUXGETSVCLSTFROMCFG, "getSvcLstFromCfg" )
   INT32 getSvcLstFromCfg( vector< string > &svcnameLst )
   {
      INT32 rc = SDB_OK;
      PD_TRACE_ENTRY ( SDB_SDBCM_LINUXGETSVCLSTFROMCFG );
      BOOLEAN isOpen = FALSE;
      DIR         *pDir;
      struct dirent *pDirent;
      struct stat statBuf;
      pDir = opendir( pmdConf );
      PD_CHECK( pDir != NULL, SDB_IO, error, PDERROR,
               "failed to open the directory:%s, errno=%d",
               pmdConf, ossGetLastError() );
      isOpen = TRUE;
      while( (pDirent = readdir( pDir )) != NULL )
      {
         CHAR fullName[OSS_MAX_PATHSIZE + 1]={0};
         strncpy( fullName, pmdConf, OSS_MAX_PATHSIZE );
         strncat( fullName, OSS_FILE_SEP, OSS_MAX_PATHSIZE - ossStrlen(fullName) );
         strncat( fullName, pDirent->d_name, OSS_MAX_PATHSIZE - ossStrlen(fullName) );
         if( stat( fullName, &statBuf ) == -1 )
         {
            rc = SDB_SYS;
            PD_RC_CHECK( rc, PDERROR,
                        "failed to get the status of the file:%s",
                        fullName );
         }

         // ignore the normal file , "." and ".."
         if ( !S_ISDIR( statBuf.st_mode )
            || ossStrcmp( pDirent->d_name, "." ) == 0
            || ossStrcmp( pDirent->d_name, ".." ) == 0 )
         {
            continue;
         }
         svcnameLst.push_back( pDirent->d_name );
      }
   done:
      if ( isOpen )
      {
         closedir( pDir );
      }
      PD_TRACE_EXITRC ( SDB_SDBCM_LINUXGETSVCLSTFROMCFG, rc );
      return rc;
   error:
      goto done;
   }

#else

   INT32 getProcessInfoBySvcname( const string &svcName,
                              struct Process &processInfo )
   {
      SDB_ASSERT( FALSE, "Unrealized" );
      return SDB_OK;
   }
   INT32 getSvcLstFromCfg( vector< string > &svcnameLst )
   {
      SDB_ASSERT( FALSE, "Unrealized" );
      return SDB_OK;
   }

#endif

   //PD_TRACE_DECLARE_FUNCTION ( SDB_SDBCM_STARTALLNODES, "startAllNodes" )
   INT32 startAllNodes()
   {
      INT32 rc = SDB_OK;
      //PD_TRACE_ENTRY ( SDB_SDBCM_STARTALLNODES );
      UINT32 i = 0;
      vector< string > svcnameLst;
      //vector< boost::thread * > thrdLst;
      struct Process processInfo;
      rc = getSvcLstFromCfg( svcnameLst );
      PD_RC_CHECK( rc, PDERROR,
                  "failed to get node list(rc=%d)",
                  rc );
      for( ; i < svcnameLst.size(); i++ )
      {
         rc = getProcessInfoBySvcname( svcnameLst[i], processInfo );
         if ( rc )
         {
            if ( SDBCM_NODE_NOTEXISTED == rc )
            {
               rc = SDB_OK;
               PD_LOG( PDEVENT,
                     "start the node(svcname=%s)",
                     svcnameLst[i].c_str() );
               /*boost::thread *pthrd = new boost::thread( boost::bind( &sdbStart2, svcnameLst[i], TYPE_MONITOR ) );
               thrdLst.push_back( pthrd );*/
               boost::thread thrd( boost::bind( &sdbStart2, svcnameLst[i], TYPE_MONITOR ) );
               thrd.detach();
            }
            else
            {
               PD_LOG( PDERROR,
                     "failed to get process info(svcname=%s)",
                     svcnameLst[i].c_str() );
            }
         }
         else
         {
            svcList[svcnameLst[i]] = processInfo;
            PD_LOG( PDEVENT,
                  "node(svcname=%s) has already started, do nothing!",
                  svcnameLst[i].c_str() );
         }
      }
      /*for ( i = 0; i < thrdLst.size(); i++ )
      {
         thrdLst[i]->join();
         delete thrdLst[i];
      }*/
   done:
      //PD_TRACE_EXITRC ( SDB_SDBCM_STARTALLNODES, rc );
      return rc;
   error:
      goto done;
   }

   cCMService::cCMService()
   {
   }

   const CHAR *cCMService::getProgramName()
   {
      return SDBCM_EXE_FILE_NAME;
   }

   const CHAR *cCMService::getArguments()
   {
      return SDBCM_EXE_FILE_NAME;
   }

   //PD_TRACE_DECLARE_FUNCTION ( SDB_RTNCMSVC_DMNMAIN, "cCMService::dmnMain" )
   INT32 cCMService::svcMain( INT32 argc, CHAR **argv )
   {
      INT32 rc = SDB_OK;
      //PD_TRACE_ENTRY ( SDB_RTNCMSVC_DMNMAIN );
      UINT16 port = SDBCM_DFT_PORT ;
      rc = initEnv ( port ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to initialize environment, rc = %d", rc ) ;
         goto error ;
      }
      //PD_TRACE1 ( SDB_RTNCMSVC_DMNMAIN, PD_PACK_USHORT(port) );
      try
      {
         boost::thread listener ( cmTcpListener, port ) ;
         listener.detach () ;
      }
      catch ( boost::exception& )
      {
         rc = SDB_SYS ;
         PD_LOG ( PDERROR, "Failed to new cmTcpListener thread" ) ;
         goto error ;
      }
      if ( autoStart )
      {
         rc = startAllNodes();
         PD_RC_CHECK( rc, PDERROR,
                     "failed to start nodes(rc=%d)",
                     rc );
      }
#if defined (_LINUX)
      {
         // once cmTcpListener is successfully started, we can rename the process
         CHAR sdbcmProcessName[SDBCM_NAME_BUF_LEN + 1] = {0} ;
         ossSnprintf ( sdbcmProcessName, SDBCM_NAME_BUF_LEN,
                       SDBCM_NAME_PATTERN, port ) ;
         ossEnableNameChanges ( argc, argv ) ;
         ossRenameProcess ( sdbcmProcessName ) ;
      }
#endif
      while ( isRunning() )
      {
         pidMonitor () ;
         ossSleepsecs ( SLEEPTIME ) ;
      }
   done:
      //PD_TRACE_EXITRC ( SDB_RTNCMSVC_DMNMAIN, rc );
      return rc;
   error:
      goto done;
   }

   INT32 cCMService::init()
   {
      INT32 rc = SDB_OK;
      CHAR logDir[ OSS_MAX_PATHSIZE + 1 ] = {0};
      UINT32 len = 0;
      rc = ossGetEWD( logDir, OSS_MAX_PATHSIZE );
      PD_RC_CHECK( rc, PDERROR,
                  "failed to get working directory(rc=%d)",
                  rc );
      len = ossStrlen( logDir ) + 1 + ossStrlen( SDBCM_LOG_DIR );
      PD_CHECK( len <= OSS_MAX_PATHSIZE, SDB_INVALIDARG, error,
               PDERROR, "log-path longer than %d", OSS_MAX_PATHSIZE );
      ossStrncat( logDir, OSS_FILE_SEP, 1 );
      ossStrncat( logDir, SDBCM_LOG_DIR, ossStrlen(SDBCM_LOG_DIR) );
#if defined (_WINDOWS)
      rc = iPmdDMNChildProc::init("50010", logDir );
#elif defined (_LINUX)
      rc = iPmdDMNChildProc::init(50010, logDir );
#endif
   done:
      return rc;
   error:
      goto done;
   }
}
