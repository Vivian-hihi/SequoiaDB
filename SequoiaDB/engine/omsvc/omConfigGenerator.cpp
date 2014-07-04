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

   Source File Name = omConfigGenerator.cpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          07/03/2014  LYB Initial Draft

   Last Changed =

*******************************************************************************/


#include "omConfigGenerator.hpp"
#include "omDef.hpp"
#include "pd.hpp"

using namespace bson ;

namespace engine
{
   #define OM_GENERATOR_DOT    ","
   #define OM_GENERATOR_LINE   "-"

   //*****************config generator****************************
   struct sdbConfTemplate
   {
      string businessType ;
      string clusterType ;
      INT32 replicaNum ;
      INT32 dataGroupNum ;
      INT32 catalogNum ;
      INT32 coordNum ;
      string userName ;
      string userPasswd ;
      string userGroup ;
   } ;

   struct configItems
   {
      string dbPath ;
      INT32 svnName ;
      INT32 replName ;
      INT32 shardName ;
      INT32 catalogName ;
      INT32 httpName ;
      INT32 diagLevel ;
      string role ;
      INT32 logfileSize ;
      INT32 logfileNum ;
      BOOLEAN transaction ;
      string preferedInstance ;
      INT32 numPageCleaners ;
      INT32 pageCleanInterval ;
   } ;

   struct omHostInfo
   {
      string IP ;
      string hostName ;
      
   } ;

   class rangeValidator
   {
      public:
         rangeValidator( string singleRange ) ;
         rangeValidator( string begin, string end, BOOLEAN isClosed = TRUE ) ;

      public:
         BOOLEAN     isValid( const string &value ) ;

      private:
         BOOLEAN     _isClosed ;
         string      _begin ;
         string      _end ;
   } ;

   class ConfigItem
   {
      public:
         ConfigItem() ;
         ~ConfigItem() ;

      public:
         INT32       init( const BSONObj &bsonItem ) ;
         string      getDefaultValue() ;
         string      getItemName() ;
         BOOLEAN     isValid( const string &value ) ;

      private:
         string      _trim( string &value ) ;
         void        _addRange( const string &value ) ;

      private:
         string                 _name ;
         string                 _defaultValue ;
         list<rangeValidator *> _validatorList ;
   } ;

   rangeValidator::rangeValidator( string singleRange ) 
   {
      rangeValidator( singleRange, singleRange, TRUE ) ;
   }

   rangeValidator::rangeValidator( string begin, string end, BOOLEAN isClosed )
   {
      _isClosed = isClosed ;
      _begin    = begin ;
      _end      = end ;
   }

   BOOLEAN rangeValidator::isValid( const string &value )
   {
      INT32 compareBegin ;
      INT32 compareEnd ;

      compareEnd   = value.compare( _end ) ;

      if ( _isClosed )
      {
         if ( compareEnd == 0 )
         {
            return TRUE ;
         }
      }

      compareBegin = value.compare( _begin ) ;
      if ( compareBegin >= 0 && compareEnd < 0 )
      {
         return TRUE ;
      }

      return FALSE ;
   }

   ConfigItem::ConfigItem()
   {
   }

   ConfigItem::~ConfigItem()
   {
      list<rangeValidator*>::iterator ite = _validatorList.begin() ;
      while ( ite != _validatorList.end() )
      {
         rangeValidator *pRV = *ite ;
         delete pRV ;

         _validatorList.erase( ite++ ) ;
      }
   }

   string ConfigItem::_trim( string &value )
   {
      string::size_type pos = value.find_first_not_of( " " ) ;
      value.erase( 0, pos ) ;

      pos = value.find_last_not_of( " " ) ;
      if ( pos == string::npos )
      {
         value.erase() ;
      }
      else
      {
         value.erase( pos + 1 ) ;
      }

      return value ;
   }

   void ConfigItem::_addRange( const string &value )
   {
      rangeValidator *rv       = NULL ;
      string tmpValue          = value ;
      string::size_type posTmp = tmpValue.find( OM_GENERATOR_LINE ) ;

      if( string::npos != posTmp )
      {
         rv = new rangeValidator( tmpValue.substr( 0, posTmp ), 
                                  tmpValue.substr( posTmp + 1 ) ) ;
      }
      else
      {
         rv = new rangeValidator( _trim( tmpValue ) ) ;
      }

      _validatorList.push_back( rv ) ;
   }

   INT32 ConfigItem::init( const BSONObj &bsonItem )
   {
      string valid ;
      string::size_type pos1 ;
      string::size_type pos2 ;

      _name         = bsonItem.getStringField( OM_BSON_PROPERTY_NAME ) ;
      _defaultValue = bsonItem.getStringField( OM_BSON_PROPERTY_DEFAULT ) ;
      valid         = bsonItem.getStringField( OM_BSON_PROPERTY_VALID ) ;

      pos1          = 0 ;
      pos2          = valid.find( OM_GENERATOR_DOT ) ;
      while( string::npos != pos2 ) 
      {
         _addRange( valid.substr( pos1, pos2 - pos1 ) ) ;

         pos1 = pos2 + 1 ;
         pos2 = valid.find( OM_GENERATOR_DOT, pos1 ) ;
      }

      _addRange( valid.substr( pos1 ) ) ;

      return SDB_OK ;
   }

   string ConfigItem::getDefaultValue()
   {
      return _defaultValue ;
   }
   
   string ConfigItem::getItemName()
   {
      return _name ;
   }

   /*   1.检查template合法性
        2.检查hostInfo合法性
        3.根据ConfigItem、TemplateItem和hostInfo为节点设置配置
        
   */
   INT32 getBsonStringField( const BSONObj &bsonTemplate, 
                             const string &fieldName,
                             string &value )
   {
      INT32 rc = SDB_OK ;
      BSONElement element = bsonTemplate.getField( fieldName ) ;
      if ( element.eoo() || String != element.type() )
      {
         rc = SDB_INVALIDARG ;
         goto error ;
      }

      value = element.String() ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 getTemplate( const BSONObj &bsonTemplate, sdbConfTemplate &conf )
   {
      INT32 rc = SDB_OK ;
//      getBsonStringField( OM_BSON_BUSINESS_TYPE ) ;
      return rc ;
   }
   
   INT32 generateSDBConfig( const BSONObj &bsonTemplate, 
                            const BSONObj &bsonConfigItem, 
                            const BSONObj &bsonHostInfo, BSONObj &bsonConfig )
   {
      INT32 rc = SDB_OK ;
      sdbConfTemplate confTempalte ;
      rc = getTemplate( bsonTemplate, confTempalte ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "get template failed:rc=%d", rc ) ;
         goto error ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }
}





