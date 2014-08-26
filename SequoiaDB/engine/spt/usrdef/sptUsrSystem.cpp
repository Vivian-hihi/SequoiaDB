/*******************************************************************************

   Copyright (C) 2011-2014 SequoiaDB Ltd.

   This program is free software: you can redistribute it and/or modify
   it under the term of the GNU Affero General Public License, version 3,
   as published by the Free Software Foundation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warrenty of
   MARCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU Affero General Public License for more details.

   You should have received a copy of the GNU Affero General Public License
   along with this program. If not, see <http://www.gnu.org/license/>.

   Source File Name = sptUsrSystem.cpp

   Descriptive Name =

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          09/14/2012  YW  Initial Draft

   Last Changed =

*******************************************************************************/

#include "sptUsrSystem.hpp"
#include "sptCmdRunner.hpp"
#include "ossUtil.hpp"
#include "utilStr.hpp"
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#if defined (_LINUX)
#include <net/if.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#endif


using namespace bson ;

namespace engine
{
   JS_STATIC_FUNC_DEFINE( _sptUsrSystem, ping )
   JS_STATIC_FUNC_DEFINE( _sptUsrSystem, type )
   JS_STATIC_FUNC_DEFINE( _sptUsrSystem, getReleaseInfo )
   JS_STATIC_FUNC_DEFINE( _sptUsrSystem, getHostsMap )
   JS_STATIC_FUNC_DEFINE( _sptUsrSystem, getCpuInfo )
   JS_STATIC_FUNC_DEFINE( _sptUsrSystem, getMemInfo )
   JS_STATIC_FUNC_DEFINE( _sptUsrSystem, getDiskInfo )
   JS_STATIC_FUNC_DEFINE( _sptUsrSystem, getNetcardInfo )
   JS_STATIC_FUNC_DEFINE( _sptUsrSystem, getIpTablesInfo )
   JS_STATIC_FUNC_DEFINE( _sptUsrSystem, getHostName )
   JS_STATIC_FUNC_DEFINE( _sptUsrSystem, help )

   JS_BEGIN_MAPPING( _sptUsrSystem, "System" )
      JS_ADD_STATIC_FUNC( "ping", ping )
      JS_ADD_STATIC_FUNC( "type", type )
      JS_ADD_STATIC_FUNC( "getReleaseInfo", getReleaseInfo )
      JS_ADD_STATIC_FUNC( "getHostsMap", getHostsMap )
      JS_ADD_STATIC_FUNC( "getCpuInfo", getCpuInfo )
      JS_ADD_STATIC_FUNC( "getMemInfo", getMemInfo )
      JS_ADD_STATIC_FUNC( "getDiskInfo", getDiskInfo )
      JS_ADD_STATIC_FUNC( "getNetcardInfo", getNetcardInfo )
      JS_ADD_STATIC_FUNC( "getIpTablesInfo", getIpTablesInfo )
      JS_ADD_STATIC_FUNC( "getHostName", getHostName )
      JS_ADD_STATIC_FUNC( "help", help )
   JS_MAPPING_END()

   INT32 _sptUsrSystem::ping( const _sptArguments &arg,
                              _sptReturnVal &rval,
                              bson::BSONObj &detail )
   {
      INT32 rc = SDB_OK ;
      BSONObjBuilder builder ;
      string host ;
      stringstream cmd ;
      _sptCmdRunner runner ;
      UINT32 exitCode = 0 ;
      rc = arg.getString( 0, host ) ;
      if ( SDB_OUT_OF_BOUND == rc )
      {
         detail = BSON( SPT_ERR << "hostname must be config" ) ;
      }
      else if ( rc )
      {
         detail = BSON( SPT_ERR << "hostname must be string" ) ;
      }
      PD_RC_CHECK( rc, PDERROR, "Failed to get hostname, rc: %d", rc ) ;

#if defined (_LINUX)
      cmd << "ping " << host << " -q -c 1" ;
      rc = runner.exec( cmd.str().c_str(), exitCode ) ;
#elif defined (_WINDOWS)
      rc = SDB_SYS ;
#endif
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to exec cmd, rc:%d, exit:%d",
                 rc, exitCode ) ;
         if ( SDB_OK == rc )
         {
            rc = SDB_SYS ;
         }
         stringstream ss ;
         ss << "failed to exec cmd \"ping\",rc:"
            << rc
            << ",exit:"
            << exitCode ;
         detail = BSON( SPT_ERR << ss.str() ) ;
         goto error ;
      }

      builder.append( SPT_USR_SYSTEM_TARGET, host ) ;
      builder.appendBool( SPT_USR_SYSTEM_REACHABLE, SDB_OK == exitCode ) ;
      rval.setStringVal( "", builder.obj().toString( FALSE, TRUE ).c_str() ) ;
   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _sptUsrSystem::type( const _sptArguments &arg,
                              _sptReturnVal &rval,
                              bson::BSONObj &detail )
   {
      INT32 rc = SDB_OK ;
      if ( 0 < arg.argc() )
      {
         PD_LOG( PDERROR, "type() should have non arguments" ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }

#if defined (_LINUX)
      rval.setStringVal( "", "LINUX") ;
#elif defined (_WINDOWS)
      rval.setStringVal( "", "WINDOWS" ) ;
#endif
   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _sptUsrSystem::getReleaseInfo( const _sptArguments &arg,
                                        _sptReturnVal &rval,
                                        bson::BSONObj &detail )
   {
      INT32 rc = SDB_OK ;
      UINT32 exitCode = 0 ;
      _sptCmdRunner runner ;
      const SINT64 bufLen = 1024 * 5 ;
      SINT64 read = 0 ;
      CHAR buf[bufLen + 1] = { 0 } ;
      BSONObjBuilder builder ;

      if ( 0 < arg.argc() )
      {
         PD_LOG( PDERROR, "type() should have non arguments" ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }

#if defined (_LINUX)
      rc = runner.exec( "lsb_release -a |grep -v \"LSB Version\"", exitCode ) ;
#elif defined (_WINDOWS)
      rc = SDB_SYS ;
#endif
      if ( SDB_OK != rc || SDB_OK != exitCode )
      {
         PD_LOG( PDERROR, "failed to exec cmd, rc:%d, exit:%d",
                 rc, exitCode ) ;
         if ( SDB_OK == rc )
         {
            rc = SDB_SYS ;
         }
         stringstream ss ;
         ss << "failed to exec cmd \"lsb_release -a\",rc:"
            << rc
            << ",exit:"
            << exitCode ;
         detail = BSON( SPT_ERR << ss.str() ) ;
         goto error ;
      }

      rc = runner.read( buf, bufLen, read ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to read msg from cmd runner:%d", rc ) ;
         stringstream ss ;
         ss << "failed to read msg from cmd \"lsb_release -a\", rc:"
            << rc ;
         detail = BSON( SPT_ERR << ss.str() ) ;
         goto error ;
      }

      rc = _extractReleaseInfo( buf, builder ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to read msg from cmd runner:%d", rc ) ;
         stringstream ss ;
         ss << "failed to extract info from release info:"
            << buf ;
         detail = BSON( SPT_ERR << ss.str() ) ;
         goto error ;
      }

      ossMemset( buf, '\0', bufLen + 1 ) ;
      read = 0 ;
#if defined (_LINUX)
      rc = runner.exec( "uname -a", exitCode ) ;
#elif defined (_WINDOWS)
      rc = SDB_SYS ;
#endif
      if ( SDB_OK != rc || SDB_OK != exitCode )
      {
         PD_LOG( PDERROR, "failed to exec cmd, rc:%d, exit:%d",
                 rc, exitCode ) ;
         if ( SDB_OK == rc )
         {
            rc = SDB_SYS ;
         }
         stringstream ss ;
         ss << "failed to exec cmd \"uname -a\", rc:"
            << rc
            << ",exit:"
            << exitCode ;
         detail = BSON( SPT_ERR << ss.str() ) ;
         goto error ;
      }

      rc = runner.read( buf, bufLen, read ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to read msg from cmd runner:%d", rc ) ;
         stringstream ss ;
         ss << "failed to read msg from cmd \"uname -a\", rc:"
            << rc ;
         detail = BSON( SPT_ERR << ss.str() ) ;
         goto error ;
      }

      if ( NULL != ossStrstr( buf, "x86_64") )
      {
         builder.append( SPT_USR_SYSTEM_BIT, 64 ) ;
      }
      else
      {
         builder.append( SPT_USR_SYSTEM_BIT, 32 ) ;
      }

      rval.setStringVal( "", builder.obj().toString( FALSE, TRUE ).c_str() ) ;
   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _sptUsrSystem::_extractReleaseInfo( const CHAR *buf,
                                             bson::BSONObjBuilder &builder )
   {
      INT32 rc = SDB_OK ;
      vector<string> splited ;
      /// not performance sensitive.
      boost::algorithm::split( splited, buf, boost::is_any_of("\n:") ) ;
      vector<string>::iterator itr = splited.begin() ;
      const string *distributor = NULL ;
      const string *release = NULL ;
      for ( ; itr != splited.end(); itr++ )
      {
         if ( itr->empty() )
         {
            continue ;
         }
         boost::algorithm::trim( *itr ) ;
         if ( "Distributor ID" == *itr &&
              itr < splited.end() - 1 )
         {
            distributor = &( *( itr + 1 ) ) ;
         }
         else if ( "Release" == *itr &&
                   itr < splited.end() - 1 )
         {
            release = &( *( itr + 1 ) ) ;
         }
      }

      if ( NULL == distributor ||
           NULL == release )
      {
         PD_LOG( PDERROR, "failed to split release info:%s",
                 buf )  ;
         rc = SDB_SYS ;
         goto error ;
      }

      builder.append( SPT_USR_SYSTEM_DISTRIBUTOR, *distributor ) ;
      builder.append( SPT_USR_SYSTEM_RELASE, *release ) ;
   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _sptUsrSystem::getHostsMap( const _sptArguments &arg,
                                     _sptReturnVal &rval,
                                     bson::BSONObj &detail )
   {
      INT32 rc = SDB_OK ;
      UINT32 exitCode = 0 ;
      _sptCmdRunner runner ;
      const SINT64 bufLen = 1024 * 1024 ;
      SINT64 read = 0 ;
      CHAR buf[bufLen + 1] = { 0 } ;
      BSONObjBuilder builder ;

      if ( 0 < arg.argc() )
      {
         PD_LOG( PDERROR, "type() should have non arguments" ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }

#if defined (_LINUX)
      rc = runner.exec( "cat /etc/hosts", exitCode ) ;
#elif defined (_WINDOWS)
      rc = SDB_SYS ;
#endif
      if ( SDB_OK != rc || SDB_OK != exitCode )
      {
         PD_LOG( PDERROR, "failed to exec cmd, rc:%d, exit:%d",
                 rc, exitCode ) ;
         if ( SDB_OK == rc )
         {
            rc = SDB_SYS ;
         }
         stringstream ss ;
         ss << "failed to exec cmd \"cat /etc/hosts\",rc:"
            << rc
            << ",exit:"
            << exitCode ;
         detail = BSON( SPT_ERR << ss.str() ) ;
         goto error ;
      }

      rc = runner.read( buf, bufLen, read ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to read msg from cmd runner:%d", rc ) ;
         stringstream ss ;
         ss << "failed to read msg from cmd \"cat /etc/hosts\", rc:"
            << rc ;
         detail = BSON( SPT_ERR << ss.str() ) ;
         goto error ;
      }

      rc = _extractHosts( buf, builder ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to extract host from file:%d", rc ) ;
         stringstream ss ;
         ss << "failed to read msg from buf:"
            << buf ;
         detail = BSON( SPT_ERR << ss.str() ) ;
         goto error ;
      }

      rval.setStringVal( "", builder.obj().toString( FALSE, TRUE ).c_str() ) ;
   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _sptUsrSystem::_extractHosts( const CHAR *buf,
                                       bson::BSONObjBuilder &builder )
   {
      INT32 rc = SDB_OK ;
      BSONArrayBuilder arrBuilder ;
      vector<string> splited ;
      boost::algorithm::split( splited, buf, boost::is_any_of("\n") ) ;
      if ( splited.empty() )
      {
         goto done ;
      }

      for ( vector<string>::iterator itr = splited.begin() ;
            itr != splited.end() ;
            itr++ )
      {
         if ( itr->empty() )
         {
            continue ;
         }
         boost::algorithm::trim( *itr ) ;
         vector<string> columns ;
         boost::algorithm::split( columns, *itr, boost::is_any_of("\t ") ) ;

         for ( vector<string>::iterator itr2 = columns.begin();
               itr2 != columns.end();
                /// do not ++
               )
         {
            if ( itr2->empty() )
            {
               itr2 = columns.erase( itr2 ) ;
            }
            else
            {
               ++itr2 ;
            }
         }

         /// xxx.xxx.xxx.xxx xxxx
         /// xxx.xxx.xxx.xxx xxxx.xxxx xxxx
         if ( 2 != columns.size() && 3 != columns.size() )
         {
            continue ;
         }

         if ( !isValidIPV4( columns.at( 0 ).c_str() ) )
         {
            continue ;
         }

         arrBuilder << BSON( SPT_USR_SYSTEM_IP << columns.at( 0 )
                             << SPT_USR_SYSTEM_HOSTNAME
                             << columns.at( columns.size() - 1 ) ) ;
      }

      builder.append( SPT_USR_SYSTEM_HOSTS, arrBuilder.arr() ) ;
   done:
      return rc ;
   }

   INT32 _sptUsrSystem::getCpuInfo( const _sptArguments &arg,
                                    _sptReturnVal &rval,
                                    bson::BSONObj &detail )
   {
      INT32 rc = SDB_OK ;
      UINT32 exitCode = 0 ;
      _sptCmdRunner runner ;
      const SINT64 bufLen = 1024 * 100 ;
      SINT64 read = 0 ;
      CHAR buf[bufLen + 1] = { 0 } ;
      BSONObjBuilder builder ;
#define CPU_CMD "cat /proc/cpuinfo |grep name | cut -f2 -d: |uniq -c"
#if defined (_LINUX)
      rc = runner.exec( CPU_CMD, exitCode ) ;
#elif defined (_WINDOWS)
      rc = SDB_SYS ;
#endif
      if ( SDB_OK != rc || SDB_OK != exitCode )
      {
         PD_LOG( PDERROR, "failed to exec cmd, rc:%d, exit:%d",
                 rc, exitCode ) ;
         if ( SDB_OK == rc )
         {
            rc = SDB_SYS ;
         }
         stringstream ss ;
         ss << "failed to exec cmd \"CPU_CMD\",rc:"
            << rc
            << ",exit:"
            << exitCode ;
         detail = BSON( SPT_ERR << ss.str() ) ;
         goto error ;
      }

      rc = runner.read( buf, bufLen, read ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to read msg from cmd runner:%d", rc ) ;
         stringstream ss ;
         ss << "failed to read msg from cmd \"CPU_CMD\", rc:"
            << rc ;
         detail = BSON( SPT_ERR << ss.str() ) ;
         goto error ;
      }

      rc = _extractCpuInfo( buf, builder ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to extract cpu info:%d", rc ) ;
         stringstream ss ;
         ss << "failed to read msg from buf:"
            << buf ;
         detail = BSON( SPT_ERR << ss.str() ) ;
         goto error ;
      }

      {
      SINT64 user = 0 ;
      SINT64 sys = 0 ;
      SINT64 idle = 0 ;
      SINT64 other = 0 ;
      rc = ossGetCPUInfo( user, sys, idle, other ) ;
      if ( SDB_OK != rc )
      {
         goto error ;
      }

      builder.appendNumber( SPT_USR_SYSTEM_USER, user ) ;
      builder.appendNumber( SPT_USR_SYSTEM_SYS, sys ) ;
      builder.appendNumber( SPT_USR_SYSTEM_IDLE, idle ) ;
      builder.appendNumber( SPT_USR_SYSTEM_OTHER, other ) ;
      }
      rval.setStringVal( "", builder.obj().toString( FALSE, TRUE ).c_str() ) ;
   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _sptUsrSystem::_extractCpuInfo( const CHAR *buf,
                                         bson::BSONObjBuilder &builder )
   {
      INT32 rc = SDB_OK ;
      BSONArrayBuilder arrBuilder ;
      vector<string> splited ;
      boost::algorithm::split( splited, buf, boost::is_any_of("\n") ) ;
      for ( vector<string>::iterator itr = splited.begin();
            itr != splited.end();
            itr++ )
      {
         if ( itr->empty() )
         {
            continue ;
         }
         boost::algorithm::trim( *itr ) ;
         vector<string> columns ;
         boost::algorithm::split( columns, *itr, boost::is_any_of("\t ") ) ;
         /// eg: 4  Intel(R) Xeon(R) CPU E5-2620 0 @ 2.00GHz
         if ( columns.size() < 4 )
         {
            rc = SDB_SYS ;
            goto error ;
         }
         UINT32 coreNum = 0 ;
         stringstream info ;
         string *frequency = NULL ;
         try
         {
            coreNum = boost::lexical_cast<UINT32>( columns.at( 0 ) ) ;
         }
         catch ( std::exception &e )
         {
            PD_LOG( PDERROR, "unexpected err happened:%s", e.what() ) ;
            rc = SDB_SYS ;
            goto error ;
         }

         for ( UINT32 i = 1; i < columns.size(); i++ )
         {
            if ( "@" == columns.at( i ) )
            {
               if ( i == columns.size() - 2 )
               {
                  frequency = &( columns.at( columns.size() - 1 ) ) ;
                  break ;
               }
               else
               {
                  rc = SDB_SYS ;
                  goto error ;
               }
            }

            info << columns.at( i ) << " " ;
         }

         if ( NULL == frequency )
         {
            rc = SDB_SYS ;
            goto error ;
         }
         arrBuilder << BSON( SPT_USR_SYSTEM_CORE << coreNum
                             << SPT_USR_SYSTEM_INFO << info.str()
                             << SPT_USR_SYSTEM_FREQ << *frequency ) ;
      }

      builder.append( SPT_USR_SYSTEM_CPUS, arrBuilder.arr() ) ;
   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _sptUsrSystem::getMemInfo( const _sptArguments &arg,
                                    _sptReturnVal &rval,
                                    bson::BSONObj &detail )
   {
      INT32 rc = SDB_OK ;
      UINT32 exitCode = 0 ;
      _sptCmdRunner runner ;
      const SINT64 bufLen = 1024 ;
      SINT64 read = 0 ;
      CHAR buf[bufLen + 1] = { 0 } ;
      BSONObjBuilder builder ;
#if defined (_LINUX)
      rc = runner.exec( "free -m |grep Mem", exitCode ) ;
#elif defined (_WINDOWS)
      rc = SDB_SYS ;
#endif
      if ( SDB_OK != rc || SDB_OK != exitCode )
      {
         PD_LOG( PDERROR, "failed to exec cmd, rc:%d, exit:%d",
                 rc, exitCode ) ;
         if ( SDB_OK == rc )
         {
            rc = SDB_SYS ;
         }
         stringstream ss ;
         ss << "failed to exec cmd \"free\",rc:"
            << rc
            << ",exit:"
            << exitCode ;
         detail = BSON( SPT_ERR << ss.str() ) ;
         goto error ;
      }

      rc = runner.read( buf, bufLen, read ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to read msg from cmd runner:%d", rc ) ;
         stringstream ss ;
         ss << "failed to read msg from cmd \"free\", rc:"
            << rc ;
         detail = BSON( SPT_ERR << ss.str() ) ;
         goto error ;
      }

      rc = _extractMemInfo( buf, builder ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to extract mem info:%d", rc ) ;
         stringstream ss ;
         ss << "failed to read msg from buf:"
            << buf ;
         detail = BSON( SPT_ERR << ss.str() ) ;
         goto error ;
      }

      rval.setStringVal( "", builder.obj().toString( FALSE, TRUE ).c_str() ) ;
   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _sptUsrSystem::_extractMemInfo( const CHAR *buf,
                                         bson::BSONObjBuilder &builder )
   {
      INT32 rc = SDB_OK ;
      vector<string> splited ;
      boost::algorithm::split( splited, buf, boost::is_any_of("\t ") ) ;

      for ( vector<string>::iterator itr = splited.begin();
            itr != splited.end();
            /// do not ++   
          )
      {
         if ( itr->empty() )
         {
            itr = splited.erase( itr ) ;
         }
         else
         {
            ++itr ;
         }
      }
      /// Mem:       8194232    2373776    5820456          0     387924     992756
      /// choose total used free
      if ( splited.size() < 4 )
      {
         rc = SDB_SYS ;
         goto error ;
      }

      try
      {
         builder.append( SPT_USR_SYSTEM_SIZE,
                         boost::lexical_cast<UINT32>(splited.at( 1 ) ) ) ;
         builder.append( SPT_USR_SYSTEM_USED,
                         boost::lexical_cast<UINT32>(splited.at( 2 ) ) ) ;
         builder.append( SPT_USR_SYSTEM_FREE,
                         boost::lexical_cast<UINT32>(splited.at( 3) ) ) ;
         builder.append( SPT_USR_SYSTEM_UNIT, "M" ) ;
      }
      catch ( std::exception &e )
      {
         PD_LOG( PDERROR, "unexpected err happened:%s", e.what() ) ;
         rc = SDB_SYS ;
         goto error ;
      }
   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _sptUsrSystem::getDiskInfo( const _sptArguments &arg,
                                     _sptReturnVal &rval,
                                     bson::BSONObj &detail )
   {
      INT32 rc = SDB_OK ;
      UINT32 exitCode = 0 ;
      _sptCmdRunner runner ;
      const SINT64 bufLen = 1024 * 1024 ;
      SINT64 read = 0 ;
      CHAR buf[bufLen + 1] = { 0 } ;
      BSONObjBuilder builder ;
#if defined (_LINUX)
      rc = runner.exec( "df -m |grep -v \"Use%\"", exitCode ) ;
#elif defined (_WINDOWS)
      rc = SDB_SYS ;
#endif
      if ( SDB_OK != rc || SDB_OK != exitCode )
      {
         PD_LOG( PDERROR, "failed to exec cmd, rc:%d, exit:%d",
                 rc, exitCode ) ;
         if ( SDB_OK == rc )
         {
            rc = SDB_SYS ;
         }
         stringstream ss ;
         ss << "failed to exec cmd \"df\",rc:"
            << rc
            << ",exit:"
            << exitCode ;
         detail = BSON( SPT_ERR << ss.str() ) ;
         goto error ;
      }

      rc = runner.read( buf, bufLen, read ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to read msg from cmd runner:%d", rc ) ;
         stringstream ss ;
         ss << "failed to read msg from cmd \"df\", rc:"
            << rc ;
         detail = BSON( SPT_ERR << ss.str() ) ;
         goto error ;
      }

      rc = _extractDiskInfo( buf, builder ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to extract disk info:%d", rc ) ;
         stringstream ss ;
         ss << "failed to read msg from buf:"
            << buf ;
         detail = BSON( SPT_ERR << ss.str() ) ;
         goto error ;
      }
      
      rval.setStringVal( "", builder.obj().toString( FALSE, TRUE ).c_str() ) ;
   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _sptUsrSystem::_extractDiskInfo( const CHAR *buf,
                                          bson::BSONObjBuilder &builder )
   {
      INT32 rc = SDB_OK ;
      BSONArrayBuilder arrBuilder ;
      string fileSystem ;
      string used ;
      string available ;
      string mount ;
      vector<string> splited ;
      boost::algorithm::split( splited, buf, boost::is_any_of("\n") ) ;
      for ( vector<string>::iterator itr = splited.begin();
            itr != splited.end();
            itr++ )
      {
         if ( itr->empty() )
         {
            continue ;
         }

         vector<string> columns ;
         boost::algorithm::split( columns, *itr, boost::is_any_of("\t ") ) ;
         
         for ( vector<string>::iterator itr2 = columns.begin();
               itr2 != columns.end();
               /// do not ++      
               )
         {
            if ( itr2->empty() )
            {
               itr2 = columns.erase( itr2 ) ;
            }
            else
            {
               ++itr2 ;
            }
         }

         if ( 6 == columns.size() )
         {
            fileSystem = columns.at( 0 ) ;
            used = columns.at( 2 ) ;
            available = columns.at( 3 ) ;
            mount = columns.at( 5 ) ;
         }
         else if ( 1 == columns.size() )
         {
            fileSystem = columns.at( 0 ) ;
         }
         else if ( 5 == columns.size() )
         {
            used = columns.at( 1 ) ;
            available = columns.at( 2 ) ;
            mount = columns.at( 4 ) ;
         }
         else
         {
            rc = SDB_SYS ;
            goto error ;
         }

         if ( !mount.empty() )
         {
            if ( 0 != ossStrncmp( "/dev/shm", mount.c_str(), 8 ) )
            {
               SINT64 total = 0 ;
               SINT64 usedNumber = 0 ;
               SINT64 avaNumber = 0 ;
               BSONObjBuilder lineBuilder ;
               try
               {
                  usedNumber = boost::lexical_cast<SINT64>( used ) ;
                  avaNumber = boost::lexical_cast<SINT64>( available ) ;
                  total = usedNumber + avaNumber ;
                  lineBuilder.append( SPT_USR_SYSTEM_FILESYSTEM,
                                      fileSystem.c_str() ) ;
                  lineBuilder.appendNumber( SPT_USR_SYSTEM_SIZE, total ) ;
                  lineBuilder.appendNumber( SPT_USR_SYSTEM_USED, usedNumber ) ;
                  lineBuilder.append( SPT_USR_SYSTEM_UNIT, "M" ) ;
                  lineBuilder.append( SPT_USR_SYSTEM_MOUNT, mount ) ;
                  lineBuilder.appendBool( SPT_USR_SYSTEM_ISLOCAL,
                                          string::npos != fileSystem.find( "/dev/sd", 0, 7 )) ;
                  arrBuilder << lineBuilder.obj() ;
               }
               catch ( std::exception &e )
               {
                  rc = SDB_SYS ;
                  goto error ;
               }
            }

            used.clear();
            available.clear() ;
            mount.clear() ;
            fileSystem.clear() ;

         }
      }

      builder.append( SPT_USR_SYSTEM_DISKS, arrBuilder.arr() ) ;
   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _sptUsrSystem::getNetcardInfo( const _sptArguments &arg,
                                        _sptReturnVal &rval,
                                        bson::BSONObj &detail )
   {
      INT32 rc = SDB_OK ;
      BSONObjBuilder builder ;
      if ( 0 < arg.argc() )
      {
         rc = SDB_INVALIDARG ;
         goto error ;
      }

      rc = _extractNetcards( builder ) ;
      if ( SDB_OK != rc )
      {
         stringstream ss ;
         ss << "failed to get netcard info:" << rc ;
         detail = BSON( SPT_ERR << ss.str() ) ;
         goto error ; 
      }

      rval.setStringVal( "", builder.obj().toString( FALSE, TRUE ).c_str() ) ;
   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _sptUsrSystem::_extractNetcards( bson::BSONObjBuilder &builder )
   {
      INT32 rc = SDB_OK ;
#if defined (_WINDOWS)
      rc = SDB_SYS ;
      goto error ;
#elif defined (_LINUX)
      BSONArrayBuilder arrBuilder ;
      BYTE buf[512] = { 0 } ;
      struct ifconf ifc ;
      ifc.ifc_len = 512 ;
      ifc.ifc_buf = ( CHAR * )buf;
      struct ifreq *ifreq = NULL ;
      INT32 sock ;

      if ( (sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0 )
      {
         PD_LOG( PDERROR, "failed to init socket" ) ;
         rc = SDB_SYS ;
         goto error ;
      }

      if ( SDB_OK != ioctl( sock, SIOCGIFCONF, &ifc ) )
      {
         rc = SDB_SYS ;
         PD_LOG( PDERROR, "failed to call ioctl" ) ;
         goto error ;
      }

      ifreq = ( struct ifreq * )buf ;
      for ( INT32 i = ifc.ifc_len / sizeof(struct ifreq);
            i > 0;
            --i )
      {
         arrBuilder << BSON( SPT_USR_SYSTEM_NAME << ifreq->ifr_name
                             << SPT_USR_SYSTEM_IP <<
                             inet_ntoa(((struct sockaddr_in*)&
                                         (ifreq->ifr_addr))->sin_addr) ) ;
         ++ifreq ;
      }

      builder.append( SPT_USR_SYSTEM_NETCARDS, arrBuilder.arr() ) ;
#endif
   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _sptUsrSystem::getIpTablesInfo( const _sptArguments &arg,
                                         _sptReturnVal &rval,
                                         bson::BSONObj &detail )
   {
      INT32 rc = SDB_OK ;
      if ( 0 < arg.argc() )
      {
         rc = SDB_INVALIDARG ;
         goto error ;
      }

      {
      BSONObj info = BSON( "FireWall" << "unknown" ) ;
      rval.setStringVal( "", info.toString().c_str() ) ;
      }
   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _sptUsrSystem::getHostName( const _sptArguments &arg,
                                _sptReturnVal &rval,
                                bson::BSONObj &detail )
   {
      INT32 rc = SDB_OK ;
      if ( 0 < arg.argc() )
      {
         rc = SDB_INVALIDARG ;
         goto error ;
      }

      {
      UINT32 exitCode = 0 ;
      _sptCmdRunner runner ;
      const SINT64 bufLen = 1024 ;
      SINT64 read = 0 ;
      CHAR buf[bufLen + 1] = { 0 } ;
#if defined (_LINUX)
      rc = runner.exec( "hostname", exitCode ) ;
#elif defined (_WINDOWS)
      rc = SDB_SYS ;
#endif
      if ( SDB_OK != rc || SDB_OK != exitCode )
      {
         PD_LOG( PDERROR, "failed to exec cmd, rc:%d, exit:%d",
                 rc, exitCode ) ;
         if ( SDB_OK == rc )
         {
            rc = SDB_SYS ;
         }
         stringstream ss ;
         ss << "failed to exec cmd \"hostname\",rc:"
            << rc
            << ",exit:"
            << exitCode ;
         detail = BSON( SPT_ERR << ss.str() ) ;
         goto error ;
      }

      rc = runner.read( buf, bufLen, read ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to read msg from cmd runner:%d", rc ) ;
         stringstream ss ;
         ss << "failed to read msg from cmd \"hostname\", rc:"
            << rc ;
         detail = BSON( SPT_ERR << ss.str() ) ;
         goto error ;
      }

      if ( '\0' != buf[0] )
      {
         /// erase the last '\n'
         buf[ossStrlen( buf ) - 1] = '\0' ;
      }
      rval.setStringVal( "", buf ) ;
      }
   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _sptUsrSystem::help( const _sptArguments & arg,
                              _sptReturnVal & rval,
                              BSONObj & detail )
   {
      stringstream ss ;
      ss << "System functions:" << endl
         << " System.ping( hostname )" << endl
         << " System.type()" << endl
         << " System.getReleaseInfo()" << endl
         << " System.getHostsMap()" << endl
         << " System.getCpuInfo()" << endl
         << " System.getMemInfo()" << endl
         << " System.getDiskInfo()" << endl
         << " System.getNetcardInfo()" << endl
         << " System.getIpTablesInfo()" << endl
         << " System.getHostName()" << endl ;
      rval.setStringVal( "", ss.str().c_str() ) ;
      return SDB_OK ;
   }

}

