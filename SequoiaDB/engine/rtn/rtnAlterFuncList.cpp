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

   Source File Name = rtnAlterFuncList.cpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          08/05/2015  YW  Initial Draft

   Last Changed =

*******************************************************************************/

#include "rtnAlterFuncList.hpp"
#include "dpsLogWrapper.hpp"
#include "rtnAlterFuncs.hpp"

namespace engine
{
   _rtnAlterFuncList::_rtnAlterFuncList()
   :_inited( FALSE )
   {

   }

   _rtnAlterFuncList::~_rtnAlterFuncList()
   {
      _fl.clear() ;
      _tl.clear() ;
   }

   INT32 _rtnAlterFuncList::init()
   {
      INT32 rc = SDB_OK ;
      BOOLEAN locked = FALSE ;
      if ( !_inited )
      {
         _latch.get() ;
         locked = TRUE ;
         if ( !_inited )
         {
            rc = _init() ;
            if ( SDB_OK != rc )
            {
               goto error ;
            }
            _inited = TRUE ;
         }
      }
   done:
      if ( locked )
      {
         _latch.release() ;
      }
      return rc ;
   error:
      goto done ;
   }

   INT32 _rtnAlterFuncList::getFunc( RTN_ALTER_TYPE type,
                                     const CHAR *name,
                                     RTN_ALTER_FUNC &func )
   {
      INT32 rc = SDB_OK ;
      ALL_FUNCS::const_iterator itrObj ;
      FUNC_LST::const_iterator itrFunc ;

      if ( !_inited )
      {
         rc = init() ;
         if ( SDB_OK != rc )
         {
            PD_LOG( PDERROR, "failed to init rpc func list:%d", rc ) ;
            goto error ;
         }
      }

      itrObj = _fl.find( type ) ;
      if ( _fl.end() == itrObj ) 
      {
         PD_LOG( PDERROR, "can not find type:%d", type ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }

      itrFunc = itrObj->second.fl.find( name ) ;
      if ( itrObj->second.fl.end() == itrFunc )
      {
         PD_LOG( PDERROR, "can not find func[%s] of type[%d]",
                 name, type ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }

      func = itrFunc->second ;
   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _rtnAlterFuncList::getObjType( const CHAR *name,
                                      RTN_ALTER_TYPE &type )
   {
      INT32 rc = SDB_OK ;
      if ( !_inited )
      {
         rc = init() ;
         if ( SDB_OK != rc )
         {
            PD_LOG( PDERROR, "failed to init rpc func list:%d", rc ) ;
            goto error ;
         }
      }

      {
      string str( name ) ;
      boost::algorithm::to_lower( str ) ;
      ALL_TYPES::const_iterator itr = _tl.find( str.c_str() ) ;
      if ( _tl.end() != itr )
      {
         type = itr->second ;
      }
      else
      {
         PD_LOG( PDERROR, "invalid type:%s", name ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }
      }
   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _rtnAlterFuncList::_init()
   {
      INT32 rc = SDB_OK ;

      /// init types
      _tl.insert( std::make_pair( SDB_ALTER_CL, RTN_ALTER_TYPE_CL ) ) ;

      /// collection
      {
         funcObj obj ;
         obj.type = RTN_ALTER_TYPE_CL ;

         /// create id index
         if ( !obj.fl.insert(
                  std::make_pair( SDB_ALTER_CRT_ID_INDEX,
                                  &rtnCreateIDIndex ) ).second )
         {
            PD_LOG( PDERROR, "duplicate func name:%s",
                    SDB_ALTER_CRT_ID_INDEX ) ;
            rc = SDB_SYS ;
            goto error ;
         }

         /// drop id index
         if ( !obj.fl.insert(
                  std::make_pair( SDB_ALTER_DROP_ID_INDEX,
                                  &rtnDropIDIndex ) ).second )
         {
            PD_LOG( PDERROR, "duplicate func name:%s",
                    SDB_ALTER_CRT_ID_INDEX ) ;
            rc = SDB_SYS ;
            goto error ;
         }

         if ( !_fl.insert( std::make_pair( RTN_ALTER_TYPE_CL, obj ) ).second )
         {
            PD_LOG( PDERROR, "duplicate obj type:%d",
                    obj.type ) ;
            rc = SDB_SYS ;
            goto error ;
         }
      }
   done:
      return rc ;
   error:
      goto done ;
   }
}

