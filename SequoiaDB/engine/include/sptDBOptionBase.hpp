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

   Source File Name = sptDBOptionBase.hpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          27/06/2018  ZWB  Initial Draft

   Last Changed =

*******************************************************************************/
#ifndef SPT_DB_OPTIONBASE_HPP
#define SPT_DB_OPTIONBASE_HPP
#include "sptApi.hpp"
   namespace engine
   {
      #define SPT_OPTIONBASE_NAME                "SdbOptionBase"
      #define SPT_OPTIONBASE_COND_FIELD          "_cond"
      #define SPT_OPTIONBASE_SEL_FIELD           "_sel"
      #define SPT_OPTIONBASE_SORT_FIELD          "_sort"
      #define SPT_OPTIONBASE_HINT_FIELD          "_hint"
      #define SPT_OPTIONBASE_SKIP_FIELD          "_skip"
      #define SPT_OPTIONBASE_LIMIT_FIELD         "_limit"

      class _sptDBOptionBase : public SDBObject
      {
         JS_DECLARE_CLASS( _sptDBOptionBase )
      public:
         _sptDBOptionBase() ;
         virtual ~_sptDBOptionBase() ;
      public:
         INT32 construct( const _sptArguments &arg,
                          _sptReturnVal &rval,
                          bson::BSONObj &detail ) ;
         INT32 destruct() ;
         INT32 cond( const _sptArguments &arg,
                     _sptReturnVal &rval,
                     bson::BSONObj &detail ) ;
         INT32 sel( const _sptArguments &arg,
                     _sptReturnVal &rval,
                     bson::BSONObj &detail ) ;
         INT32 sort( const _sptArguments &arg,
                     _sptReturnVal &rval,
                     bson::BSONObj &detail ) ;
         INT32 hint( const _sptArguments &arg,
                     _sptReturnVal &rval,
                     bson::BSONObj &detail ) ;
         INT32 skip( const _sptArguments &arg,
                     _sptReturnVal &rval,
                     bson::BSONObj &detail ) ;
         INT32 limit( const _sptArguments &arg,
                     _sptReturnVal &rval,
                     bson::BSONObj &detail ) ;
         INT32 flags( const _sptArguments &arg,
                     _sptReturnVal &rval,
                     bson::BSONObj &detail ) ;
         static INT32 cvtToBSON( const CHAR* key, const sptObject &value,
                                 BOOLEAN isSpecialObj, BSONObjBuilder& builder,
                                 string &errMsg ) ;
         static INT32 fmpToBSON( const sptObject &value, BSONObj &retObj,
                                 string &errMsg ) ;
         static INT32 bsonToJSObj( sdbclient::sdb &db, const BSONObj &data,
                                   _sptReturnVal &rval, bson::BSONObj &detail ) ;
      } ;
      typedef _sptDBOptionBase sptDBOptionBase ;
   }
#endif

