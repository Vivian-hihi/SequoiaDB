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

   Source File Name = sptUsrOmaCommon.hpp

   Descriptive Name =

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          03/18/2017  WJM  Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef SPT_USROMA_COMMON_HPP_
#define SPT_USROMA_COMMON_HPP_
#include "ossTypes.hpp"
#include "../bson/bson.hpp"
namespace engine
{
   class _sptUsrOmaCommon: public SDBObject
   {
   public:
      /*
         static functions
      */
      static INT32 getOmaInstallInfo( bson::BSONObj& retObj, string &err ) ;

      static INT32 getOmaInstallFile( string &retStr, string &err ) ;

      static INT32 getOmaConfigFile( string &retStr, string &err ) ;

      static INT32 getOmaConfigs( const bson::BSONObj &arg,
                                  bson::BSONObj &retObj,
                                  string &err ) ;

      static INT32 setOmaConfigs( const bson::BSONObj &arg,
                                  const bson::BSONObj &confObj,
                                  string &err ) ;

      static INT32 getAOmaSvcName( const bson::BSONObj &arg,
                                   string &retStr,
                                   string &err ) ;

      static INT32 addAOmaSvcName( const bson::BSONObj &valueObj,
                                   const bson::BSONObj &optionObj,
                                   const bson::BSONObj &matchObj,
                                   string &err ) ;

      static INT32 delAOmaSvcName( const bson::BSONObj &arg,
                                      string &err ) ;
   private:
      static INT32 _getConfFile( string &confFile ) ;

      static INT32  _getConfInfo( const string &confFile,
                                  bson::BSONObj &conf,
                                  string &err,
                                  BOOLEAN allowNotExist = FALSE ) ;

      static INT32  _confObj2Str( const bson::BSONObj &conf, string &str,
                                  string &err,
                                  const CHAR* pExcept = NULL ) ;
   } ;
}

#endif
