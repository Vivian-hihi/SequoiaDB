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

   Source File Name = sptDBOID.hpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          19/01/2018  WJM  Initial Draft

   Last Changed =

*******************************************************************************/
#ifndef SPT_DB_OID_HPP
#define SPT_DB_OID_HPP
#include "sptApi.hpp"
#include "client.hpp"
namespace engine
{
   class _sptDBOID : public SDBObject
   {
      JS_DECLARE_CLASS( _sptDBOID )
   public:
      _sptDBOID() ;
      virtual ~_sptDBOID() ;
   public:
      INT32 construct( const _sptArguments &arg,
                       _sptReturnVal &rval,
                       bson::BSONObj &detail ) ;
      INT32 destruct() ;
      static INT32 cvtToBSON( const CHAR* key, const sptObject &value,
                              BOOLEAN isSpecialObj, BSONObjBuilder& builder,
                              string &errMsg ) ;
      static INT32 fmpToBSON( const sptObject &value, BSONObj &retObj,
                              string &errMsg ) ;
      static INT32 bsonToJSObj( sdbclient::sdb &db, const BSONObj &data,
                                _sptReturnVal &rval,
                                bson::BSONObj &detail ) ;
   } ;
   typedef _sptDBOID sptDBOID ;
}
#endif
