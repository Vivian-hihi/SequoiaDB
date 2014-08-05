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

   Source File Name = omConfigGenerator.hpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          07/03/2014  LYB Initial Draft

   Last Changed =

*******************************************************************************/
#ifndef OM_CONFIG_GENERATOR_HPP__
#define OM_CONFIG_GENERATOR_HPP__

#include "core.hpp"
#include "oss.hpp"
#include <map>
#include "../bson/bson.h"

using namespace bson;

namespace engine
{
   #define OM_TEMPLATE_REPLICA_NUM     "replicanum"
   #define OM_TEMPLATE_DATAGROUP_NUM   "datagroupnum"
   #define OM_TEMPLATE_CATALOG_NUM     "catalognum"
   #define OM_TEMPLATE_COORD_NUM       "coordnum"

   // extend configure
   #define OM_CONF_DETAIL_EX_DG_NAME   "datagroupname"

   class sdbConfTemplate : public SDBObject
   {
      public:
         string businessType ;
         string deployMod ;
         INT32 replicaNum ;
         INT32 dataNum ;
         INT32 dataGroupNum ;
         INT32 catalogNum ;
         INT32 coordNum ;

      public:
         void init() ;
   } ;

   class sdbConfDetail : public SDBObject
   {
      public:
         string   dbPath ;
         INT32    svcName ;
         INT32    dialevel ;
         string   role ;
         INT32    logFileSize ;
         INT32    logFileNum ;
         BOOLEAN  transaction ;
         string   preferedInstance ;
         INT32    numPageCleaner ;
         INT32    pageCleanInterval ;

         string   dataGroupID ;
         string   hostName ;
         string   diskName ;
         string   user ;         /* root user */ 
         string   passwd ;       /* root passwd */

      public:
         void init() ;
   } ;

   class rangeValidator : public SDBObject
   {
      public:
         rangeValidator( string type, const CHAR *value ) ;
         rangeValidator( string type, const CHAR *begin, const CHAR *end, 
                         BOOLEAN isClosed = TRUE ) ;
         ~rangeValidator() ;

      public:
         BOOLEAN     isValid( const string &value ) ;
         string      getMinValidValue() ;

      private:
         INT32       _compare( string left, string right ) ;
      private:
         string      _type ;
         BOOLEAN     _isClosed ;
         BOOLEAN     _isValidAll ;
         string      _begin ;
         string      _end ;
   } ;

   class omConfigItem : public SDBObject
   {
      public:
         omConfigItem() ;
         ~omConfigItem() ;

      public:
         INT32       init( const BSONObj &bsonItem ) ;
         string      getDefaultValue() ;
         string      getItemName() ;
         BOOLEAN     isValid( const string &value ) ;
         string      getMinValidValue() ;
         string      getValidString() ;

      private:
         void        _addRange( const string &value ) ;

      private:
         string      _name ;
         string      _type ;
         string      _defaultValue ;
         string      _valid ;
         string      _webName ;
         string      _display ;
         string      _edit ;
         string      _desc ;
         string      _level ;
         list<rangeValidator *> _validatorList ;
         typedef list<rangeValidator *>::iterator VALIDATORLIST_ITER ;
   } ;

   struct omNodeInfo : public SDBObject
   {
      string role ;
      string dbPath ;
      string dataGroupName ;
      string businessName ;
      INT32  svcName ;
   } ;

   struct omDiskInfo : public SDBObject
   {
      string  diskName ;
      string  mountPath ;
      UINT64  totalSize ;
      UINT64  freeSize ;
      BOOLEAN isUsed ;
      INT32   standAloneCount ;
      INT32   coordCount ;
      INT32   catalogCount ;
      INT32   dataCount ;
      void    init() ;
      INT32   getNodeCount( string role ) ;
      INT32   getNodeCount() ;
   } ;

   struct hostNodeCounter : public SDBObject
   {
      INT32    unUsedDiskCount ;
      INT32    diskCount ;
      INT32    standAloneCount ;
      INT32    coordCount ;
      INT32    catalogCount ;
      INT32    dataCount ;

      INT32    getNodeCount( string role ) ;
      INT32    getNodeCount() ;
      INT32    getDiskCount() ;
      INT32    getUnusedDiskCount() ;
   } ;
   
   class omHostInfo : public SDBObject
   {
      public:
         omHostInfo() ;
         ~omHostInfo() ;

      public:
         INT32      init( const BSONObj &hostInfo, const BSONObj &config,
                          map<string, omConfigItem*> *confDetailMap,
                          string businessName ) ;
         INT32      assign( string role, string dataGroupID, 
                            sdbConfDetail &confDetail) ;
         INT32      getAvailableGroupID() ;
         INT32      getNodeCount( string role ) ;
         INT32      getNodeCount() ;
         INT32      getDiskCount() ;
         INT32      getUnusedDiskCount() ;
         void       getNodeInfo( hostNodeCounter &nodeCounter ) ;

         string     getHostName() ;

         BOOLEAN    isDiskExist( string dbPath ) ;
         BOOLEAN    isSvcNameConflict( string svcName ) ;
         INT32      addNode( const BSONObj &config ) ;

      private:
         INT32      _initNodeInfo( const BSONObj &config ) ;
         INT32      _initCounter() ;
         void       _increaseNodeCount( string dbpath, string role ) ;

      private:
         hostNodeCounter  _nodeCounter ;
         string           _ip ;
         string           _hostName ;
         INT32            _availableSvcName ;
         INT32            _availableGroupID ;
         string           _businessName ;    
         /* save the businessName to handle this situation: 
               two business are install in the same host.
         */ 

         map<string, omConfigItem*> *_pConfDetailMap ;

         list<omDiskInfo> _diskList ;
         typedef list<omDiskInfo>::iterator DISKINFO_ITER ;

         list<omNodeInfo> _nodeInfoList ;
         typedef list<omNodeInfo>::iterator NODEINFOLIST_ITER ;

         set<string> _usedDiskSet ;
   } ;

   class omConfigGenerator : public SDBObject
   {
      public:
         omConfigGenerator() ;
         virtual ~omConfigGenerator() ;

      public:
         INT32       generateSDBConfig( const BSONObj &bsonTemplate, 
                                        const BSONObj &bsonConfigDetails, 
                                        const BSONObj &bsonHostInfo, 
                                        BSONObj &bsonConfig ) ;

         INT32       checkSDBConfig( const BSONObj &bsonConfValue,
                                     const BSONObj &bsonAllconf, 
                                     const BSONObj &bsonHostInfo ) ;

         string      getErrorDetail() ;

      private:
         INT32       _parseTemplate( const BSONObj &bsonTemplate ) ;
         INT32       _parseHostInfo( string businessName, 
                                     const BSONObj &bsonHostInfo ) ;
         void        _addToItemMap( const string &itemName, 
                                    omConfigItem* pItem ) ;
         INT32       _parseConfigDetail( const BSONObj &bsonConfigDetails ) ;
         INT32       _generateConfig( BSONObj &bsonConfig ) ;
         INT32       _getAvailableGroupID() ;
         string      _calculateGroupID( INT32 baseGroupdID, INT32 dataIndex,
                                        INT32 maxDataNumber, 
                                        INT32 maxGroupNumber ) ;
         INT32       _generateClusterConfig( list<sdbConfDetail> &configList ) ;
         omHostInfo* _getBestHost( string role ) ;
         INT32       _generateStandAloneConfig( 
                                             list<sdbConfDetail> &configList ) ;
         void        _clear() ;
         INT32       _setTemplateValue( const BSONObj &templateItem ) ;
         BOOLEAN     _isAllTemplateSet() ;
         INT32       _setConfDetailValue( const BSONObj &oneProperty ) ;
         BOOLEAN     _isAllConfDetailSet() ;

         omHostInfo *_getHost( string hostName ) ;
         INT32       _checkConfValue( const BSONObj &bsonConfValue ) ;
         INT32       _parseAllConf( const BSONObj &bsonAllConf ) ;


      private:
         sdbConfTemplate               _confTemplate ;
         sdbConfDetail                 _confDetailSample ;
         string                        _errorDetail ;
         string                        _businessName ;
         map<string, omConfigItem*>    _confDetailMap ;
         // typedef map<string, omConfigItem*>::value_type CONFIGITEMMAP_TYPE ;
         // typedef map<string, omConfigItem*>::iterator CONFIGITEMMAP_ITER ;
         map<string, omHostInfo*>      _hostInfoMap ;
         typedef map<string, omHostInfo*>::iterator HOSTINFOMAP_ITER ;
         typedef map<string, omHostInfo*>::value_type HOSTINFOMAP_TYPE ;
   } ;

   typedef map<string, omConfigItem*>::value_type CONFIGITEMMAP_TYPE ;
   typedef map<string, omConfigItem*>::iterator CONFIGITEMMAP_ITER ;
}

#endif  /*OM_CONFIG_GENERATOR_HPP__*/ 

