/*******************************************************************************

   Copyright (C) 2011-2018 SequoiaDB Ltd.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU Affero General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Affero General Public License for more details.

   You should have received a copy of the GNU Affero General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.

   Source File Name = coordAutoIncItem.cpp

   Descriptive Name =

   When/how to use:

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          27/08/2018  LSQ Initial Draft

   Last Changed =

*******************************************************************************/

#include "coordAutoIncItem.hpp"
#include "pd.hpp"
#include "ossTypes.h"
#include "ossMem.hpp"
#include <map>

namespace engine

{
   /*
      implement coordAutoIncItem
   */
   _coordAutoIncItem::_coordAutoIncItem()
   {
      _fieldName = NULL ;
      _sequenceName = NULL ;
      _generatedType = AUTOINC_GEN_DEFAULT ;
      _pSubFieldMap = NULL ;
   }

   _coordAutoIncItem::~_coordAutoIncItem()
   {
      SDB_OSS_FREE( _fieldName ) ;
      SDB_OSS_FREE( _sequenceName ) ;
      if ( _pSubFieldMap )
      {
         AUTOINC_ITEM_MAP_IT it ;
         for ( it = _pSubFieldMap->begin() ; it != _pSubFieldMap->end() ; ++it )
         {
            SAFE_OSS_DELETE( it->second ) ;
         }
         delete _pSubFieldMap ;
         _pSubFieldMap = NULL ;
      }
   }

   INT32 _coordAutoIncItem::init( const CHAR *fieldName,
                                  const CHAR *sequenceName,
                                  const AUTOINC_GEN_TYPE generatedType )
   {
      /*
         If fieldName is simple, result will be like
         {
            field: "a",
            sequence: "SYS_xxx_a_SEQ",
            generated: "always",
            subFields: <unused>
         }.
         If fieldName is nested, like "a.b", result will be like
         {
            field: "a",
            sequence: <unused>,
            generated: <unused>,
            subFields: [
               {
                  field: "b",
                  sequence: "SYS_xxx_a.b_SEQ",
                  generated: "always",
                  subFields: <unused>
               }
            ]
         }
      */

      INT32             rc = SDB_OK ;
      const CHAR*       pos = NULL ;
      const CHAR*       subField = NULL ;
      INT32             strLength = 0 ;

      _coordAutoIncItem *pSubItem = NULL ;

      if ( NULL == ossStrchr( fieldName, '.' ) )
      {
         strLength = ossStrlen( fieldName ) + 1 ;
         _fieldName = (CHAR*)SDB_OSS_MALLOC( strLength ) ;
         PD_CHECK( _fieldName, SDB_OOM, error, PDERROR,
                   "Failed to malloc for field name, rc: %d", rc ) ;
         ossStrncpy( _fieldName, fieldName, strLength ) ;

         strLength = ossStrlen( sequenceName ) + 1 ;
         _sequenceName = (CHAR*)SDB_OSS_MALLOC( strLength ) ;
         PD_CHECK( _sequenceName, SDB_OOM, error, PDERROR,
                   "Failed to malloc for sequence name, rc: %d", rc ) ;
         ossStrncpy( _sequenceName, sequenceName, strLength ) ;

         _generatedType = generatedType ;
      }
      else
      {
         // extract main field
         pos = ossStrchr( fieldName, '.' ) ;
         strLength = (pos - fieldName) + 1 ;
         _fieldName = (CHAR *)SDB_OSS_MALLOC( strLength ) ;
         PD_CHECK( _fieldName, SDB_OOM, error, PDERROR,
                   "Failed to malloc for field name, rc: %d", rc ) ;
         ossStrncpy( _fieldName, fieldName, strLength ) ;
         _fieldName[ strLength - 1 ] = 0 ;

         // create subItem ;
         subField = pos + 1 ;
         pSubItem = SDB_OSS_NEW _coordAutoIncItem() ;
         PD_CHECK( pSubItem, SDB_OOM, error, PDERROR,
                   "Failed to malloc for sub autoIncrement item, rc: %d", rc ) ;
         rc = pSubItem->init( subField, sequenceName, generatedType ) ;
         PD_RC_CHECK( rc, PDERROR, "Failed to init autoIncrement sub item, rc: %d", rc ) ;

         _pSubFieldMap = new(std::nothrow) AUTOINC_ITEM_MAP ;
         PD_CHECK( _pSubFieldMap, SDB_OOM, error, PDERROR,
                   "Failed to new sub field map, rc: %d", rc ) ;
         _pSubFieldMap->insert( AUTOINC_ITEM_MAP_VAL( pSubItem->fieldName(), pSubItem ) ) ;
         pSubItem = NULL ;
      }

   done:
      return rc ;

   error:
      SDB_OSS_FREE( _fieldName ) ;
      SDB_OSS_FREE( _sequenceName ) ;
      SAFE_OSS_DELETE( pSubItem ) ;
      if ( _pSubFieldMap )
      {
         delete _pSubFieldMap ;
         _pSubFieldMap = NULL ;
      }
      goto done ;
   }

}

