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

#define RTN_ALTER_REGISTER( type, op, func )\
        do\
        {\
           rc = _registerFunc( rtnAlterFuncKey( (type), (op)), (func) ) ;\
           if ( SDB_OK != rc )\
           {\
              goto error ;\
           }\
        } while ( FALSE )

namespace engine
{
   _rtnAlterFuncList::_rtnAlterFuncList()
   :_inited( FALSE )
   {

   }

   _rtnAlterFuncList::~_rtnAlterFuncList()
   {
      _fl.clear() ;
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
      FUNC_LST::const_iterator itrFunc ;
      rtnAlterFuncKey key( type, name ) ;

      if ( !_inited )
      {
         rc = init() ;
         if ( SDB_OK != rc )
         {
            PD_LOG( PDERROR, "failed to init rpc func list:%d", rc ) ;
            goto error ;
         }
      }

      itrFunc = _fl.find( key ) ;
      if ( _fl.end() == itrFunc ) 
      {
         PD_LOG( PDERROR, "can not find func[%s]",
                 key.toString().c_str() ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }

      func = itrFunc->second ;
   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _rtnAlterFuncList::_init()
   {
      INT32 rc = SDB_OK ;

      /// init funcs
      RTN_ALTER_REGISTER( RTN_ALTER_TYPE_CL,
                          SDB_ALTER_CRT_ID_INDEX,
                          &rtnCreateIDIndex ) ;

      RTN_ALTER_REGISTER( RTN_ALTER_TYPE_CL,
                          SDB_ALTER_DROP_ID_INDEX,
                          &rtnDropIDIndex ) ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _rtnAlterFuncList::_registerFunc( const rtnAlterFuncKey &key,
                                           RTN_ALTER_FUNC func )
   {
      INT32 rc = SDB_OK ;
      if ( !_fl.insert( std::make_pair( key, func ) ).second )
      {
         rc = SDB_INVALIDARG ;
         PD_LOG( PDERROR, "duplicate func:%s",
                 key.toString().c_str() ) ;
         goto error ;
      }
   done:
      return rc ;
   error:
      goto done ;
   }
}

