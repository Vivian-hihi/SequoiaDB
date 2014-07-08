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
#include "ossUtil.hpp"

using namespace bson ;

namespace engine
{
   #define OM_GENERATOR_DOT    ","
   #define OM_GENERATOR_LINE   "-"

   #define OM_TEMPLATE_REPLICA_NUM     "replica_num"
   #define OM_TEMPLATE_DATAGROUP_NUM   "data_group_num"
   #define OM_TEMPLATE_CATALOG_NUM     "catalog_num"
   #define OM_TEMPLATE_COORD_NUM       "coord_num"
   #define OM_TEMPLATE_USER_NAME       "user_name"
   #define OM_TEMPLATE_USER_PASSWD     "user_passwd"
   #define OM_TEMPLATE_USER_GROUP      "user_group"


   #define OM_CONF_DETAIL_DBPATH       "dbpath"
   #define OM_CONF_DETAIL_SVCNAME      "svcname"
   #define OM_CONF_DETAIL_REPLNAME     "replname"
   #define OM_CONF_DETAIL_SHARDNAME    "shardname"
   #define OM_CONF_DETAIL_CATALOGNAME  "catalogname"
   #define OM_CONF_DETAIL_HTTPNAME     "httpname"
   #define OM_CONF_DETAIL_DIAGLEVEL    "diaglevel"
   #define OM_CONF_DETAIL_ROLE         "role"
   #define OM_CONF_DETAIL_LOGFSIZE     "logfilesz"
   #define OM_CONF_DETAIL_LOGFNUM      "logfilenum"
   #define OM_CONF_DETAIL_TRANSACTION  "transactionon"
   #define OM_CONF_DETAIL_PREINSTANCE  "preferedinstance"
   #define OM_CONF_DETAIL_PCNUM        "numpagecleaners"
   #define OM_CONF_DETAIL_PCINTERVAL   "pagecleaninterval"

   static INT32 getBsonStringField( const BSONObj &bsonTemplate, 
                                    const string &fieldName, string &value ) ;

   static string omStringTrim( string &value ) ;


   void sdbConfTemplate::init()
   {
      businessType = "" ;
      clusterType  = "" ;
      replicaNum   = -1 ;
      dataGroupNum = -1 ;
      catalogNum   = -1 ;
      coordNum     = -1 ;
      userName     = "" ;
      userPasswd   = "" ;
      userGroup    = "" ;
   }

   void sdbConfDetail::init() 
   {
      dbPath            = "" ;
      svcName           = -1 ;
      replName          = -1 ;
      shardName         = -1 ;
      catalogName       = -1 ;
      httpName          = -1 ;
      dialevel          = -1 ;
      role              = "" ;
      logFileSize       = -1 ;
      logFileNum        = -1 ;
      transaction       = FALSE ;
      preferedInstance  = "" ;
      numPageCleaner    = -1 ;
      pageCleanInterval = -1 ;
   }

   rangeValidator::rangeValidator( string singleRange ) 
   {
      rangeValidator( singleRange, singleRange, TRUE ) ;
   }

   rangeValidator::rangeValidator( string begin, string end, BOOLEAN isClosed )
   {
      _isClosed = isClosed ;
      _begin    = begin ;
      _end      = end ;

      omStringTrim( _begin ) ;
      omStringTrim( _end ) ;

      if ( ( _begin.length() == 0 ) && ( _end.length() == 0 ) )
      {
         /* if range is empty, all the value is valid */
         _isValidAll = TRUE ;
      }
   }

   BOOLEAN rangeValidator::isValid( const string &value )
   {
      INT32 compareBegin ;
      INT32 compareEnd ;

      if ( _isValidAll )
      {
         return TRUE ;
      }

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

   string rangeValidator::getMinValidValue()
   {
      return _begin ;
   }

   omConfigItem::omConfigItem()
   {
   }

   omConfigItem::~omConfigItem()
   {
      list<rangeValidator*>::iterator ite = _validatorList.begin() ;
      while ( ite != _validatorList.end() )
      {
         rangeValidator *pRV = *ite ;
         delete pRV ;

         _validatorList.erase( ite++ ) ;
      }
   }

   void omConfigItem::_addRange( const string &value )
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
         rv = new rangeValidator( tmpValue ) ;
      }

      _validatorList.push_back( rv ) ;
   }

   INT32 omConfigItem::init( const BSONObj &bsonItem )
   {
      string::size_type pos1 ;
      string::size_type pos2 ;
      INT32 rc = SDB_OK ;

      rc = getBsonStringField( bsonItem, OM_BSON_PROPERTY_NAME, _name ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "get field failed:field=%s", OM_BSON_PROPERTY_NAME ) ;
         goto error ;
      }

      rc = getBsonStringField( bsonItem, OM_BSON_PROPERTY_DEFAULT, 
                               _defaultValue ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "get field failed:field=%s", 
                 OM_BSON_PROPERTY_DEFAULT ) ;
         goto error ;
      }

      rc = getBsonStringField( bsonItem, OM_BSON_PROPERTY_VALID, _valid ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "get field failed:field=%s", 
                 OM_BSON_PROPERTY_VALID ) ;
         goto error ;
      }

      pos1          = 0 ;
      pos2          = _valid.find( OM_GENERATOR_DOT ) ;
      while( string::npos != pos2 ) 
      {
         _addRange( _valid.substr( pos1, pos2 - pos1 ) ) ;

         pos1 = pos2 + 1 ;
         pos2 = _valid.find( OM_GENERATOR_DOT, pos1 ) ;
      }

      _addRange( _valid.substr( pos1 ) ) ;

      if ( !isValid( _defaultValue ) )
      {
         rc = SDB_INVALIDARG ;
         PD_LOG( PDERROR, "default value is invalid:field=%s,value=%s", 
                 OM_BSON_PROPERTY_DEFAULT, _defaultValue.c_str() ) ;
         goto error ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   string omConfigItem::getDefaultValue()
   {
      return _defaultValue ;
   }
   
   string omConfigItem::getItemName()
   {
      return _name ;
   }

   BOOLEAN omConfigItem::isValid( const string &value )
   {
      VALIDATORLIST_ITER iter = _validatorList.begin() ;
      while ( iter != _validatorList.end() )
      {
         if ( ( *iter )->isValid( value ) )
         {
            return TRUE ;
         }

         iter++ ;
      }

      return FALSE ;
   }

   string omConfigItem::getMinValidValue()
   {
      VALIDATORLIST_ITER iter = _validatorList.begin() ;
      rangeValidator *rv = *iter ;
      return rv->getMinValidValue() ;
   }

   string omConfigItem::getValidString()
   {
      return _valid ;
   }

   omHostInfo::omHostInfo()
   {
   }

   omHostInfo::~omHostInfo()
   {
   }

   INT32 omHostInfo::init( const BSONObj &oneHost, 
                           map<string, omConfigItem*> *confDetailMap )
   {
      INT32 rc = SDB_OK ;
      BSONElement hostElement ;
      CONFIGITEMMAP_ITER iter ;
      string minValue ;

      _diskCount       = 0 ;
      _unusedDiskCount = 0 ;
      _standAloneCount = 0 ;
      _coordCount      = 0 ;
      _catalogCount    = 0 ;
      _dataCount       = 0 ;
      _pConfDetailMap = confDetailMap ;

      rc = getBsonStringField( oneHost, OM_BSON_FIELD_HOST_IP, _ip ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "host info miss field=%s", OM_BSON_FIELD_HOST_IP ) ;
         goto error ;
      }

      rc = getBsonStringField( oneHost, OM_BSON_FIELD_HOST_NAME, _hostName ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "host info miss field=%s", OM_BSON_FIELD_HOST_NAME ) ;
         goto error ;
      }

      hostElement = oneHost.getField( OM_BSON_FIELD_DISK ) ;
      if ( hostElement.eoo() || Array != hostElement.type() )
      {
         rc = SDB_INVALIDARG ;
         PD_LOG( PDERROR, "field is not array type:field=%s,type=%d", 
                 OM_BSON_FIELD_DISK, hostElement.type() ) ;
         goto error ;
      }
      {
         BSONObjIterator i( hostElement.embeddedObject() ) ;
         while ( i.more() )
         {
            omDiskInfo disk ;
            string tmp ;
            BSONElement ele = i.next() ;
            if ( Object != ele.type() )
            {
               rc = SDB_INVALIDARG ;
               PD_LOG( PDERROR, "array's element is invalid:element=%s", 
                       ele.toString().c_str() ) ;
               goto error ;
            }

            BSONObj oneDisk = ele.embeddedObject() ;
            getBsonStringField( oneDisk, OM_BSON_FIELD_DISK_NAME, 
                                disk.diskName );
            getBsonStringField( oneDisk, OM_BSON_FIELD_DISK_SIZE, tmp );
            disk.totalSize = ossAtoll( tmp.c_str() ) ;
            getBsonStringField( oneDisk, OM_BSON_FIELD_DISK_MOUNT, 
                                disk.mountPath );
            getBsonStringField( oneDisk, OM_BSON_FIELD_DISK_FREE_SIZE, tmp );
            disk.freeSize = ossAtoll( tmp.c_str() ) ;
            getBsonStringField( oneDisk, OM_BSON_FIELD_DISK_USED, tmp );
            disk.isUsed = ( ossStrcasecmp( tmp.c_str(), "true" ) == 0 )
                          ? TRUE : FALSE ;
            _diskList.push_back( disk ) ;

            _diskCount++ ;
            _unusedDiskCount++ ;
         }
      }

      iter = _pConfDetailMap->find( OM_CONF_DETAIL_SVCNAME ) ;
      SDB_ASSERT( iter != _pConfDetailMap->end(), "" ) ;
      minValue          = iter->second->getMinValidValue() ;
      _availableSvcName = ossAtoi( minValue.c_str() ) ;

   done:
      return rc ;
   error:
      _pConfDetailMap = NULL ;
      _diskList.clear() ;
      goto done ;
   }

   INT32 omHostInfo::assign( string role )
   {
      
      return 0 ;
   }
   
   INT32 omHostInfo::getNodeCount( string role )
   {
      return _coordCount ;
   }

   INT32 omHostInfo::getDiskCount()
   {
      return _diskCount ;
   }

   INT32 omHostInfo::getUnusedDiskCount()
   {
      return _unusedDiskCount ;
   }

   omConfigGenerator::omConfigGenerator()
   {
   }

   omConfigGenerator::~omConfigGenerator()
   {
      _clear() ;
   }

   INT32 omConfigGenerator::_parseHostInfo( const BSONObj &bsonHostInfo )
   {
      INT32 rc = SDB_OK ;
      //TODO:
      omHostInfo host ;
      host.init( bsonHostInfo, &_confDetailMap ) ;
      return SDB_OK ;
   }

   INT32 omConfigGenerator::generateSDBConfig( const BSONObj &bsonTemplate, 
                                               const BSONObj &bsonConfigDetails, 
                                               const BSONObj &bsonHostInfo, 
                                               BSONObj &bsonConfig )
   {
      INT32 rc = SDB_OK ;
      rc = _parseTemplate( bsonTemplate ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "get template failed:rc=%d", rc ) ;
         goto error ;
      }

      rc = _parseConfigDetail( bsonConfigDetails ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "get template failed:rc=%d", rc ) ;
         goto error ;
      }

      rc = _parseHostInfo( bsonHostInfo ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "get template failed:rc=%d", rc ) ;
         goto error ;
      }

      rc = _generateConfig( bsonConfig ) ;
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

   string omConfigGenerator::getErrorDetail()
   {
      return _errorDetail ;
   }

   void omConfigGenerator::_addToItemMap( const string &itemName, 
                                          omConfigItem* pItem )
   {
      CONFIGITEMMAP_ITER iter = _confDetailMap.find( itemName ) ;
      if ( iter != _confDetailMap.end() )
      {
         omConfigItem *old = iter->second ;
         delete old ;
         _confDetailMap.erase( iter ) ;
      }

      _confDetailMap.insert( CONFIGITEMMAP_TYPE( itemName, pItem ) ) ;
   }

   void omConfigGenerator::_clear() 
   {
      CONFIGITEMMAP_ITER iter = _confDetailMap.begin() ;
      while ( iter != _confDetailMap.end() )
      {
         omConfigItem *old = iter->second ;
         delete old ;

         _confDetailMap.erase( iter++ ) ;
      }

      _confTemplate.init() ;
   }

   INT32 omConfigGenerator::_setTemplateValue( const BSONObj &templateItem ) 
   {
      INT32 rc = SDB_OK ;
      string itemName ;
      string itemValue ;
      omConfigItem item ;
      rc = getBsonStringField( templateItem, OM_BSON_PROPERTY_NAME, 
                               itemName ) ;
      if ( SDB_OK != rc )
      {
         _errorDetail = string( "Template miss bson field=" )
                        + OM_BSON_PROPERTY_NAME ;
         PD_LOG( PDERROR, "%s", _errorDetail.c_str() ) ;
         goto error ;
      }

      rc = getBsonStringField( templateItem, OM_BSON_PROPERTY_VALUE, 
                               itemValue ) ;
      if ( SDB_OK != rc )
      {
         _errorDetail = string( "Template miss bson field=" )
                        + OM_BSON_PROPERTY_VALUE ;
         PD_LOG( PDERROR, "%s", _errorDetail.c_str() ) ;
         goto error ;
      }

      if ( itemName.compare( OM_TEMPLATE_REPLICA_NUM ) == 0 )
      {
         _confTemplate.replicaNum   = ossAtoi( itemValue.c_str() ) ;
      }
      else if ( itemName.compare( OM_TEMPLATE_DATAGROUP_NUM ) == 0 )
      {
         _confTemplate.dataGroupNum = ossAtoi( itemValue.c_str() ) ;
      }
      else if ( itemName.compare( OM_TEMPLATE_CATALOG_NUM ) == 0 )
      {
         _confTemplate.catalogNum   = ossAtoi( itemValue.c_str() ) ;
      }
      else if ( itemName.compare( OM_TEMPLATE_COORD_NUM ) == 0 )
      {
         _confTemplate.coordNum     = ossAtoi( itemValue.c_str() ) ;
      }
      else if ( itemName.compare( OM_TEMPLATE_USER_NAME ) == 0 )
      {
         _confTemplate.userName     = itemValue ;
      }
      else if ( itemName.compare( OM_TEMPLATE_USER_PASSWD ) == 0 )
      {
         _confTemplate.userPasswd   = itemValue ;
      }
      else if ( itemName.compare( OM_TEMPLATE_USER_GROUP ) == 0 )
      {
         _confTemplate.userGroup    = itemValue ;
      }

      rc = item.init( templateItem ) ;
      if ( SDB_OK != rc )
      {
         _errorDetail = string( "Template bson format error" ) ;
         PD_LOG( PDERROR, "Template bson format error:rc=%d", rc ) ;
         goto error ;
      }

      if ( !item.isValid( itemValue ) )
      {
         rc = SDB_INVALIDARG ;
         _errorDetail = string("Template value is invalid:item=") 
                        + OM_BSON_PROPERTY_NAME + ",value=" 
                        + itemValue.c_str() ;
         PD_LOG( PDERROR, "%s", _errorDetail.c_str() ) ;
         goto error ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   BOOLEAN omConfigGenerator::_isAllTemplateSet()
   {
      if ( _confTemplate.replicaNum == -1 )
      {
         _errorDetail = string( OM_TEMPLATE_REPLICA_NUM ) 
                        + " have not been set" ;
         return FALSE ;
      }
      else if ( _confTemplate.dataGroupNum == -1 )
      {
         _errorDetail = string( OM_TEMPLATE_DATAGROUP_NUM ) 
                        + " have not been set" ;
         return FALSE ;
      }
      else if ( _confTemplate.catalogNum == -1 )
      {
         _errorDetail = string( OM_TEMPLATE_CATALOG_NUM ) 
                        + " have not been set" ;
         return FALSE ;
      }
      else if ( _confTemplate.coordNum == -1 )
      {
         _errorDetail = string( OM_TEMPLATE_COORD_NUM ) 
                        + " have not been set" ;
         return FALSE ;
      }
      else if ( _confTemplate.userName.length() == 0 )
      {
         _errorDetail = string( OM_TEMPLATE_USER_NAME ) 
                        + " have not been set" ;
         return FALSE ;
      }
      else if ( _confTemplate.userPasswd.length() == 0 )
      {
         _errorDetail = string( OM_TEMPLATE_USER_PASSWD ) 
                        + " have not been set" ;
         return FALSE ;
      }
      else if ( _confTemplate.userGroup.length() == 0 )
      {
         _errorDetail = string( OM_TEMPLATE_USER_GROUP ) 
                        + " have not been set" ;
         return FALSE ;
      }

      return TRUE ;
   }

   INT32 omConfigGenerator::_parseTemplate( const BSONObj &bsonTemplate )
   {
      INT32 rc        = SDB_OK ;
      string tmpValue = "" ;
      BSONObj properyItem ;
      BSONElement propertyElement ;
      _confTemplate.init() ;
      rc = getBsonStringField( bsonTemplate, OM_BSON_BUSINESS_TYPE, 
                               _confTemplate.businessType ) ;
      if ( SDB_OK != rc )
      {
         _errorDetail = string( "Template miss bson field=" ) 
                        + OM_BSON_BUSINESS_TYPE ;
         PD_LOG( PDERROR, "%s", _errorDetail.c_str() ) ;
         goto error ;
      }

      rc = getBsonStringField( bsonTemplate, OM_BSON_CLUSTER_TYPE, 
                               _confTemplate.clusterType ) ;
      if ( SDB_OK != rc )
      {
         _errorDetail = string( "Template miss bson field=" ) 
                        + OM_BSON_CLUSTER_TYPE ;
         PD_LOG( PDERROR, "%s", _errorDetail.c_str() ) ;
         goto error ;
      }

      propertyElement = bsonTemplate.getField( OM_BSON_PROPERTY_ARRAY ) ;
      if ( propertyElement.eoo() || Array != propertyElement.type() )
      {
         rc = SDB_INVALIDARG ;
         _errorDetail = string( "Template miss bson array field=" ) 
                        + OM_BSON_PROPERTY_ARRAY ;
         PD_LOG( PDERROR, "field is not array type:field=%s,type=%d", 
                 OM_BSON_PROPERTY_ARRAY, propertyElement.type() ) ;
         goto error ;
      }
      {
         BSONObjIterator i( propertyElement.embeddedObject() ) ;
         while ( i.more() )
         {
            BSONElement ele = i.next() ;
            if ( Object != ele.type() )
            {
               rc = SDB_INVALIDARG ;
               PD_LOG( PDERROR, "array's element is invalid:element=%s", 
                       ele.toString().c_str() ) ;
               goto error ;
            }
            
            BSONObj oneProperty = ele.embeddedObject() ;
            rc = _setTemplateValue( oneProperty ) ;
            if ( SDB_OK != rc )
            {
               PD_LOG( PDERROR, "_setPropertyItem failed:rc=%d", rc ) ;
               goto error ;
            }
         }
      }

      if ( !_isAllTemplateSet() )
      {
         rc = SDB_INVALIDARG ;
         PD_LOG( PDERROR, "miss template configur item" ) ;
         goto error ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 omConfigGenerator::_setConfDetailValue( const BSONObj &oneProperty )
   {
      string itemName ;
      string itemValue ;
      INT32 rc           = SDB_OK ;
      omConfigItem *pItem = NULL ;;

      pItem = new omConfigItem() ;
      rc    = pItem->init( oneProperty ) ;
      if ( SDB_OK != rc )
      {
         _errorDetail = string( "conf detail bson format error" ) ;
         PD_LOG( PDERROR, "conf detail bson format error:rc=%d", rc ) ;
         goto error ;
      }

      itemName  = pItem->getItemName() ;
      _addToItemMap( itemName, pItem ) ;
      pItem = NULL ;

   done:
      return rc ;
   error:
      if ( NULL != pItem )
      {
         delete pItem ;
      }
      goto done ;
   }

   BOOLEAN omConfigGenerator::_isAllConfDetailSet()
   {
      CONFIGITEMMAP_ITER iter = _confDetailMap.find( OM_CONF_DETAIL_DBPATH ) ;
      if ( iter == _confDetailMap.end() )
      {
         _errorDetail = string( OM_CONF_DETAIL_DBPATH ) 
                        + " have not been set" ;
         return FALSE ;
      }
      iter = _confDetailMap.find( OM_CONF_DETAIL_SVCNAME ) ;
      if ( iter == _confDetailMap.end() )
      {
         _errorDetail = string( OM_CONF_DETAIL_SVCNAME ) 
                        + " have not been set" ;
         return FALSE ;
      }
      iter = _confDetailMap.find( OM_CONF_DETAIL_REPLNAME ) ;
      if ( iter == _confDetailMap.end() )
      {
         _errorDetail = string( OM_CONF_DETAIL_REPLNAME ) 
                        + " have not been set" ;
         return FALSE ;
      }
      iter = _confDetailMap.find( OM_CONF_DETAIL_SHARDNAME ) ;
      if ( iter == _confDetailMap.end() )
      {
         _errorDetail = string( OM_CONF_DETAIL_SHARDNAME ) 
                        + " have not been set" ;
         return FALSE ;
      }
      iter = _confDetailMap.find( OM_CONF_DETAIL_CATALOGNAME ) ;
      if ( iter == _confDetailMap.end() )
      {
         _errorDetail = string( OM_CONF_DETAIL_CATALOGNAME ) 
                        + " have not been set" ;
         return FALSE ;
      }
      iter = _confDetailMap.find( OM_CONF_DETAIL_HTTPNAME ) ;
      if ( iter == _confDetailMap.end() )
      {
         _errorDetail = string( OM_CONF_DETAIL_HTTPNAME ) 
                        + " have not been set" ;
         return FALSE ;
      }
      iter = _confDetailMap.find( OM_CONF_DETAIL_DIAGLEVEL ) ;
      if ( iter == _confDetailMap.end() )
      {
         _errorDetail = string( OM_CONF_DETAIL_DIAGLEVEL ) 
                        + " have not been set" ;
         return FALSE ;
      }
      iter = _confDetailMap.find( OM_CONF_DETAIL_ROLE ) ;
      if ( iter == _confDetailMap.end() )
      {
         _errorDetail = string( OM_CONF_DETAIL_ROLE ) 
                        + " have not been set" ;
         return FALSE ;
      }
      iter = _confDetailMap.find( OM_CONF_DETAIL_LOGFSIZE ) ;
      if ( iter == _confDetailMap.end() )
      {
         _errorDetail = string( OM_CONF_DETAIL_LOGFSIZE ) 
                        + " have not been set" ;
         return FALSE ;
      }
      iter = _confDetailMap.find( OM_CONF_DETAIL_LOGFNUM ) ;
      if ( iter == _confDetailMap.end() )
      {
         _errorDetail = string( OM_CONF_DETAIL_LOGFNUM ) 
                        + " have not been set" ;
         return FALSE ;
      }
      iter = _confDetailMap.find( OM_CONF_DETAIL_TRANSACTION ) ;
      if ( iter == _confDetailMap.end() )
      {
         _errorDetail = string( OM_CONF_DETAIL_TRANSACTION ) 
                        + " have not been set" ;
         return FALSE ;
      }
      iter = _confDetailMap.find( OM_CONF_DETAIL_PREINSTANCE ) ;
      if ( iter == _confDetailMap.end() )
      {
         _errorDetail = string( OM_CONF_DETAIL_PREINSTANCE ) 
                        + " have not been set" ;
         return FALSE ;
      }iter = _confDetailMap.find( OM_CONF_DETAIL_PCNUM ) ;
      if ( iter == _confDetailMap.end() )
      {
         _errorDetail = string( OM_CONF_DETAIL_PCNUM ) 
                        + " have not been set" ;
         return FALSE ;
      }
      iter = _confDetailMap.find( OM_CONF_DETAIL_PCINTERVAL ) ;
      if ( iter == _confDetailMap.end() )
      {
         _errorDetail = string( OM_CONF_DETAIL_PCINTERVAL ) 
                        + " have not been set" ;
         return FALSE ;
      }

      return TRUE ;
   }
   
   INT32 omConfigGenerator::_parseConfigDetail( 
                                        const BSONObj &bsonConfigDetails )
   {
      INT32 rc = SDB_OK ;
      BSONElement propertyElement ;
      _clear() ;

      propertyElement = bsonConfigDetails.getField( OM_BSON_PROPERTY_ARRAY ) ;
      if ( propertyElement.eoo() || Array != propertyElement.type() )
      {
         rc = SDB_INVALIDARG ;
         _errorDetail = string( "configDetails miss bson array field=" ) 
                        + OM_BSON_PROPERTY_ARRAY ;
         PD_LOG( PDERROR, "field is not array type:field=%s,type=%d", 
                 OM_BSON_PROPERTY_ARRAY, propertyElement.type() ) ;
         goto error ;
      }
      {
         BSONObjIterator i( propertyElement.embeddedObject() ) ;
         while ( i.more() )
         {
            BSONElement ele = i.next() ;
            if ( Object != ele.type() )
            {
               rc = SDB_INVALIDARG ;
               PD_LOG( PDERROR, "array's element is invalid:element=%s", 
                       ele.toString().c_str() ) ;
               goto error ;
            }
            
            BSONObj oneProperty = ele.embeddedObject() ;
            rc = _setConfDetailValue( oneProperty ) ;
            if ( SDB_OK != rc )
            {
               PD_LOG( PDERROR, "_setConfDetailValue failed:rc=%d", rc ) ;
               goto error ;
            }
         }
      }

      if ( !_isAllConfDetailSet() )
      {
         rc = SDB_INVALIDARG ;
         PD_LOG( PDERROR, "miss confDetail configur item" ) ;
         goto error ;
      }

   done:
      return rc ;
   error:
      _clear() ;
      goto done ;
   }

   INT32 omConfigGenerator::_generateConfig( const BSONObj &bsonConfig )
   {
      INT32 rc = SDB_OK ;
      if ( SDB_OK != rc )
      {
         goto error ;
      }
   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 getBsonStringField( const BSONObj &bsonTemplate, 
                             const string &fieldName,
                             string &value )
   {
      INT32 rc = SDB_OK ;
      BSONElement element = bsonTemplate.getField( fieldName ) ;
      if ( element.eoo() )
      {
         rc = SDB_INVALIDARG ;
         goto error ;
      }

      if ( String == element.type() )
      {
         value = element.String() ;
      }
      else if ( NumberInt == element.type() )
      {
         CHAR tmp[20] ;
         ossSnprintf( tmp, sizeof(tmp), "%d", element.Int() ) ;
         value = string( tmp ) ;
      }
      else if ( NumberLong == element.type() )
      {
         CHAR tmp[40] ;
         ossSnprintf( tmp, sizeof(tmp), OSS_LL_PRINT_FORMAT, element.Long() ) ;
         value = string( tmp ) ;
      }
      else
      {
         rc = SDB_INVALIDARG ;
         goto error ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   string omStringTrim( string &value )
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
}





