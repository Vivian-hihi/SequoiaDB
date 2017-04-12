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

   Source File Name = coordCommon.cpp

   Descriptive Name =

   When/how to use:

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          04/05/2017  XJH Initial Draft

   Last Changed =

*******************************************************************************/

#include "coordCommon.hpp"
#include "msgCatalogDef.h"

using namespace bson ;

namespace engine
{

   INT32 coordInitCataPtrFromObj( const BSONObj &obj,
                                  CoordCataInfoPtr & cataPtr)
   {
      INT32 rc = SDB_OK ;
      CoordCataInfo *pCataInfoTmp = NULL ;

      try
      {
         BSONElement eName, eVersion ;
         // Check types of name and version elements
         eName = obj.getField( CAT_CATALOGNAME_NAME ) ;
         if ( String != eName.type() )
         {
            PD_LOG( PDERROR, "Failed to get field[%s] from obj[%s]",
                    CAT_CATALOGNAME_NAME, obj.toString().c_str() ) ;
            rc = SDB_INVALIDARG ;
            goto error ;
         }

         eVersion = obj.getField( CAT_CATALOGVERSION_NAME ) ;
         if ( !eVersion.isNumber() )
         {
            PD_LOG( PDERROR, "Failed to get field[%s] from obj[%s]",
                    CAT_CATALOGVERSION_NAME, obj.toString().c_str() ) ;
            rc = SDB_INVALIDARG ;
            goto error ;
         }

         pCataInfoTmp = SDB_OSS_NEW CoordCataInfo( eVersion.number(),
                                                   eName.valuestr() ) ;
         if ( !pCataInfoTmp )
         {
            PD_LOG( PDERROR, "Allocate memory for catalog info failed" ) ;
            rc = SDB_OOM ;
            goto error ;
         }

         cataPtr = CoordCataInfoPtr( pCataInfoTmp ) ;
         rc = cataPtr->fromBSONObj( obj ) ;
         if ( rc )
         {
            PD_LOG( PDERROR, "Failed to init catalog info from obj[%s], "
                    "rc: %d", obj.toString().c_str(), rc ) ;
            goto error ;
         }
      }
      catch ( std::exception &e )
      {
         rc = SDB_INVALIDARG ;
         PD_LOG ( PDERROR, "Occur exception when parse catalog info "
                  "object: %s", e.what() ) ;
         goto error ;
      }

   done :
      return rc ;
   error :
      goto done ;
   }

   BOOLEAN coordIsCataAddrSame( const CoordVecNodeInfo &left,
                                const CoordVecNodeInfo &right )
   {
      if ( left.size() != right.size() )
      {
         return FALSE ;
      }
      for ( UINT32 i = 0 ; i < left.size() ; ++i )
      {
         const clsNodeItem &leftItem = left[i] ;
         const clsNodeItem &rightItem = right[i] ;

         if ( 0 != ossStrcmp( leftItem._host, rightItem._host ) ||
              0 != ossStrcmp( leftItem._service[MSG_ROUTE_CAT_SERVICE].c_str(),
                              rightItem._service[MSG_ROUTE_CAT_SERVICE].c_str() ) )
         {
            return FALSE ;
         }
      }
      return TRUE ;
   }

   BOOLEAN coordCataCheckFlag( INT32 flag )
   {
      return ( SDB_CLS_COORD_NODE_CAT_VER_OLD == flag ||
               SDB_CLS_NO_CATALOG_INFO == flag ||
               SDB_CLS_GRP_NOT_EXIST == flag ||
               SDB_CLS_NODE_NOT_EXIST == flag ||
               SDB_CAT_NO_MATCH_CATALOG == flag ) ;
   }

}

