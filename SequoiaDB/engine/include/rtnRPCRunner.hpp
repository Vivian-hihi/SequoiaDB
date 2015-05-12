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

   Source File Name = rtnRPCRunner.hpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          08/05/2015  YW  Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef RTN_RPCRUNNER_HPP_
#define RTN_RPCRUNNER_HPP_

#include "rtnRPCDef.hpp"
#include "rtnRPCFuncList.hpp"

namespace engine
{
   class _pmdEDUCB ;

   class _rtnRPCRunner : public SDBObject
   {
   public:
      _rtnRPCRunner() ;
      virtual ~_rtnRPCRunner() ;

   public:
      OSS_INLINE RTN_RPC_TYPE getType() const
      {
         return _type ;
      }

      OSS_INLINE const CHAR *getName() const
      {
         return _name ;
      }
   public:
      INT32 init( const bson::BSONObj &obj ) ;

      void clear() ;

      INT32 run( _pmdEDUCB *cb, _dpsLogWrapper *dpsCB ) ;

   private:
      OSS_INLINE BOOLEAN _inited() const
      {
         return RTN_RPC_INVALID != _type ;
      }

      INT32 _getObjType( const CHAR *str, RTN_RPC_TYPE &type ) ;

      INT32 _run( const bson::BSONObj &rpc,
                  _pmdEDUCB *cb,
                  _dpsLogWrapper *dpsCB ) ;

      INT32 _getFunc( const CHAR *name,
                      RTN_RPC_FUNC &func ) ;
   private:
      _rtnRPCOptions _options ;
      RTN_RPC_TYPE _type ;
      const CHAR *_name ;
      INT32 _v ;
      bson::BSONObj _obj ;
      bson::BSONObj _rpc ;
      bson::BSONObj _args ;
      bson::BSONObj _hint ;
      bson::BSONObjIterator _i ;

      static _rtnRPCFuncList _funcList ;
   } ;
}

#endif

