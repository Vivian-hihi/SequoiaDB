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
   class sdbConfTemplate
   {
      public:
         string businessType ;
         string clusterType ;
         INT32 replicaNum ;
         INT32 dataGroupNum ;
         INT32 catalogNum ;
         INT32 coordNum ;
         string userName ;
         string userPasswd ;
         string userGroup ;

      public:
         void init() ;
   } ;

   class sdbConfDetail
   {
      public:
         string dbPath ;
         int svcName ;
         int replName ;
         int shardName ;
         int catalogName ;
         int httpName ;
         int dialevel ;
         string role ;
         int logFileSize ;
         int logFileNum ;
         BOOLEAN transaction ;
         string preferedInstance ;
         int numPageCleaner ;
         int pageCleanInterval ;

      public:
         void init() ;
   } ;

   class rangeValidator
   {
      public:
         rangeValidator( string singleRange ) ;
         rangeValidator( string begin, string end, BOOLEAN isClosed = TRUE ) ;

      public:
         BOOLEAN     isValid( const string &value ) ;
         string      getMinValidValue() ;

      private:
         BOOLEAN     _isClosed ;
         BOOLEAN     _isValidAll ;
         string      _begin ;
         string      _end ;
   } ;

   class omConfigItem
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
         //string      _type ;
         string      _defaultValue ;
         string      _valid ;
         //string      _display ;
         //string      _edit ;
         //string      _desc ;
         //string      _level ;
         list<rangeValidator *> _validatorList ;
         typedef list<rangeValidator *>::iterator VALIDATORLIST_ITER ;
   } ;

   struct omDiskInfo
   {
      string diskName ;
      string mountPath ;
      UINT64 totalSize ;
      UINT64 freeSize ;
      BOOLEAN isUsed ;
   } ;
   
   class omHostInfo
   {
      public:
         omHostInfo() ;
         ~omHostInfo() ;

      public:
         INT32      init( const BSONObj &hostInfo, 
                          map<string, omConfigItem*> *confDetailMap ) ;
         INT32      assign( string role ) ;
         INT32      getNodeCount( string role ) ;
         INT32      getNodeCount() ;
         INT32      getDiskCount() ;
         INT32      getUnusedDiskCount() ;

      private:
         INT32            _unusedDiskCount ;
         INT32            _diskCount ;
         INT32            _standAloneCount ;
         INT32            _coordCount ;
         INT32            _catalogCount ;
         INT32            _dataCount ;
         string           _ip ;
         string           _hostName ;

         INT32            _availableSvcName ;

         map<string, omConfigItem*> *_pConfDetailMap ;

         list<omDiskInfo> _diskList ;
         typedef list<omDiskInfo>::iterator DISKINFO_ITER ;
   } ;

   class omConfigGenerator
   {
      public:
         omConfigGenerator() ;
         virtual ~omConfigGenerator() ;

      public:
         INT32      generateSDBConfig( const BSONObj &bsonTemplate, 
                                       const BSONObj &bsonConfigDetails, 
                                       const BSONObj &bsonHostInfo, 
                                       BSONObj &bsonConfig ) ;

         string     getErrorDetail() ;

      private:
         INT32      _parseTemplate( const BSONObj &bsonTemplate ) ;
         INT32      _parseHostInfo( const BSONObj &bsonHostInfo ) ;
         void       _addToItemMap( const string &itemName, 
                                   omConfigItem* pItem ) ;
         INT32      _parseConfigDetail( const BSONObj &bsonConfigDetails ) ;
         INT32      _generateConfig( const BSONObj &bsonConfig ) ;
         void       _clear() ;
         INT32      _setTemplateValue( const BSONObj &templateItem ) ;
         BOOLEAN    _isAllTemplateSet() ;
         INT32      _setConfDetailValue( const BSONObj &oneProperty ) ;
         BOOLEAN    _isAllConfDetailSet() ;


      private:
         sdbConfTemplate               _confTemplate ;
         string                        _errorDetail ;
         map<string, omConfigItem*>    _confDetailMap ;

   } ;

   typedef map<string, omConfigItem*>::value_type CONFIGITEMMAP_TYPE ;
   typedef map<string, omConfigItem*>::iterator CONFIGITEMMAP_ITER ;
}

#endif  /*OM_CONFIG_GENERATOR_HPP__*/ 

