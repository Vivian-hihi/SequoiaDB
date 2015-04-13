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

   Source File Name = omagentNodeMgr.cpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          08/04/2015  LZ Initial Draft

   Last Changed =

*******************************************************************************/
#include "omagentNodePathGuard.hpp"
#include "pd.hpp"
#include "ossUtil.hpp"
#include "ossPath.hpp"
#include "ossIO.hpp"

namespace engine {

   #define W_OK 2

   _omaNodePathGuard::_omaNodePathGuard( const CHAR *nodeName )
   {
      ossMemset( _nodeName, 0, OSS_MAX_SERVICENAME + 1 ) ;
      ossMemcpy( _nodeName, nodeName, ossStrlen(nodeName) ) ;
   }

   _omaNodePathGuard::~_omaNodePathGuard()
   {
   }

   void _omaNodePathGuard::addToPath( const CHAR *path )
   {
      SDB_ASSERT( NULL != path, "path cannot be NULL" ) ;

      std::string addPath = path ;
      _nodePaths.push_back( addPath ) ;
   }

   bool _omaNodePathGuard::checkFolderPath( const CHAR *nodeName, const CHAR *path )
   {
      INT32 rc = SDB_OK ;
      if ( 0 != ossStrncmp( nodeName, _nodeName, OSS_MAX_SERVICENAME ) )
      {
         if ( _contains( path ) || _existedFiles( path ) )
         {
            rc = SDB_FE ;
         }
      }

      return SDB_OK == rc ;
   }

   bool _omaNodePathGuard::_contains( const CHAR *path )
   {
      INT32 rc = SDB_OK ;
      std::vector<string>::const_iterator cit = _nodePaths.begin() ;
      for ( ; _nodePaths.end() != cit; ++cit)
      {
         if ( 0 == ossStrncmp( (*cit).c_str(), path,
                               ossStrlen( path ) )
              || 0 == ossStrncmp( (*cit).c_str(), path,
                               ossStrlen( (*cit).c_str() ) ) )
         {
            // path should not be created
            rc = SDB_FE ;
         }
      }

      return SDB_OK != rc ;
   }

   bool _omaNodePathGuard::_existedFiles( const CHAR* path )
   {
      SDB_ASSERT( NULL != path, "path cannot be NULL" ) ;

      INT32 rc = SDB_OK ;

      rc = ossAccess(path, W_OK );
      if ( SDB_OK == rc )
      {
         std::map<std::string, std::string> subFiles ;
         ossEnumFiles( path, subFiles, NULL, 5 ) ;
         if ( !subFiles.empty() )
         {
            PD_LOG( PDERROR, "The path: %s is existed and not empty ",
                    path ) ;
            rc = SDB_FE ;
            goto error ;
         }
      }

   done:
      return ( SDB_OK == rc ) ;
   error:
      goto done;
   }
}
