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

   Source File Name = utilParam.cpp

   Descriptive Name =

   When/how to use: str util

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          26/04/2014  XJH Initial Draft

   Last Changed =

******************************************************************************/


#include "utilParam.hpp"
#include "ossIO.hpp"
#include "ossProc.hpp"
#include "ossUtil.hpp"
#include "utilStr.hpp"
#include "pmdDef.hpp"
#include "oss.h"
#include "pmdOptions.hpp"
#include <iostream>

#include <boost/algorithm/string.hpp>

namespace engine
{

#if defined( _LINUX )
   #define SDB_INSTALL_FILE_NAME             "/etc/default/sequoiadb"
#else
   #define SDB_INSTALL_FILE_NAME             "C:\\default\\sequoiadb"
#endif // _LINUX

   INT32 utilReadConfigureFile( const CHAR *file,
                                po::options_description &desc,
                                po::variables_map &vm )
   {
      INT32 rc = SDB_OK;

      try
      {
         po::store( po::parse_config_file<char> ( file, desc, TRUE ), vm ) ;
         po::notify ( vm ) ;
      }
      catch( po::reading_file )
      {
         std::cerr << "Failed to open config file: "
                   <<( std::string ) file << std::endl ;
         rc = SDB_IO ;
         goto error ;
      }
      catch ( po::unknown_option &e )
      {
         std::cerr << "Unknown config element: "
                   << e.get_option_name () << std::endl ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }
      catch ( po::invalid_option_value &e )
      {
         std::cerr << ( std::string ) "Invalid config element: "
                   << e.get_option_name () << std::endl ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }
      catch( po::error &e )
      {
         std::cerr << e.what () << std::endl ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }

   done:
      return rc;
   error:
      goto done;
   }

   INT32 utilReadCommandLine( INT32 argc, CHAR **argv,
                              po::options_description &desc,
                              po::variables_map &vm,
                              BOOLEAN allowUnreg )
   {
      INT32 rc = SDB_OK;

      try
      {
         if ( allowUnreg )
         {
            po::store ( po::command_line_parser( argc, argv).options(
                        desc ).allow_unregistered().run(), vm ) ;
         }
         else
         {
            po::store( po::parse_command_line( argc, argv, desc ), vm ) ;
         }
         po::notify ( vm ) ;
      }
      catch ( po::unknown_option &e )
      {
         std::cerr <<  "Unknown argument: "
                   << e.get_option_name () << std::endl ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }
      catch ( po::invalid_option_value &e )
      {
         std::cerr << "Invalid argument: "
                   << e.get_option_name () << std::endl ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }
      catch( po::error &e )
      {
         std::cerr << e.what () << std::endl ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 utilWriteConfigFile( const CHAR * pFile, const CHAR * pData,
                              BOOLEAN createOnly )
   {
      INT32 rc = SDB_OK ;
      std::string tmpFile = pFile ;
      tmpFile += ".tmp" ;
      OSSFILE file ;
      BOOLEAN isOpen = FALSE ;
      BOOLEAN isBak = FALSE ;

      if ( SDB_OK == ossAccess( tmpFile.c_str() ) )
      {
         ossDelete( tmpFile.c_str() ) ;
      }

      // 1. first back up the file
      if ( SDB_OK == ossAccess( pFile ) )
      {
         if ( createOnly )
         {
            rc = SDB_FE ;
            goto error ;
         }
         if ( SDB_OK == ossRenamePath( pFile, tmpFile.c_str() ) )
         {
            isBak = TRUE ;
         }
      }

      // 2. Create the file
      rc = ossOpen ( pFile, OSS_READWRITE|OSS_SHAREWRITE|OSS_REPLACE,
                     OSS_RWXU, file ) ;
      if ( rc )
      {
         goto error ;
      }
      isOpen = TRUE ;

      // 3. write data
      {
         SINT64 written = 0 ;
         SINT64 len = ossStrlen( pData ) ;
         while ( 0 < len )
         {
            SINT64 tmpWritten = 0 ;
            rc = ossWrite( &file, pData + written , len, &tmpWritten ) ;
            if ( rc && SDB_INTERRUPT != rc )
            {
               PD_LOG( PDERROR, "Failed to write file[%s]:%d", pFile, rc ) ;
               goto error ;
            }
            written += tmpWritten ;
            len -= tmpWritten ;
            rc = SDB_OK ;
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
            ossDelete( pFile ) ;
            ossRenamePath( tmpFile.c_str(), pFile ) ;
         }
      }
      goto done ;
   }

   INT32 utilGetServiceByConfigPath( const string & confPath,
                                     string & svcname )
   {
      INT32 rc = SDB_OK ;
      po::options_description desc ;
      po::variables_map vm ;
      desc.add_options()
         ( PMD_OPTION_SVCNAME, po::value<string>(), "" ) ;
      CHAR conf[OSS_MAX_PATHSIZE + 1] = { 0 } ;
      svcname = boost::lexical_cast<string>(OSS_DFT_SVCPORT) ;

      rc = utilBuildFullPath ( confPath.c_str(), PMD_DFT_CONF,
                               OSS_MAX_PATHSIZE, conf ) ;
      if ( rc )
      {
         std::cerr << "Failed to build full path, rc: " << rc << std::endl ;
         goto error ;
      }

      rc = utilReadConfigureFile( conf, desc, vm ) ;
      if ( SDB_IO == rc )
      {
         // if the file does not exist, let's just continue so that sequoiadb
         // is able to create the file
         rc = SDB_OK ;
         goto done ;
      }
      if ( rc )
      {
         goto error ;
      }

      if ( vm.count ( PMD_OPTION_SVCNAME ) )
      {
         svcname = vm [ PMD_OPTION_SVCNAME ].as<string>() ;
      }

   done :
      return rc ;
   error :
      goto done ;
   }

   INT32 utilGetInstallInfo( utilInstallInfo & info )
   {
      INT32 rc = SDB_OK ;
      po::options_description desc ;
      po::variables_map vm ;

      PMD_ADD_PARAM_OPTIONS_BEGIN( desc )
         ( SDB_INSTALL_RUN_FILED, po::value<string>(), "after to run cmd" ) \
         ( SDB_INSTALL_USER_FIELD, po::value<string>(), "user" ) \
         ( SDB_INSTALL_PATH_FIELD, po::value<string>(), "install path" )
      PMD_ADD_PARAM_OPTIONS_END

      rc = utilReadConfigureFile( SDB_INSTALL_FILE_NAME, desc, vm ) ;
      if ( rc )
      {
         PD_LOG( PDERROR, "Failed to read install info from file, rc: %d",
                 rc ) ;
         goto error ;
      }

      if ( vm.count( SDB_INSTALL_RUN_FILED ) )
      {
         info._run = vm[ SDB_INSTALL_RUN_FILED ].as<string>() ;
      }
      if ( vm.count( SDB_INSTALL_USER_FIELD ) )
      {
         info._user = vm[ SDB_INSTALL_USER_FIELD ].as<string>() ;
      }
      if ( vm.count( SDB_INSTALL_PATH_FIELD ) )
      {
         info._path = vm[ SDB_INSTALL_PATH_FIELD ].as<string>() ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 utilCheckAndChangeUserInfo( const CHAR * curFileName )
   {
      INT32 rc = SDB_OK ;
      utilInstallInfo info ;
      OSSUID fileUID = OSS_INVALID_UID ;
      OSSGID fileGID = OSS_INVALID_GID ;
      OSSUID curUID  = OSS_INVALID_UID ;
      OSSGID curGID  = OSS_INVALID_GID ;

      // first compare file:cur uid/gid
      ossGetFileUserInfo( curFileName, fileUID, fileGID ) ;
      curUID = ossGetCurrentProcessUID() ;
      curGID = ossGetCurrentProcessGID() ;

      if ( OSS_INVALID_UID == fileUID || 0 == fileUID ||
           OSS_INVALID_GID == fileGID || 0 == fileGID )
      {
         // get install user info
         rc = utilGetInstallInfo( info ) ;
         if ( rc )
         {
            // no install info, not change
            rc = SDB_OK ;
            goto done ;
         }
         // get install user uid and gid
         rc = ossGetUserInfo( info._user.c_str(), fileUID, fileGID ) ;
         if ( rc )
         {
            // no install user, not change
            rc = SDB_OK ;
            goto done ;
         }
      }
      else
      {
         CHAR usrName[ OSS_MAX_PATHSIZE + 1 ] = { 0 } ;
         ossGetUserInfo( fileUID, usrName, OSS_MAX_PATHSIZE ) ;
         info._user = usrName ;
      }

      if ( curGID != fileGID )
      {
         rc = ossSetCurrentProcessGID( fileGID ) ;
         if ( rc )
         {
            goto error ;
         }
      }
      if ( curUID != fileUID )
      {
         rc = ossSetCurrentProcessUID( fileUID ) ;
         if ( rc )
         {
            goto error ;
         }
      }

   done:
      return rc ;
   error:
      std::cout << "Please run it by user: " << info._user << std::endl ;
      goto done ;
   }

}


