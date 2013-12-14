/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = utilRemoteExec.cpp

   Descriptive Name = Remote Excuting

   When/how to use: this program may be used on binary and text-formatted
   versions of OSS component. This file contains declares for process op.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          12/24/2012  TW  Initial Draft

   Last Changed =

*******************************************************************************/

#include "ossSocket.hpp"
#include "ossMem.hpp"
#include "ossProc.hpp"
#include "msgMessage.hpp"
#include "utilRemoteExec.hpp"
#include "sdbcm.hpp"
#include "pdTrace.hpp"
#include "utilTrace.hpp"
#include <boost/program_options.hpp>
#include <boost/program_options/parsers.hpp>

namespace po = boost::program_options;

// we may have potential "hang" issue here.
// since we perform infinite loop in recv and send, if the network is broken we
// are never going to exit the loop ( even if ctrl-c ) since we don't have
// access to krcb to detect database status here.
INT32 recv ( CHAR *pBuffer, INT32 recvSize,
             ossSocket *sock )
{
   INT32 rc = SDB_OK ;
   SDB_ASSERT ( sock, "Socket is NULL" ) ;
   INT32 receivedLen = 0 ;
   INT32 totalReceivedLen = 0 ;
   while ( TRUE )
   {
      rc = sock->recv ( &pBuffer[totalReceivedLen],
                        recvSize - totalReceivedLen,
                        receivedLen ) ;
      totalReceivedLen += receivedLen ;
      if ( SDB_TIMEOUT == rc )
      {
         continue ;
      }
      return rc ;
   }
}

INT32 send ( const CHAR *pBuffer, INT32 sendSize,
             ossSocket *sock )
{
   INT32 rc = SDB_OK ;
   SDB_ASSERT ( sock, "Socket is NULL" ) ;
   INT32 sentLen = 0 ;
   INT32 totalSentLen = 0 ;
   while ( TRUE )
   {
      rc = sock->send ( &pBuffer[totalSentLen],
                        sendSize - totalSentLen,
                        sentLen ) ;
      totalSentLen += sentLen ;
      if ( SDB_TIMEOUT == rc )
         continue ;
      return rc ;
   }
}

PD_TRACE_DECLARE_FUNCTION ( SDB_REMOTEEXEC_RDCFGFILE, "readConfigureFile")
INT32 readConfigureFile( const CHAR *conf,
                         po::options_description &desc,
                         po::variables_map &vm )
{
   INT32 rc = SDB_OK;
   PD_TRACE_ENTRY ( SDB_REMOTEEXEC_RDCFGFILE ) ;
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
   PD_TRACE_EXITRC( SDB_REMOTEEXEC_RDCFGFILE, rc ) ;
   return rc;
error:
   goto done;
}

PD_TRACE_DECLARE_FUNCTION ( SDB_REMOTEEXEC, "utilRemoteExec" )
INT32 utilRemoteExec ( SINT32 remoCode,
                      const CHAR * hostname,
                      SINT32 *retCode,
                      BSONObj *arg1,
                      BSONObj *arg2,
                      BSONObj *arg3,
                      BSONObj *arg4 )
{
   SDB_ASSERT ( retCode && hostname , "Invalid input" ) ;
   INT32 rc = SDB_OK ;
   PD_TRACE_ENTRY ( SDB_REMOTEEXEC ) ;
   PD_TRACE1 ( SDB_REMOTEEXEC,
               PD_PACK_INT ( remoCode ) ) ;
   CHAR *pCMRequest = NULL ;
   CHAR *pReceiveBuffer = NULL ;
   CHAR conf[OSS_MAX_PATHSIZE + 1] = { 0 } ;
   const CHAR *svcname = NULL ;
   UINT16 port = SDBCM_DFT_PORT ;
   INT32 reqSize = 0 ;
   SINT32 packetLength = 0 ;

   po::options_description desc ( "Config options" ) ;
   po::variables_map vm ;
   CHAR hostname2[OSS_MAX_HOSTNAME + 6] = { 0 } ;
   if ( ossStrlen(hostname) > OSS_MAX_HOSTNAME )
   {
      PD_LOG ( PDERROR, "Invalid host name: %s", hostname ) ;
      rc = SDB_INVALIDARG ;
      goto error ;
   }
   ossMemcpy( hostname2, hostname, ossStrlen( hostname ) );
   ossStrncat ( hostname2, SDBCM_CONF_PORT, ossStrlen(SDBCM_CONF_PORT) ) ;

   desc.add_options()
      (SDBCM_CONF_DFTPORT, po::value<string>(), "sdbcm default listening port")
      (hostname2, po::value<string>(), "sdbcm specified listening port")
   ;

   rc = ossGetEWD ( conf, OSS_MAX_PATHSIZE ) ;
   if ( rc )
   {
      PD_LOG ( PDERROR, "Failed to get excutable file's working directory" ) ;
      goto error ;
   }
   if ( ( ossStrlen ( conf ) + ossStrlen ( SDBCM_CONF_PATH ) + 2 ) >
                    OSS_MAX_PATHSIZE )
   {
      PD_LOG ( PDERROR, "Working directory too long" ) ;
      rc = SDB_INVALIDARG ;
      goto error ;
   }
   ossStrncat( conf, OSS_FILE_SEP, 1 );
   ossStrncat( conf, SDBCM_CONF_PATH, ossStrlen( SDBCM_CONF_PATH ) );
   rc = readConfigureFile ( conf, desc, vm ) ;
   if ( rc )
   {
      PD_LOG ( PDERROR, "Failed to read configure file, rc = %d", rc ) ;
      goto error ;
   }
   else if ( vm.count(hostname2) )
      svcname = vm[hostname2].as<string>().c_str() ;
   else if ( vm.count(SDBCM_CONF_DFTPORT) )
      svcname = vm[SDBCM_CONF_DFTPORT].as<string>().c_str() ;
   if ( svcname != NULL )
   {
      rc = ossSocket::getPort( svcname, port ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Bad sdbcm listening service name: %s", svcname ) ;
         goto error ;
      }
   }

   {
      ossSocket sock ( hostname, port, OSS_SOCKET_DFT_TIMEOUT ) ;
      rc = sock.initSocket () ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed initialize socket, rc=%d", rc ) ;
         goto error ;
      }
      rc = sock.connect () ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed connect remote server, rc=%d", rc ) ;
         goto error ;
      }

      // build message
      rc = msgBuildCMRequest ( &pCMRequest, &reqSize, remoCode,
                                arg1, arg2, arg3, arg4 ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to build cm request message, rc=%d", rc ) ;
         goto error ;
      }

      // send message
      rc = send ( pCMRequest, reqSize, &sock ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to send cm request message, rc=%d", rc ) ;
         goto error ;
      }

      // receive message
      rc = recv ( (CHAR*)&packetLength, sizeof (SINT32), &sock ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to receive cm reply message, rc=%d", rc ) ;
         goto error ;
      }
      // free at the end of this function
      pReceiveBuffer = (CHAR*)SDB_OSS_MALLOC ( sizeof(CHAR) * (packetLength) ) ;
      if ( !pReceiveBuffer )
      {
         rc = SDB_OOM ;
         PD_LOG ( PDERROR, "Failed to allocate %d bytes receive buffer",
                  packetLength ) ;
         goto error ;
      }
      *(SINT32*)(pReceiveBuffer) = packetLength ;
      rc = recv ( &pReceiveBuffer[sizeof (SINT32)],
                   packetLength-sizeof (SINT32), &sock ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to receive cm reply message, rc=%d", rc ) ;
         goto error ;
      }
   }

   {
      vector<BSONObj> objList ;
      SINT64 contextID  = 0 ;
      SINT32 startFrom = 0 ;
      SINT32 numReturned = 0 ;
      // extract message
      rc = msgExtractReply ( pReceiveBuffer, retCode,
                             &contextID, &startFrom, &numReturned, objList ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to extract cm reply message, rc=%d", rc ) ;
         goto error ;
      }
   }
done:
   if ( pCMRequest )
      SDB_OSS_FREE ( pCMRequest ) ;
   if ( pReceiveBuffer )
      SDB_OSS_FREE ( pReceiveBuffer ) ;
   PD_TRACE_EXITRC ( SDB_REMOTEEXEC, rc ) ;
   return rc ;
error:
   goto done ;
}
