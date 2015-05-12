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

   Source File Name = rtnAlterRunner.hpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          08/05/2015  YW  Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef RTN_ALTERRUNNER_HPP_
#define RTN_ALTERRUNNER_HPP_

#include "rtnAlterDef.hpp"
#include "rtnAlterFuncList.hpp"

namespace engine
{
   class _pmdEDUCB ;

   class _rtnAlterRunner : public SDBObject
   {
   public:
      _rtnAlterRunner() ;
      virtual ~_rtnAlterRunner() ;

   public:
      OSS_INLINE RTN_ALTER_TYPE getType() const
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
         return RTN_ALTER_INVALID != _type ;
      }

      INT32 _getObjType( const CHAR *str, RTN_ALTER_TYPE &type ) ;

      INT32 _run( const bson::BSONObj &rpc,
                  _pmdEDUCB *cb,
                  _dpsLogWrapper *dpsCB ) ;

      INT32 _getFunc( const CHAR *name,
                      RTN_ALTER_FUNC &func ) ;
   private:
      _rtnAlterOptions _options ;
      RTN_ALTER_TYPE _type ;
      const CHAR *_name ;
      INT32 _v ;
      bson::BSONObj _obj ;
      bson::BSONObj _rpc ;
      bson::BSONObj _args ;
      bson::BSONObj _hint ;
      bson::BSONObjIterator _i ;

      static _rtnAlterFuncList _funcList ;
   } ;
}

#endif

