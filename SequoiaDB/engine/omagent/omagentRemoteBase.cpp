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

   Source File Name = omagentRemoteBase.cpp

   Dependencies: N/A


   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          08/03/2016  WJM  Initial Draft

   Last Changed =

*******************************************************************************/

#include "omagentRemoteBase.hpp"
#include "pmdOptions.h"
#include "msgDef.h"
#include "pmd.hpp"
#include "ossSocket.hpp"
#include "ossIO.hpp"
#include "oss.h"
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#if defined (_LINUX)
#include <net/if.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#else
#include <iphlpapi.h>
#pragma comment( lib, "IPHLPAPI.lib" )
#endif

using namespace bson ;

namespace engine
{
   /*
      _remoteExec implement
   */

   _remoteExec::_remoteExec()
   {
   }

   _remoteExec::~_remoteExec()
   {
   }

   INT32 _remoteExec::init( const CHAR * pInfomation )
   {
      INT32 rc = SDB_OK ;

      try
      {
         BSONObj Obj( pInfomation ) ;

         _optionObj = Obj.getObjectField( "$optionObj" ) ;
         _matchObj  = Obj.getObjectField( "$matchObj" ) ;
         _valueObj  = Obj.getObjectField( "$valueObj" ) ;
      }
      catch( std::exception &e )
      {
         PD_LOG( PDERROR, "Occur exception: %s", e.what() ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   /*
      _remoteHost implement
   */

   _remoteHost::_remoteHost()
   {
   }

   _remoteHost::~_remoteHost()
   {
   }

   INT32 _remoteHost::_parseHostsFile( VEC_HOST_ITEM &vecItems, string &err )
   {
      INT32 rc = SDB_OK ;
      OSSFILE file ;
      stringstream ss ;
      BOOLEAN isOpen = FALSE ;
      INT64 fileSize = 0 ;
      CHAR *pBuff = NULL ;
      INT64 hasRead = 0 ;

      rc = ossGetFileSizeByName( HOSTS_FILE, &fileSize ) ;
      if ( rc )
      {
         ss << "get file[" << HOSTS_FILE << "] size failed: " << rc ;
         goto error ;
      }
      pBuff = ( CHAR* )SDB_OSS_MALLOC( fileSize + 1 ) ;
      if ( !pBuff )
      {
         ss << "alloc memory[" << fileSize << "] failed" ;
         rc = SDB_OOM ;
         goto error ;
      }
      ossMemset( pBuff, 0, fileSize + 1 ) ;

      rc = ossOpen( HOSTS_FILE, OSS_READONLY|OSS_SHAREREAD, 0,
                    file ) ;
      if ( rc )
      {
         ss << "open file[" << HOSTS_FILE << "] failed: " << rc ;
         goto error ;
      }
      isOpen = TRUE ;

      // read file
      rc = ossReadN( &file, fileSize, pBuff, hasRead ) ;
      if ( rc )
      {
         ss << "read file[" << HOSTS_FILE << "] failed: " << rc ;
         goto error ;
      }
      ossClose( file ) ;
      isOpen = FALSE ;

      rc = _extractHosts( pBuff, vecItems ) ;
      if ( rc )
      {
         ss << "extract hosts failed: " << rc ;
         goto error ;
      }

      // remove last empty
      if ( vecItems.size() > 0 )
      {
         VEC_HOST_ITEM::iterator itr = vecItems.end() - 1 ;
         usrSystemHostItem &info = *itr ;
         if ( info.toString().empty() )
         {
            vecItems.erase( itr ) ;
         }
      }

   done:
      if ( isOpen )
      {
         ossClose( file ) ;
      }
      if ( pBuff )
      {
         SDB_OSS_FREE( pBuff ) ;
      }
      return rc ;
   error:
      err = ss.str() ;
      goto done ;
   }

   INT32 _remoteHost::_writeHostsFile( VEC_HOST_ITEM & vecItems,
                                         string & err )
   {
      INT32 rc = SDB_OK ;
      std::string tmpFile = HOSTS_FILE ;
      tmpFile += ".tmp" ;
      OSSFILE file ;
      BOOLEAN isOpen = FALSE ;
      BOOLEAN isBak = FALSE ;
      stringstream ss ;

      if ( SDB_OK == ossAccess( tmpFile.c_str() ) )
      {
         ossDelete( tmpFile.c_str() ) ;
      }

      // 1. first back up the file
      if ( SDB_OK == ossAccess( HOSTS_FILE ) )
      {
         if ( SDB_OK == ossRenamePath( HOSTS_FILE, tmpFile.c_str() ) )
         {
            isBak = TRUE ;
         }
      }

      // 2. Create the file
      rc = ossOpen ( HOSTS_FILE, OSS_READWRITE|OSS_SHAREWRITE|OSS_REPLACE,
                     OSS_RU|OSS_WU|OSS_RG|OSS_RO, file ) ;
      if ( rc )
      {
         ss << "open file[" <<  HOSTS_FILE << "] failed: " << rc ;
         goto error ;
      }
      isOpen = TRUE ;

      // 3. write data
      {
         VEC_HOST_ITEM::iterator it = vecItems.begin() ;
         UINT32 count = 0 ;
         while ( it != vecItems.end() )
         {
            ++count ;
            usrSystemHostItem &item = *it ;
            ++it ;
            string text = item.toString() ;
            if ( !text.empty() || count < vecItems.size() )
            {
               text += OSS_NEWLINE ;
            }
            rc = ossWriteN( &file, text.c_str(), text.length() ) ;
            if ( rc )
            {
               ss << "write context[" << text << "] to file[" << HOSTS_FILE
                  << "] failed: " << rc ;
               goto error ;
            }
         }
      }

      // 4. remove tmp
      if ( SDB_OK == ossAccess( tmpFile.c_str() ) )
      {
         ossDelete( tmpFile.c_str() ) ;
      }

   done:
      if ( isOpen )
      {
         ossClose( file ) ;
      }
      return rc ;
   error:
      if ( isBak )
      {
         if ( isOpen )
         {
            ossClose( file ) ;
            isOpen = FALSE ;
            ossDelete( HOSTS_FILE ) ;
         }
         ossRenamePath( tmpFile.c_str(), HOSTS_FILE ) ;
      }
      err = ss.str() ;
      goto done ;
   }

   INT32 _remoteHost::_extractHosts( const CHAR *buf,
                                       VEC_HOST_ITEM &vecItems )
   {
      INT32 rc = SDB_OK ;
      vector<string> splited ;

      try
      {
         boost::algorithm::split( splited, buf, boost::is_any_of("\r\n") ) ;
      }
      catch( std::exception &e )
      {
         rc = SDB_SYS ;
         PD_LOG( PDERROR, "Failed to split result, rc: %d", rc ) ;
         goto error ;
      }
      if ( splited.empty() )
      {
         goto done ;
      }

      for ( vector<string>::iterator itr = splited.begin() ;
            itr != splited.end() ;
            itr++ )
      {
         usrSystemHostItem item ;
         vector<string> columns ;
         if ( itr->empty() )
         {
            vecItems.push_back( item ) ;
            continue ;
         }

         try
         {
            boost::algorithm::trim( *itr ) ;
            boost::algorithm::split( columns, *itr, boost::is_any_of("\t ") ) ;
         }
         catch( std::exception &e )
         {
            rc = SDB_SYS ;
            PD_LOG( PDERROR, "Failed to split result, rc: %d", rc ) ;
            goto error ;
         }
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
            item._ip = *itr ;
            vecItems.push_back( item ) ;
            continue ;
         }

         if ( !isValidIPV4( columns.at( 0 ).c_str() ) )
         {
            item._ip = *itr ;
            vecItems.push_back( item ) ;
            continue ;
         }

         item._ip = columns[ 0 ] ;
         if ( columns.size() == 3 )
         {
            item._com = columns[ 1 ] ;
            item._host = columns[ 2 ] ;
         }
         else
         {
            item._host = columns[ 1 ] ;
         }
         item._lineType = LINE_HOST ;
         vecItems.push_back( item ) ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   void _remoteHost::_buildHostsResult( VEC_HOST_ITEM & vecItems,
                                          BSONObjBuilder &builder )
   {
      BSONArrayBuilder arrBuilder ;
      VEC_HOST_ITEM::iterator it = vecItems.begin() ;
      while ( it != vecItems.end() )
      {
         usrSystemHostItem &item = *it ;
         ++it ;

         if ( LINE_HOST != item._lineType )
         {
            continue ;
         }
         arrBuilder << BSON( CMD_USR_SYSTEM_IP << item._ip <<
                             CMD_USR_SYSTEM_HOSTNAME << item._host ) ;
      }
      builder.append( CMD_USR_SYSTEM_HOSTS, arrBuilder.arr() ) ;
   }

}
