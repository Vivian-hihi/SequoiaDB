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

   Source File Name = seAdptOptionsMgr.hpp

   Descriptive Name = Search engine adapter options manager

   When/how to use: this program may be used on binary and text-formatted
   versions of data management component. This file contains structure for
   DMS storage unit and its methods.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          04/08/2017  YSD Initial Draft

   Last Changed =

*******************************************************************************/
#ifndef SEADPT_OPTIONSMGR_HPP__
#define SEADPT_OPTIONSMGR_HPP__

#include "pmdOptionsMgr.hpp"

namespace seadapter
{
   // Manage the configurations of the search engine adapter
   class _seAdptOptionsMgr : public engine::_pmdCfgRecord
   {
   public:
      _seAdptOptionsMgr() ;
      virtual ~_seAdptOptionsMgr() {}

      INT32 init( INT32 argc, CHAR **argv, const CHAR *exePath ) ;

      void setSvcName( const CHAR *svcName ) ;
      const CHAR* getCfgFileName() const { return _cfgFileName ; }
      const CHAR* getSvcName() const { return _serviceName ; }
      const CHAR* getDbHost() const { return _dbHost ; }
      const CHAR* getDbService() const { return _dbService ; }
      const CHAR* getSeHost() const { return _seHost ; }
      const CHAR* getSeService() const { return _seService ; }
      PDLEVEL     getDiagLevel() const ;
      INT32       getTimeout() const ;
      INT32       getBulkBuffSize() const ;

   protected:
      virtual INT32 doDataExchange( engine::pmdCfgExchange *pEX ) ;

   private:
      CHAR     _cfgFileName[ OSS_MAX_PATHSIZE + 1 ] ;
      CHAR     _serviceName[ OSS_MAX_SERVICENAME + 1 ] ;
      CHAR     _dbHost[ OSS_MAX_HOSTNAME + 1 ] ;
      CHAR     _dbService[ OSS_MAX_SERVICENAME + 1 ] ;
      CHAR     _seHost[ OSS_MAX_PATHSIZE + 1 ] ;
      CHAR     _seService[ OSS_MAX_SERVICENAME + 1 ] ;
      UINT16   _diagLevel ;
      INT32    _timeout ;
      INT32    _bulkBuffSize ;
   } ;
   typedef _seAdptOptionsMgr seAdptOptionsMgr ;
}

#endif /* SEADPT_OPTIONSMGR_HPP__ */
