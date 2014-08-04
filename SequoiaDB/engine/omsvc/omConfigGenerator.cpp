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

   #define OM_CONF_DETAIL_DBPATH       "dbpath"
   #define OM_CONF_DETAIL_SVCNAME      "svcname"
//   #define OM_CONF_DETAIL_REPLNAME     "replName"
//   #define OM_CONF_DETAIL_SHARDNAME    "shardname"
//   #define OM_CONF_DETAIL_CLNAME       "catalogname"
//   #define OM_CONF_DETAIL_HTTPNAME     "httpname"
//   #define OM_CONF_DETAIL_CLADDRESS    "catalogaddr"
   #define OM_CONF_DETAIL_DIAGLEVEL    "diaglevel"
   #define OM_CONF_DETAIL_ROLE         "role"
   #define OM_CONF_DETAIL_LOGFSIZE     "logfilesz"
   #define OM_CONF_DETAIL_LOGFNUM      "logfilenum"
   #define OM_CONF_DETAIL_TRANSACTION  "transactionon"
   #define OM_CONF_DETAIL_PREINSTANCE  "preferedinstance"
   #define OM_CONF_DETAIL_PCNUM        "numpagecleaners"
   #define OM_CONF_DETAIL_PCINTERVAL   "pagecleaninterval"

   #define OM_DG_NAME_PATTERN          "DATAGROUP"

   #define OM_CLUSTER_TYPE_STANDALONE  "standalone"
   #define OM_CLUSTER_TYPE_CLUSTER     "cluster"

   #define OM_NODE_TYPE_STANDALONE     "standalone"
   #define OM_NODE_TYPE_COORD          "coord"
   #define OM_NODE_TYPE_CATALOG        "catalog"
   #define OM_NODE_TYPE_DATA           "data"

   #define OM_SVCNAME_STEP             (10)
   #define OM_PATH_LENGTH              (256)
   #define OM_INT32_LENGTH             (20)

   #define OM_CONF_VALUE_INT_TYPE      "int"


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
   }

   void sdbConfDetail::init() 
   {
      dbPath            = "" ;
      svcName           = -1 ;
      dialevel          = -1 ;
      role              = "" ;
      logFileSize       = -1 ;
      logFileNum        = -1 ;
      transaction       = FALSE ;
      preferedInstance  = "" ;
      numPageCleaner    = -1 ;
      pageCleanInterval = -1 ;
      dataGroupID       = "" ;

      user              = "" ;
      passwd            = "" ;
   }

   rangeValidator::rangeValidator( string type, const CHAR *value ) 
   {
      _isClosed   = TRUE ;
      _begin      = value ;
      _end        = value ;
      _isValidAll = FALSE ;
      _type       = type ;

      omStringTrim( _begin ) ;
      omStringTrim( _end ) ;

      if ( ( _begin.length() == 0 ) && ( _end.length() == 0 ) )
      {
         /* if range is empty, all the value is valid */
         _isValidAll = TRUE ;
      }
   }

   rangeValidator::rangeValidator( string type, const CHAR *begin, 
                                   const CHAR *end, BOOLEAN isClosed )
   {
      _isClosed   = isClosed ;
      _begin      = begin ;
      _end        = end ;
      _isValidAll = FALSE ;
      _type       = type ;

      omStringTrim( _begin ) ;
      omStringTrim( _end ) ;

      if ( ( _begin.length() == 0 ) && ( _end.length() == 0 ) )
      {
         /* if range is empty, all the value is valid */
         _isValidAll = TRUE ;
      }
   }

   rangeValidator::~rangeValidator()
   {
   }

   INT32 rangeValidator::_compare( string left, string right )
   {
      if ( _type.compare( OM_CONF_VALUE_INT_TYPE ) == 0 )
      {
         INT32 leftInt  = ossAtoi( left.c_str() ) ;
         INT32 rightInt = ossAtoi( right.c_str() ) ;
         return ( leftInt - rightInt ) ;
      }

      return left.compare( right ) ;
   }

   BOOLEAN rangeValidator::isValid( const string &value )
   {
      INT32 compareBegin ;
      INT32 compareEnd ;

      if ( _isValidAll )
      {
         return TRUE ;
      }

      compareEnd   = _compare( value, _end ) ;
      if ( _isClosed )
      {
         if ( compareEnd == 0 )
         {
            return TRUE ;
         }
      }

      compareBegin = _compare( value, _begin ) ;
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
         SDB_OSS_DEL pRV ;

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
         rv = SDB_OSS_NEW rangeValidator( _type,
                                          tmpValue.substr(0,posTmp).c_str(), 
                                          tmpValue.substr(posTmp+1).c_str() ) ;
      }
      else
      {
         rv = SDB_OSS_NEW rangeValidator( _type, tmpValue.c_str() ) ;
      }

      _validatorList.push_back( rv ) ;
   }

   INT32 omConfigItem::init( const BSONObj &bsonItem )
   {
      string::size_type pos1 ;
      string::size_type pos2 ;
      string tmp ;
      INT32 rc = SDB_OK ;

      rc = getBsonStringField( bsonItem, OM_BSON_PROPERTY_TYPE, _type ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "get field failed:field=%s", OM_BSON_PROPERTY_TYPE ) ;
         goto error ;
      }
      rc = getBsonStringField( bsonItem, OM_BSON_PROPERTY_WEBNAME, _webName ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "get field failed:field=%s", 
                 OM_BSON_PROPERTY_WEBNAME ) ;
         goto error ;
      }

      rc = getBsonStringField( bsonItem, OM_BSON_PROPERTY_DISPLAY, _display ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "get field failed:field=%s", 
                 OM_BSON_PROPERTY_DISPLAY ) ;
         goto error ;
      }

      rc = getBsonStringField( bsonItem, OM_BSON_PROPERTY_EDIT, _edit ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "get field failed:field=%s", OM_BSON_PROPERTY_EDIT ) ;
         goto error ;
      }

      rc = getBsonStringField( bsonItem, OM_BSON_PROPERTY_DESC, _desc ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "get field failed:field=%s", OM_BSON_PROPERTY_DESC ) ;
         goto error ;
      }

      rc = getBsonStringField( bsonItem, OM_BSON_PROPERTY_LEVEL, _level ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "get field failed:field=%s", 
                 OM_BSON_PROPERTY_LEVEL ) ;
         goto error ;
      }

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

      tmp = _valid.substr( pos1 ) ;
      _addRange( tmp ) ;

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

   INT32 omDiskInfo::getNodeCount( string role )
   {
      if ( role.compare( OM_NODE_TYPE_STANDALONE ) == 0 )
      {
         return standAloneCount ;
      }

      if ( role.compare( OM_NODE_TYPE_COORD ) == 0 )
      {
         return coordCount ;
      }

      if ( role.compare( OM_NODE_TYPE_CATALOG ) == 0 )
      {
         return catalogCount ;
      }

      if ( role.compare( OM_NODE_TYPE_DATA ) == 0 )
      {
         return dataCount ;
      }

      return 0 ;
   }

   void omDiskInfo::init()
   {
      diskName        = "" ;
      mountPath       = "" ;
      totalSize       = 0 ;
      freeSize        = 0 ;
      isUsed          = FALSE ;
      standAloneCount = 0 ;
      coordCount      = 0 ;
      catalogCount    = 0 ;
      dataCount       = 0 ;
   }

   INT32 omDiskInfo::getNodeCount()
   {
      return ( standAloneCount + coordCount + catalogCount + dataCount ) ;
   }

   omHostInfo::omHostInfo()
   {
   }

   omHostInfo::~omHostInfo()
   {
   }

   /*
     {
        Config:[
                 {"dbpath":"", svcname:"11810", ...}
                 ...
               ]
     }
   */
   INT32 omHostInfo::_initNodeInfo( const BSONObj &config )
   {
      BSONElement element ;
      NODEINFOLIST_ITER iter ;
      INT32 rc = SDB_OK ;
      if ( config.isEmpty() )
      {
         goto done ;
      }

      element  = config.getField( OM_BSON_FIELD_CONFIG ) ;
      if ( element.eoo() || Array != element.type() )
      {
         rc = SDB_INVALIDARG ;
         PD_LOG( PDERROR, "field is not array type:field=%s,type=%d", 
                 OM_BSON_FIELD_DISK, element.type() ) ;
         goto error ;
      }
      {
         BSONObjIterator i( element.embeddedObject() ) ;
         while ( i.more() )
         {
            string tmpSvcName ;
            omNodeInfo node ;
            BSONElement ele = i.next() ;
            if ( Object != ele.type() )
            {
               rc = SDB_INVALIDARG ;
               PD_LOG( PDERROR, "array's element is invalid:element=%s", 
                       ele.toString().c_str() ) ;
               goto error ;
            }

            BSONObj oneNode = ele.embeddedObject() ;
            node.dbPath     = oneNode.getStringField( OM_CONF_DETAIL_DBPATH ) ;
            node.role       = oneNode.getStringField( OM_CONF_DETAIL_ROLE ) ;
            tmpSvcName      = oneNode.getStringField( OM_CONF_DETAIL_SVCNAME ) ;
            node.svcName    = ossAtoi( tmpSvcName.c_str() ) ;

            node.businessName  = oneNode.getStringField( 
                                                   OM_BSON_BUSINESS_NAME ) ;
            node.dataGroupName = oneNode.getStringField( 
                                                   OM_CONF_DETAIL_EX_DG_NAME ) ;
            _nodeInfoList.push_back( node ) ;
         }
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   void omHostInfo::_increaseNodeCount( string dbpath, string role )
   {
      DISKINFO_ITER iterDisk ;
      iterDisk = _diskList.begin() ;
      while ( iterDisk != _diskList.end() )
      {
         string::size_type pathPos ;
         pathPos = dbpath.find( iterDisk->mountPath ) ;
         if ( pathPos != string::npos )
         {
            iterDisk->isUsed = TRUE ;
            // record the unique disk path
            _usedDiskSet.insert( iterDisk->mountPath ) ;
            break ;
         }

         iterDisk++ ;
      }

      SDB_ASSERT( iterDisk != _diskList.end(), "" ) ;

      // calculate the role count
      if ( role.compare( OM_NODE_TYPE_STANDALONE ) == 0 )
      {
         _nodeCounter.standAloneCount++ ;
         iterDisk->standAloneCount++ ;
      }
      else if ( role.compare( OM_NODE_TYPE_COORD ) == 0 )
      {
         _nodeCounter.coordCount++ ;
         iterDisk->coordCount++ ;
      }
      else if ( role.compare( OM_NODE_TYPE_CATALOG ) == 0 )
      {
         _nodeCounter.catalogCount++ ;
         iterDisk->catalogCount++ ;
      }
      else if ( role.compare( OM_NODE_TYPE_DATA ) == 0 )
      {
         _nodeCounter.dataCount++ ;
         iterDisk->dataCount++ ;
      }
      else
      {
         SDB_ASSERT(FALSE, "node type error") ;
      }
   }

   INT32 omHostInfo::_initCounter()
   {
      CONFIGITEMMAP_ITER iterMap ;
      NODEINFOLIST_ITER iterNodeInfo ;
      string defaultSvcName ;
      INT32 maxSvcName ;
      INT32 maxGroupID = 0;

      iterMap           = _pConfDetailMap->find( OM_CONF_DETAIL_SVCNAME ) ;
      SDB_ASSERT( iterMap != _pConfDetailMap->end(), "" ) ;
      defaultSvcName    = iterMap->second->getDefaultValue() ;
      maxSvcName        = ossAtoi( defaultSvcName.c_str() ) ;

      iterNodeInfo = _nodeInfoList.begin() ;
      while ( iterNodeInfo != _nodeInfoList.end() )
      {
         // get the maxSvcName
         if ( maxSvcName < iterNodeInfo->svcName )
         {
            maxSvcName = iterNodeInfo->svcName ;
         }

         // if business is not the same , do not count the node info
         if ( _businessName.compare( iterNodeInfo->businessName ) != 0 )
         {
            iterNodeInfo++ ;
            continue ;
         }

         // get the max group id
         string::size_type pos = 0 ;
         pos = iterNodeInfo->dataGroupName.find( OM_DG_NAME_PATTERN ) ;
         if ( pos != string::npos )
         {
            string groupID ;
            INT32 id ;
            string::size_type start = pos + ossStrlen( OM_DG_NAME_PATTERN ) ;
            groupID = iterNodeInfo->dataGroupName.substr( start ) ;
            id      = ossAtoi( groupID.c_str() ) ;
            if ( id > maxGroupID )
            {
               maxGroupID = id ;
            }
         }

         _increaseNodeCount( iterNodeInfo->dbPath, iterNodeInfo->role ) ;

         iterNodeInfo++ ;
      }

      _availableSvcName = maxSvcName ;
      if ( _nodeInfoList.size() != 0 )
      {
         _availableSvcName += OM_SVCNAME_STEP ;
      }

      _nodeCounter.unUsedDiskCount = _nodeCounter.diskCount - 
                                                          _usedDiskSet.size() ;
      _availableGroupID = maxGroupID + 1 ; 

      return SDB_OK ;
   }

   INT32 omHostInfo::init( const BSONObj &oneHost, const BSONObj &config,
                           map<string, omConfigItem*> *confDetailMap, 
                           string businessName )
   {
      INT32 rc = SDB_OK ;
      BSONElement hostElement ;
      CONFIGITEMMAP_ITER iter ;
      string minValue ;

      _nodeCounter.diskCount       = 0 ;
      _nodeCounter.unUsedDiskCount = 0 ;
      _nodeCounter.standAloneCount = 0 ;
      _nodeCounter.coordCount      = 0 ;
      _nodeCounter.catalogCount    = 0 ;
      _nodeCounter.dataCount       = 0 ;
      _pConfDetailMap = confDetailMap ;
      _businessName   = businessName ;
      _diskList.clear() ;
      _nodeInfoList.clear() ;

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
            disk.init() ;
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

            _nodeCounter.diskCount++ ;
         }
      }

      rc = _initNodeInfo( config ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "_initNodeInfo failed:rc=%d", rc ) ;
         goto error ;
      }

      rc = _initCounter() ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "_initCounter failed:rc=%d", rc ) ;
         goto error ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   /*
      rule1: the less the better which disk contains role's count
   */
   INT32 omHostInfo::assign( string role, string dataGroupID, 
                             sdbConfDetail &confDetail )
   {
      CHAR dbPath[OM_PATH_LENGTH] ;
      omNodeInfo nodeInfo;
      DISKINFO_ITER bestIter ;
      DISKINFO_ITER iter = _diskList.begin() ;
      bestIter = iter ;
      iter++ ;
      while ( iter != _diskList.end() )
      {
         INT32 roleCount ;
         INT32 bestRoleCount ;
         INT32 nodeCount ;
         INT32 bestNodeCount ;
         roleCount     = iter->getNodeCount( role ) ;
         bestRoleCount = bestIter->getNodeCount( role ) ;
         if ( roleCount != bestRoleCount  )
         {
            if ( roleCount < bestRoleCount )
            {
               bestIter = iter ;
            }

            iter++;
            continue ;
         }

         nodeCount     = iter->getNodeCount() ;
         bestNodeCount = bestIter->getNodeCount() ;
         if ( nodeCount != bestNodeCount  )
         {
            if ( nodeCount < bestNodeCount )
            {
               bestIter = iter ;
            }

            iter++;
            continue ;
         }

         iter++ ;
      }

      confDetail.hostName = _hostName ;
      confDetail.diskName = bestIter->diskName ;
      confDetail.svcName  = _availableSvcName ;
      _availableSvcName   += OM_SVCNAME_STEP ;
      confDetail.role     = role ;
      ossSnprintf( dbPath, OM_PATH_LENGTH, "%s/%s/%d", 
                   bestIter->mountPath.c_str(),
                   confDetail.role.c_str(), confDetail.svcName ) ;
      confDetail.dbPath  = string( dbPath ) ;
      bestIter->isUsed   = TRUE ;
      if ( role.compare( OM_NODE_TYPE_STANDALONE ) == 0 )
      {
         _nodeCounter.standAloneCount++ ;
         bestIter->standAloneCount++ ;
      }
      else if ( role.compare( OM_NODE_TYPE_COORD ) == 0 )
      {
         _nodeCounter.standAloneCount++ ;
         bestIter->coordCount++ ;
      }
      else if ( role.compare( OM_NODE_TYPE_CATALOG ) == 0 )
      {
         _nodeCounter.standAloneCount++ ;
         bestIter->catalogCount++ ;
      }
      else if ( role.compare( OM_NODE_TYPE_DATA ) == 0 )
      {
         confDetail.dataGroupID = dataGroupID ;
         _nodeCounter.standAloneCount++ ;
         bestIter->dataCount++ ;
      }

      nodeInfo.dataGroupName = confDetail.dataGroupID ;
      nodeInfo.role          = confDetail.role ;
      nodeInfo.dbPath        = confDetail.dbPath ;
      nodeInfo.svcName       = confDetail.svcName ;
      _nodeInfoList.push_back( nodeInfo ) ;


      return SDB_OK ;
   }

   INT32 omHostInfo::getAvailableGroupID()
   {
      return _availableGroupID ;
   }

   INT32 omHostInfo::getNodeCount( string role )
   {
      return _nodeCounter.getNodeCount( role ) ;
   }

   INT32 omHostInfo::getNodeCount()
   {
      return _nodeCounter.getNodeCount() ;
   }

   INT32 omHostInfo::getDiskCount()
   {
      return _nodeCounter.getDiskCount() ;
   }

   INT32 omHostInfo::getUnusedDiskCount()
   {
      return _nodeCounter.getUnusedDiskCount() ;
   }

   void omHostInfo::getNodeInfo( hostNodeCounter &nodeCounter )
   {
      nodeCounter = _nodeCounter ;
   }

   string omHostInfo::getHostName()
   {
      return _hostName ;
   }

   BOOLEAN omHostInfo::isDiskExist( string dbPath )
   {
      DISKINFO_ITER iter = _diskList.begin() ;
      while ( iter != _diskList.end() )
      {
         string::size_type pos = dbPath.find( iter->mountPath ) ;
         if ( pos != string::npos )
         {
            return TRUE ;
         }

         iter++ ;
      }

      return FALSE ;
   }

   BOOLEAN omHostInfo::isSvcNameConflict( string svcName )
   {
      INT32 iSvcName = ossAtoi( svcName.c_str() ) ;
      NODEINFOLIST_ITER iter = _nodeInfoList.begin() ;
      while ( iter != _nodeInfoList.end() )
      {
         if ( iter->svcName > iSvcName )
         {
            if ( ( iter->svcName - iSvcName ) < OM_SVCNAME_STEP )
            {
               return TRUE ;
            }
         }
         else
         {
            if ( ( iSvcName - iter->svcName ) < OM_SVCNAME_STEP )
            {
               return TRUE ;
            }
         }

         iter++ ;
      }

      return FALSE ;
   }

   INT32 omHostInfo::addNode( const BSONObj &oneNode )
   {
      omNodeInfo node ;
      node.dbPath        = oneNode.getStringField( OM_CONF_DETAIL_DBPATH ) ;
      SDB_ASSERT( isDiskExist( node.dbPath ), "disk is not exist" ) ;

      node.role          = oneNode.getStringField( OM_CONF_DETAIL_ROLE ) ;

      string tmpSvcName  = oneNode.getStringField( OM_CONF_DETAIL_SVCNAME ) ;
      SDB_ASSERT( !isSvcNameConflict( tmpSvcName ), "svc conflict" ) ;

      node.svcName       = ossAtoi( tmpSvcName.c_str() ) ;
      node.businessName  = oneNode.getStringField( 
                                             OM_BSON_BUSINESS_NAME ) ;
      node.dataGroupName = oneNode.getStringField( 
                                             OM_CONF_DETAIL_EX_DG_NAME ) ;
      _nodeInfoList.push_back( node ) ;

      return SDB_OK ;
   }

   INT32 hostNodeCounter::getNodeCount( string role )
   {
      if ( role.compare( OM_NODE_TYPE_STANDALONE ) == 0 )
      {
         return standAloneCount ;
      }

      if ( role.compare( OM_NODE_TYPE_COORD ) == 0 )
      {
         return coordCount ;
      }

      if ( role.compare( OM_NODE_TYPE_CATALOG ) == 0 )
      {
         return catalogCount ;
      }

      if ( role.compare( OM_NODE_TYPE_DATA ) == 0 )
      {
         return dataCount ;
      }

      return 0 ;
   }

   INT32 hostNodeCounter::getNodeCount()
   {
      return ( standAloneCount + coordCount + catalogCount + dataCount ) ;
   }

   INT32 hostNodeCounter::getDiskCount()
   {
      return diskCount ;
   }

   INT32 hostNodeCounter::getUnusedDiskCount()
   {
      return unUsedDiskCount ;
   }

   omConfigGenerator::omConfigGenerator()
   {
   }

   omConfigGenerator::~omConfigGenerator()
   {
      _clear() ;
   }

   /*
   bsonConfValue:
   {
      "BusinessType":"sequoiadb", "BusinessName":"b1", "ClusterType":"cluster", 
      "ClusterName":"c1", 
      "Config":
      [
         {"HostName": "host1", "datagroupname": "", 
          "dbpath": "/home/db2/standalone/11830", "svcname": "11830", ...}
         ,...
      ]
   }
   bsonAllconf:
   {
      "Property":[{"Name":"dbpath", "Type":"path", "Default":"/opt/sequoiadb", 
                      "Valid":"1", "Display":"edit box", "Edit":"false", 
                      "Desc":"", "WebName":"" }
                      , ...
                 ] 
   }
   bsonHostInfo:
   {
     "HostInfo":[
                   {
                      "HostName":"host1", "ClusterName":"c1", 
                      "Disk":{"Name":"/dev/sdb", Size:"", Mount:"", Used:""},
                      "Config":[{"BusinessName":"b2","dbpath":"", svcname:"", 
                                 "role":"", ... }, ...]
                   }
                    , ... 
                ]
   }
   */
   INT32 omConfigGenerator::checkSDBConfig( const BSONObj &bsonConfValue,
                                            const BSONObj &bsonAllConf, 
                                            const BSONObj &bsonHostInfo )
   {
      INT32 rc = SDB_OK ;
      _clear() ;

      rc = _parseAllConf( bsonAllConf ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "get _parseAllConf failed:rc=%d", rc ) ;
         goto error ;
      }

      _businessName = bsonConfValue.getStringField( OM_BSON_BUSINESS_NAME ) ;
      rc = _parseHostInfo( _businessName, bsonHostInfo ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "get template failed:rc=%d", rc ) ;
         goto error ;
      }

      rc = _checkConfValue( bsonConfValue ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "get _checkConfValue failed:rc=%d", rc ) ;
         goto error ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   omHostInfo *omConfigGenerator::_getHost( string hostName )
   {
      omHostInfo *pHost = NULL ;
      HOSTINFOMAP_ITER iter = _hostInfoMap.find( hostName ) ;
      if ( iter != _hostInfoMap.end() )
      {
         pHost = iter->second ;
      }

      return pHost ;
   }

   INT32 omConfigGenerator::_checkConfValue( const BSONObj &bsonConfValue )
   {
      INT32 rc = SDB_OK ;
      string businessName ;
      string clusterName ;

      businessName = bsonConfValue.getStringField( OM_BSON_BUSINESS_NAME ) ;
      clusterName  = bsonConfValue.getStringField( 
                                                  OM_BSON_FIELD_CLUSTER_NAME ) ;

      BSONObj config = bsonConfValue.getObjectField( OM_BSON_FIELD_CONFIG ) ;
      BSONObjIterator NodeIter( config ) ;
      while ( NodeIter.more() )
      {
         BSONElement ele   = NodeIter.next() ;
         BSONObj oneNode   = ele.embeddedObject() ;
         string hostName   = oneNode.getStringField( OM_BSON_FIELD_HOST_NAME ) ;
         string dbPath     = oneNode.getStringField( OM_CONF_DETAIL_DBPATH ) ;
         string svcName    = oneNode.getStringField( OM_CONF_DETAIL_SVCNAME ) ;
         omHostInfo *pHost = _getHost( hostName ) ;
         if ( NULL == pHost )
         {
            rc = SDB_DMS_RECORD_NOTEXIST ;
            _errorDetail = string( "host is not exist:host=" ) + hostName ;
            PD_LOG( PDERROR, "%s", _errorDetail.c_str() ) ;
            goto error ;
         }

         if ( !pHost->isDiskExist( dbPath ) )
         {
            rc = SDB_DMS_RECORD_NOTEXIST ;
            _errorDetail = string( OM_CONF_DETAIL_DBPATH ) 
                           + "'s disk is not exist:" + dbPath ;
            PD_LOG( PDERROR, "%s", _errorDetail.c_str() ) ;
            goto error ;
         }

         if ( pHost->isSvcNameConflict( svcName ) )
         {
            rc = SDB_INVALIDARG ;
            _errorDetail = string( OM_CONF_DETAIL_SVCNAME ) + 
                           " is conflict:" + svcName ;
            PD_LOG( PDERROR, "%s", _errorDetail.c_str() ) ;
            goto error ;
         }

         BSONObjIterator itemIter( oneNode ) ;
         while ( itemIter.more() )
         {
            BSONElement itemEle = itemIter.next() ;
            string fieldName    = itemEle.fieldName() ;
            string value        = itemEle.String() ;
            if ( fieldName.compare( OM_BSON_FIELD_HOST_NAME ) != 0 )
            {
               omConfigItem *pItem = NULL ;
               CONFIGITEMMAP_ITER iter = _confDetailMap.find( fieldName ) ;
               if ( iter == _confDetailMap.end() )
               {
                  rc = SDB_DMS_RECORD_NOTEXIST ;
                  _errorDetail = string( "can't find the config:name=" ) 
                                 + fieldName ;
                  PD_LOG( PDERROR, "%s", _errorDetail.c_str() ) ;
                  goto error ;
               }

               pItem = iter->second ;
               if ( !pItem->isValid( value ) )
               {
                  rc = SDB_INVALIDARG ;
                  _errorDetail = string( "config value is invalid:name=" )
                                 + fieldName + " value=" + value ;
                  PD_LOG( PDERROR, "%s", _errorDetail.c_str() ) ;
                  goto error ;
               }
            }
         }

         pHost->addNode( oneNode ) ;
      }
   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 omConfigGenerator::_parseAllConf( const BSONObj &bsonAllConf )
   {
      INT32 rc = SDB_OK ;
      BSONObj property ;

      property = bsonAllConf.getObjectField( OM_BSON_PROPERTY_ARRAY ) ;
      BSONObjIterator i( property ) ;
      while ( i.more() )
      {
         BSONElement ele = i.next() ;
         BSONObj oneProperty = ele.embeddedObject() ;
         rc = _setConfDetailValue( oneProperty ) ;
         if ( SDB_OK != rc )
         {
            PD_LOG( PDERROR, "_setConfDetailValue failed:rc=%d", rc ) ;
            goto error ;
         }
      }
   done:
      return rc ;
   error:
      goto done ;
   }

   /*
   bsonTemplate:
   {
      "ClusterName":"c1","BusinessType":"sequoiadb", "BusinessName":"b1",
      "ClusterType": "standalone", 
      "Property":[{"Name":"replicanum", "Type":"int", "Default":"1", 
                      "Valid":"1", "Display":"edit box", "Edit":"false", 
                      "Desc":"", "WebName":"" }
                      , ...
                 ] 
   }
   bsonConfigDetails:
   {
      "Property":[{"Name":"dbpath", "Type":"path", "Default":"/opt/sequoiadb", 
                      "Valid":"1", "Display":"edit box", "Edit":"false", 
                      "Desc":"", "WebName":"" }
                      , ...
                 ] 
   }
   bsonHostInfo:
   { 
     "HostInfo":[
                   {
                      "HostName":"host1", "ClusterName":"c1", 
                      "Disk":{"Name":"/dev/sdb", Size:"", Mount:"", Used:""},
                      "Config":[{"BusinessName":"b2","dbpath":"", svcname:"", 
                                 "role":"", ... }, ...]
                   }
                    , ... 
                ]
   }
   */
   INT32 omConfigGenerator::generateSDBConfig( const BSONObj &bsonTemplate, 
                                               const BSONObj &bsonConfigDetails, 
                                               const BSONObj &bsonHostInfo, 
                                               BSONObj &bsonConfig )
   {
      INT32 rc = SDB_OK ;

      _clear() ;

      _businessName = bsonTemplate.getStringField( OM_BSON_BUSINESS_NAME ) ;
      rc            = _parseTemplate( bsonTemplate ) ;
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

      rc = _parseHostInfo( _businessName, bsonHostInfo ) ;
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
         SDB_OSS_DEL old ;
         _confDetailMap.erase( iter ) ;
      }

      _confDetailMap.insert( CONFIGITEMMAP_TYPE( itemName, pItem ) ) ;
   }

   void omConfigGenerator::_clear() 
   {
      CONFIGITEMMAP_ITER iter     = _confDetailMap.begin() ;
      while ( iter != _confDetailMap.end() )
      {
         omConfigItem *old = iter->second ;
         _confDetailMap.erase( iter++ ) ;

         SDB_OSS_DEL old ;
         old = NULL ;
      }

      HOSTINFOMAP_ITER iterHost  = _hostInfoMap.begin() ;
      while ( iterHost != _hostInfoMap.end() )
      {
         omHostInfo *host = iterHost->second ;
         _hostInfoMap.erase( iterHost++ ) ;

         SDB_OSS_DEL host ;
         host = NULL ;
      }

      _confTemplate.init() ;
      _errorDetail = "" ;
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
                        + item.getItemName() + ",value=" 
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

      return TRUE ;
   }

   INT32 omConfigGenerator::_parseTemplate( const BSONObj &bsonTemplate )
   {
      INT32 rc        = SDB_OK ;
      string tmpValue = "" ;
      BSONObj properyItem ;
      BSONElement propertyElement ;
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

      _confTemplate.dataNum = _confTemplate.dataGroupNum * 
                                                      _confTemplate.replicaNum ;

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

      pItem = SDB_OSS_NEW omConfigItem() ;
      rc    = pItem->init( oneProperty ) ;
      if ( SDB_OK != rc )
      {
         _errorDetail = string( "conf detail bson format error" ) ;
         PD_LOG( PDERROR, "conf detail bson format error:rc=%d", rc ) ;
         goto error ;
      }

      itemName  = pItem->getItemName() ;
      itemValue = pItem->getDefaultValue() ;
      if ( itemName.compare( OM_CONF_DETAIL_DBPATH ) == 0 )
      {
         _confDetailSample.dbPath      = itemValue ;
      }
      else if ( itemName.compare( OM_CONF_DETAIL_SVCNAME ) == 0 )
      {
         _confDetailSample.svcName     = ossAtoi( itemValue.c_str() ) ;
      }
      else if ( itemName.compare( OM_CONF_DETAIL_DIAGLEVEL ) == 0 )
      {
         _confDetailSample.dialevel    = ossAtoi( itemValue.c_str() ) ;
      }
      else if ( itemName.compare( OM_CONF_DETAIL_ROLE ) == 0 )
      {
         _confDetailSample.role        = itemValue ;
      }
      else if ( itemName.compare( OM_CONF_DETAIL_LOGFSIZE ) == 0 )
      {
         _confDetailSample.logFileSize = ossAtoi( itemValue.c_str() ) ;
      }
      else if ( itemName.compare( OM_CONF_DETAIL_LOGFNUM ) == 0 )
      {
         _confDetailSample.logFileNum  = ossAtoi( itemValue.c_str() ) ;
      }
      else if ( itemName.compare( OM_CONF_DETAIL_TRANSACTION ) == 0 )
      {
         _confDetailSample.transaction = ((itemValue.compare("true") == 0) ?
                                              TRUE : FALSE) ;
      }
      else if ( itemName.compare( OM_CONF_DETAIL_PREINSTANCE ) == 0 )
      {
         _confDetailSample.preferedInstance  = itemValue ;
      }
      else if ( itemName.compare( OM_CONF_DETAIL_PCNUM ) == 0 )
      {
         _confDetailSample.numPageCleaner    = ossAtoi( itemValue.c_str() ) ;
      }
      else if ( itemName.compare( OM_CONF_DETAIL_PCINTERVAL ) == 0 )
      {
         _confDetailSample.pageCleanInterval = ossAtoi( itemValue.c_str() ) ;
      }

      _confDetailSample.dataGroupID = "" ;

      _addToItemMap( itemName, pItem ) ;
      pItem = NULL ;

   done:
      return rc ;
   error:
      if ( NULL != pItem )
      {
         SDB_OSS_DEL pItem ;
      }
      _clear() ;
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
      BSONObj property ;

      property = bsonConfigDetails.getObjectField( OM_BSON_PROPERTY_ARRAY ) ;
      {
         BSONObjIterator i( property ) ;
         while ( i.more() )
         {
            BSONElement ele = i.next() ;
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
         PD_LOG( PDERROR, "miss confDetail configur item:%s", 
                 _errorDetail.c_str() ) ;
         goto error ;
      }

   done:
      return rc ;
   error:
      _clear() ;
      goto done ;
   }

   /*
   bsonHostInfo:
   { 
     "HostInfo":[
                   {
                      "HostName":"host1", "ClusterName":"c1", 
                      "Disk":{"Name":"/dev/sdb", Size:"", Mount:"", Used:""},
                      "Config":[{"dbpath":"", svcname:"", "role":"", ... }, ...]
                   }
                    , ... 
                ]
   }
   */
   INT32 omConfigGenerator::_parseHostInfo( string businessName ,
                                            const BSONObj &bsonHostInfo )
   {
      INT32 rc = SDB_OK ;
      BSONObjBuilder builder ;
      BSONObj confFilter ;
      builder.append( OM_BSON_FIELD_CONFIG, "" ) ;
      confFilter = builder.obj() ;

      BSONObj hosts ;
      hosts = bsonHostInfo.getObjectField( OM_BSON_FIELD_HOST_INFO ) ;
      {
         BSONObjIterator iter( hosts ) ;
         while ( iter.more() )
         {
            BSONObj oneHostConf ;
            BSONElement ele ;
            BSONObj oneHost ;
            BSONObj config ;
            omHostInfo *host = NULL  ;

            ele              = iter.next() ;
            oneHostConf      = ele.embeddedObject() ;
            host             = SDB_OSS_NEW omHostInfo() ;
            config  = oneHostConf.filterFieldsUndotted( confFilter, true ) ;
            oneHost = oneHostConf.filterFieldsUndotted( confFilter, false ) ;
            rc = host->init( oneHost, config, &_confDetailMap, businessName ) ;
            if ( SDB_OK != rc )
            {
               _errorDetail = string( "initial host failed" ) ;
               PD_LOG( PDERROR, "initial host failed:rc=%d", rc ) ;
               SDB_OSS_DEL host ;
               goto error ;
            }

            _hostInfoMap.insert( HOSTINFOMAP_TYPE( host->getHostName(), 
                                                   host ) ) ;
          }
       }

   done:
      return rc ;
   error:
      _clear() ;
      goto done ;
   }

   /*
      get best host rule:
          rule1: the less the better which host contains specify role's count
          rule2: the more the better which host contains unused disk's count
          rule3: the less the better which host contains node's count
                 ( all the roles )
   */
   omHostInfo* omConfigGenerator::_getBestHost( string role )
   {
      HOSTINFOMAP_ITER iter ;
      omHostInfo *pHostInfo = NULL ;

      iter = _hostInfoMap.begin() ;
      if ( iter == _hostInfoMap.end() )
      {
         PD_LOG( PDERROR, "hostinfo is null!" ) ;
         goto error ;
      }

      pHostInfo = iter->second ;
      iter++ ;
      while ( iter != _hostInfoMap.end() )
      {
         omHostInfo *pTmp = NULL ;  
         INT32 roleCount ;
         INT32 nodeCount ;
         INT32 unUsedDiskCount ;

         INT32 bestRoleCount ;
         INT32 bestNodeCount ;
         INT32 bestUnUsedDiskCount ;

         pTmp          = iter->second ;
         roleCount     = pTmp->getNodeCount( role ) ;
         bestRoleCount = pHostInfo->getNodeCount( role );
         if ( roleCount != bestRoleCount )
         {
            if ( roleCount < bestRoleCount )
            {
               // rule1
               pHostInfo = pTmp ;
            }

            iter++ ;
            continue ;
         }

         unUsedDiskCount     = pTmp->getUnusedDiskCount() ;
         bestUnUsedDiskCount = pHostInfo->getUnusedDiskCount() ;
         if ( unUsedDiskCount != bestUnUsedDiskCount )
         {
            if ( unUsedDiskCount > bestUnUsedDiskCount )
            {
               pHostInfo = pTmp ;
            }

            iter++ ;
            continue ;
         }

         nodeCount     = pTmp->getNodeCount() ;
         bestNodeCount = pHostInfo->getNodeCount() ;
         if ( nodeCount != bestNodeCount )
         {
            if ( nodeCount < bestNodeCount )
            {
               pHostInfo = pTmp ;
            }

            iter++ ;
            continue ;
         }

         iter++ ;
      }

   done:
      return pHostInfo ;
   error:
      pHostInfo = NULL ;
      goto done ;
   }

   INT32 omConfigGenerator::_generateStandAloneConfig( 
                                               list<sdbConfDetail> &configList )
   {
      INT32 rc = SDB_OK ;
      sdbConfDetail details ;
      omHostInfo *host = _getBestHost( OM_NODE_TYPE_STANDALONE ) ;
      if ( NULL == host )
      {
         rc = SDB_DMS_RECORD_NOTEXIST ;
         PD_LOG( PDERROR, "_getBestHost failed:rc=%d", rc ) ;
         goto error ;
      }

      details = _confDetailSample ;
      host->assign( OM_NODE_TYPE_STANDALONE, "", details ) ;
      configList.push_back( details ) ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 omConfigGenerator::_getAvailableGroupID()
   {
      HOSTINFOMAP_ITER iter ;
      INT32 availableGroupID = 0 ;
      iter = _hostInfoMap.begin() ;
      while ( iter != _hostInfoMap.end() )
      {
         INT32 tmpID ;
         omHostInfo *pHostInfo = iter->second ;
         tmpID = pHostInfo->getAvailableGroupID() ;
         if ( tmpID > availableGroupID )
         {
            availableGroupID = tmpID ;
         }

         iter++ ;
      }

      return availableGroupID ;
   }

   string omConfigGenerator::_calculateGroupID( INT32 baseGroupdID, 
                                                INT32 dataIndex,
                                                INT32 maxDataNumber, 
                                                INT32 maxGroupNumber )
   {
      CHAR groupID[ OM_INT32_LENGTH ] ;
      INT32 step = dataIndex / maxDataNumber ;
      if ( step > maxGroupNumber )
      {
         step = maxGroupNumber ;
      }

      ossSnprintf( groupID, OM_INT32_LENGTH, "%s%d", OM_DG_NAME_PATTERN, 
                   ( baseGroupdID + step ) ) ;
      return string(groupID) ;
   }

   INT32 omConfigGenerator::_generateClusterConfig( 
                                               list<sdbConfDetail> &configList )
   {
      INT32 rc = SDB_OK ;
      sdbConfDetail details ;
      INT32 coordCount   = 0 ;
      INT32 catalogCount = 0 ;
      INT32 dataCount    = 0 ;
      INT32 baseGroupID  = 0 ;

      if ( _confTemplate.coordNum == 0 )
      {
         _confTemplate.coordNum = ( INT32 )_hostInfoMap.size() ;
      }
      while ( coordCount < _confTemplate.coordNum )
      {
         omHostInfo *host = _getBestHost( OM_NODE_TYPE_COORD ) ;
         if ( NULL == host )
         {
            rc = SDB_DMS_RECORD_NOTEXIST ;
            PD_LOG( PDERROR, "_getBestHost failed:rc=%d", rc ) ;
            goto error ;
         }

         details = _confDetailSample ;
         host->assign( OM_NODE_TYPE_COORD, "", details ) ;
         configList.push_back( details ) ;
         coordCount++ ;
      }

      while ( catalogCount < _confTemplate.catalogNum )
      {
         omHostInfo *host = _getBestHost( OM_NODE_TYPE_CATALOG) ;
         if ( NULL == host )
         {
            rc = SDB_DMS_RECORD_NOTEXIST ;
            PD_LOG( PDERROR, "_getBestHost failed:rc=%d", rc ) ;
            goto error ;
         }

         details = _confDetailSample ;
         host->assign( OM_NODE_TYPE_CATALOG, "", details ) ;
         configList.push_back( details ) ;
         catalogCount++ ;
      }

      baseGroupID = _getAvailableGroupID() ;
      while ( dataCount < _confTemplate.dataNum )
      {
         string groupID ;
         omHostInfo *host = _getBestHost( OM_NODE_TYPE_DATA ) ;
         if ( NULL == host )
         {
            rc = SDB_DMS_RECORD_NOTEXIST ;
            PD_LOG( PDERROR, "_getBestHost failed:rc=%d", rc ) ;
            goto error ;
         }

         details = _confDetailSample ;
         groupID = _calculateGroupID( baseGroupID, dataCount, 
                                      _confTemplate.dataNum, 
                                      _confTemplate.dataGroupNum ) ;
         host->assign( OM_NODE_TYPE_DATA, groupID, details ) ;
         configList.push_back( details ) ;
         dataCount++ ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 omConfigGenerator::_generateConfig( BSONObj &bsonConfig )
   {
      INT32 rc = SDB_OK ;
      list<sdbConfDetail> configList ;
      list<sdbConfDetail>::iterator iter ;
      BSONObjBuilder confBuilder ;
      BSONArrayBuilder arrBuilder ;
      if ( ossStrcasecmp( _confTemplate.clusterType.c_str(), 
                          OM_CLUSTER_TYPE_STANDALONE ) == 0 )
      {
         rc = _generateStandAloneConfig( configList ) ;
      }
      else if ( ossStrcasecmp( _confTemplate.clusterType.c_str(), 
                               OM_CLUSTER_TYPE_CLUSTER ) == 0 )
      {
         rc = _generateClusterConfig( configList ) ;
      }
      else
      {
         rc = SDB_INVALIDARG ;
         _errorDetail = string( "unrecognized cluster type:type=%s" )
                        + _confTemplate.clusterType ;
         PD_LOG( PDERROR, "unrecognized cluster type:type=%s", 
                 _confTemplate.clusterType.c_str() ) ;
         goto error ;
      }

      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "generate config failed:rc=%d", rc ) ;
         goto error ;
      }

      iter = configList.begin() ;
      while( iter != configList.end() )
      {
         CHAR tmp[OM_INT32_LENGTH] = "" ;
         BSONObjBuilder builder ;
         builder.append( OM_BSON_FIELD_HOST_NAME, iter->hostName ) ;
         builder.append( OM_CONF_DETAIL_EX_DG_NAME, iter->dataGroupID ) ;
         builder.append( OM_CONF_DETAIL_DBPATH, iter->dbPath ) ;

         ossItoa( iter->svcName, tmp, OM_INT32_LENGTH ) ;
         builder.append( OM_CONF_DETAIL_SVCNAME, tmp ) ;

         ossItoa( iter->dialevel, tmp, OM_INT32_LENGTH ) ;
         builder.append( OM_CONF_DETAIL_DIAGLEVEL, tmp ) ;
         builder.append( OM_CONF_DETAIL_ROLE, iter->role ) ;

         ossItoa( iter->logFileSize, tmp, OM_INT32_LENGTH ) ;
         builder.append( OM_CONF_DETAIL_LOGFSIZE, tmp ) ;

         ossItoa( iter->logFileNum, tmp, OM_INT32_LENGTH ) ;
         builder.append( OM_CONF_DETAIL_LOGFNUM, tmp ) ;

         string transaction = iter->transaction ? "true" : "false" ;
         builder.append( OM_CONF_DETAIL_TRANSACTION, transaction ) ;
         builder.append( OM_CONF_DETAIL_PREINSTANCE, iter->preferedInstance ) ;

         ossItoa( iter->numPageCleaner, tmp, OM_INT32_LENGTH ) ;
         builder.append( OM_CONF_DETAIL_PCNUM, tmp ) ;

         ossItoa( iter->pageCleanInterval, tmp, OM_INT32_LENGTH ) ;
         builder.append( OM_CONF_DETAIL_PCINTERVAL, tmp ) ;

         arrBuilder.append( builder.obj() ) ;
         iter++ ;
      }

      confBuilder.append( OM_BSON_FIELD_CONFIG, arrBuilder.arr() ) ;
      confBuilder.append( OM_BSON_BUSINESS_NAME, _businessName ) ;
      confBuilder.append( OM_BSON_BUSINESS_TYPE, _confTemplate.businessType ) ;
      confBuilder.append( OM_BSON_CLUSTER_TYPE, _confTemplate.clusterType ) ;
      bsonConfig = confBuilder.obj() ;

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





