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

   Source File Name = sptReturnVal.cpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          31/03/2014  YW  Initial Draft

   Last Changed =

*******************************************************************************/

#include "sptReturnVal.hpp"
#include "ossUtil.hpp"
#include "pd.hpp"

using namespace bson ;

namespace engine
{
   _sptProperty::_sptProperty()
   :_value(0),
    _type(EOO)
   {
 
   }

   _sptProperty::_sptProperty( const _sptProperty &other )
   {
      _name = other._name ;
      _type = other._type ;
      _value = 0 ;
      if ( String == _type )
      {
         /// may return NULL.
         const CHAR *p = ossStrdup( ( const CHAR *)(other._value) ) ;
         if ( NULL != p )
         {
            _value = ( UINT64 )p ;
         }
      }
      else
      {
         _value = other._value ;
      }
   }

   _sptProperty::~_sptProperty()
   {
      if ( String == _type )
      {
         SDB_OSS_FREE( (CHAR *)_value ) ;
         _value = NULL ;
      }

      /// when type is object, the value will be free in JS_Destructor.
   }

   INT32 _sptProperty::assign( const CHAR *name,
                               bson::BSONType type,
                               const void *value )
   {
      INT32 rc = SDB_OK ;
      SDB_ASSERT( NumberDouble == type ||
                  String == type ||
                  Bool == type ||
                  NumberInt == type, "invalid value type" )
      SDB_ASSERT( NULL != name && NULL != value, "can not be NULL" )
      _value = 0 ;

      if ( NumberDouble == type )
      {
         FLOAT64 *v = (FLOAT64 *)(&_value) ;
         *v = *((const FLOAT64 *)value) ;
      }
      else if ( String == type )
      {
         CHAR *p = ossStrdup( (const CHAR *)value ) ;
         if ( NULL == p )
         {
            PD_LOG( PDERROR, "failed to allocate mem." ) ;
            rc = SDB_OOM ;
            goto error ;
         }
         _value = (UINT64)p ;
      }
      else if ( Bool == type )
      {
         BOOLEAN *v = (BOOLEAN *)(&_value);
         *v = *((const BOOLEAN *)value) ;
      }
      else
      {
         INT32 *v = (INT32 *)(&_value) ;
         *v = *((const INT32 *)value) ;
      }

      _name.assign( name ) ;
      _type = type ;
   done:
      return rc ;
   error:
      goto done ;
   }


   void _sptProperty::assignObject( const CHAR *name,
                                    void *value )
   {
      SDB_ASSERT( NULL != name && NULL != value, "can not be NULL" )
      _value = 0 ;
      _name.assign(name);
      _type = Object ;
      _value = ( UINT64 )value ;
      return ;
   }

   INT32 _sptProperty::getINT32() const
   {
      SDB_ASSERT( NumberInt == _type, "type must be int" )
      return ( INT32 )_value ;
   }

   FLOAT64 _sptProperty::getFLOAT64() const
   {
      SDB_ASSERT( NumberDouble == _type, "type must be double" )
      return ( FLOAT64 )_value ;
   }

   BOOLEAN _sptProperty::getBool() const
   {
      SDB_ASSERT( Bool == _type, "type must be bool" )
      return ( BOOLEAN )_value ;
   }

   const CHAR *_sptProperty::getString() const
   {
      SDB_ASSERT( String == _type, "type must be string" )
      return ( CHAR * )_value ;
   }

   void _sptProperty::assignVoid()
   {
      SDB_ASSERT( EOO == _type, "do not reassign the value." )
      if ( String == _type )
      {
         SDB_OSS_FREE( (CHAR *)_value ) ;
         _value = 0 ;   
      }
      _name.clear() ;
      return ;
   }

////////////////////////////////////////////////////

   INT32 _sptReturnVal::setNativeVal( const CHAR *name,
                                      bson::BSONType type,
                                      const void *value )
   {
      INT32 rc = SDB_OK ;
      rc = _property.assign( name, type, value ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to assign property:%d", rc ) ;
         goto error ;
      }
   done:
      return rc ;
   error:
      goto done ;
   }

   void _sptReturnVal::setObjectVal( const CHAR *name,
                                     void *value,
                                     const void *classDef )
   {
      SDB_ASSERT( NULL != classDef, "class def can not be NULL" )
      _classDef = classDef ;
      _property.assignObject( name, value ) ;
      return ;
   }
}

