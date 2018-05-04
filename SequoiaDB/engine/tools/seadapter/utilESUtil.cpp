/*******************************************************************************


   Copyright (C) 2011-2018 SequoiaDB Ltd.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

   Source File Name = utilESUtil.cpp

   Descriptive Name = Elasticsearch utility.

   When/how to use: this program may be used on binary and text-formatted
   versions of PMD component. This file contains main function for sdbcm,
   which is used to do cluster managing.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          15/03/2018  YSD  Initial Draft

   Last Changed =

*******************************************************************************/
#include "pd.hpp"
#include "utilESUtil.hpp"
#include <sstream>

using bson::BSONObjBuilder ;

namespace seadapter
{
   static const CHAR* getTypeStr( ES_DATA_TYPE type )
   {
      switch ( type )
      {
         case ES_TEXT:
            return "text" ;
         case ES_KEYWORD:
            return "keyword" ;
         case ES_DATE:
            return "date" ;
         case ES_LONG:
            return "long" ;
         case ES_DOUBLE:
            return "double" ;
         case ES_BOOLEAN:
            return "boolean" ;
         case ES_IP:
            return "ip" ;
         case ES_OBJECT:
            return "object" ;
         case ES_NESTED:
            return "nested" ;
         case ES_GEO_POINT:
            return "geo_point" ;
         case ES_GEO_SHAPE:
            return "geo_shape" ;
         case ES_COMPLETION:
            return "completion" ;
         default:
            return NULL ;
      } ;
   }

   _utilESMapProp::_utilESMapProp( const CHAR *name, ES_DATA_TYPE type )
   {
      SDB_ASSERT( name, "Name is NULL" ) ;
      _name = string( name ) ;
      _type = type ;
   }

   _utilESMapProp::~_utilESMapProp()
   {
   }

   _utilESMapping::_utilESMapping( const CHAR *index, const CHAR *type )
   {
      SDB_ASSERT( index, "Index is NULL" ) ;
      SDB_ASSERT( type, "type is NULL" ) ;

      _index = string( index ) ;
      _type = string( type ) ;
   }

   _utilESMapping::~_utilESMapping()
   {
   }

   INT32 _utilESMapping::addProperty( const CHAR *name, ES_DATA_TYPE type,
                                      BSONObj *parameters )
   {
      INT32 rc = SDB_OK ;

      if ( !name )
      {
         rc = SDB_INVALIDARG ;
         goto error ;
      }

      _properties.push_back( utilESMapProp( name, type ) ) ;
   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _utilESMapping::toObj( BSONObj &mapObj ) const
   {
      INT32 rc = SDB_OK ;
      try
      {
         BSONObjBuilder builder ;

         BSONObjBuilder mapBuilder( builder.subobjStart( "mappings" ) ) ;
         BSONObjBuilder typeBuilder( mapBuilder.subobjStart( _type.c_str() ) ) ;
         BSONObjBuilder propBuilder( typeBuilder.subobjStart( "properties" ) ) ;

         for ( vector<_utilESMapProp>::const_iterator itr = _properties.begin() ;
               itr != _properties.end(); ++itr )
         {
            BSONObjBuilder fieldBuilder(
               propBuilder.subobjStart( itr->getName().c_str() ) ) ;

            fieldBuilder.append( "type", getTypeStr( itr->getType() ) ) ;
            fieldBuilder.done() ;
         }

         propBuilder.done() ;
         typeBuilder.done() ;
         mapBuilder.done() ;

         mapObj = builder.obj() ;
      }
      catch ( std::exception &e )
      {
         rc = SDB_SYS ;
         PD_LOG( PDERROR, "Unexpected exception occurred: %s", e.what() ) ;
         goto error ;
      }
   done:
      return rc ;
   error:
      goto done ;
   }
}

