/*******************************************************************************


   Copyright (C) 2011-2017 SequoiaDB Ltd.

   This program is free software: you can redistribute it and/or modify
   it under the term of the GNU Affero General Public License, version 3,
   as published by the Free Software Foundation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warrenty of
   MARCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU Affero General Public License for more details.

   You should have received a copy of the GNU Affero General Public License
   along with this program. If not, see <http://www.gnu.org/license/>.

   Source File Name = utilESCltMgr.cpp

   Descriptive Name = Elasticsearch client manager.

   When/how to use: this program may be used on binary and text-formatted
   versions of data management component. This file contains structure for
   DMS storage unit and its methods.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          14/04/2017  YSD Initial Draft

   Last Changed =

*******************************************************************************/
#include "core.hpp"
#include "pd.hpp"
#include "utilESClt.hpp"
#include "utilESCltMgr.hpp"

namespace engine
{
   _utilESCltMgr::_utilESCltMgr()
   {
   }

   _utilESCltMgr::~_utilESCltMgr()
   {
      // Clean all the clients.
      _utilList<utilESClt *>::iterator itr = _seCltList.begin() ;
      while ( itr != _seCltList.end() )
      {
         _seCltList.erase( itr ) ;
      }
   }

   INT32 _utilESCltMgr::init( const std::string &url )
   {
      INT32 rc = SDB_OK ;
      utilESClt *seClt = NULL ;

      _url = url ;

      // Initial one connection with search engine, and push it into the list.
      rc = getSeClt( &seClt ) ;
      if ( rc )
      {
         PD_LOG( PDERROR, "Failed to init the first client with search engine, "
                 "rc: %d", rc ) ;
         goto error ;
      }

      _seCltList.push_back( seClt ) ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _utilESCltMgr::getSeClt( utilESClt **seClt )
   {
      INT32 rc = SDB_OK ;
      utilESClt* client = NULL ;

      SDB_ASSERT( seClt, "Parameter should not be null" ) ;

      // If there are idle client, get it. Otherwise, create a new one.
      if ( _seCltList.size() > 0 )
      {
         client = _seCltList.back() ;
         _seCltList.pop_back() ;
      }
      else
      {
         client = SDB_OSS_NEW _utilESClt() ;
         if ( !client )
         {
            rc = SDB_OOM ;
            PD_LOG( PDERROR, "Failed to allocate memory for search engine client, "
                    "rc: %d", rc ) ;
            goto error ;
         }
         rc = client->init( _url ) ;
         if ( rc )
         {
            PD_LOG( PDERROR, "Failed to init search engine client, rc: %d",
                    rc ) ;
            goto error ;
         }
      }

      *seClt = client ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _utilESCltMgr::releaseClt( utilESClt **seClt )
   {
      INT32 rc = SDB_OK ;

      SDB_ASSERT( seClt, "Client pointer should not be null" ) ;
      if ( !seClt || !(*seClt) )
      {
         rc = SDB_INVALIDARG ;
         PD_LOG( PDERROR, "Null search engine client object to release, rc: %d",
                 rc ) ;
         goto error ;
      }



   done:
      return rc ;
   error:
      goto done ;
   }
}

