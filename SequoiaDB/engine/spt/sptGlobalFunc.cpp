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

   Source File Name = sptGlobalFunc.cpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          31/03/2014  YW  Initial Draft

   Last Changed =

*******************************************************************************/

#include "sptGlobalFunc.hpp"
#include "ossUtil.hpp"
#include "sptCommon.hpp"

using namespace bson ;

namespace engine
{
JS_GLOBAL_FUNC_DEFINE_NORESET( _sptGlobalFunc, getLastErrorMsg )
JS_GLOBAL_FUNC_DEFINE_NORESET( _sptGlobalFunc, getLastError )
JS_GLOBAL_FUNC_DEFINE_NORESET( _sptGlobalFunc, getLastErrorObj )
JS_GLOBAL_FUNC_DEFINE_NORESET( _sptGlobalFunc, setLastErrorMsg )
JS_GLOBAL_FUNC_DEFINE_NORESET( _sptGlobalFunc, setLastError )
JS_GLOBAL_FUNC_DEFINE_NORESET( _sptGlobalFunc, setLastErrorObj )
JS_GLOBAL_FUNC_DEFINE( _sptGlobalFunc, sleep )

JS_BEGIN_MAPPING( _sptGlobalFunc, "" )
   JS_ADD_GLOBAL_FUNC( "getLastErrMsg", getLastErrorMsg )
   JS_ADD_GLOBAL_FUNC( "setLastErrMsg", setLastErrorMsg )
   JS_ADD_GLOBAL_FUNC( "getLastError", getLastError )
   JS_ADD_GLOBAL_FUNC( "setLastError", setLastError )
   JS_ADD_GLOBAL_FUNC( "getLastErrObj", getLastErrorObj )
   JS_ADD_GLOBAL_FUNC( "setLastErrObj", setLastErrorObj )
   JS_ADD_GLOBAL_FUNC( "sleep", sleep )
JS_MAPPING_END()

   INT32 _sptGlobalFunc::getLastErrorMsg( const _sptArguments &arg,
                                          _sptReturnVal &rval,
                                          bson::BSONObj &detail )
   {
      if ( NULL != sdbGetErrMsg() )
      {
         rval.setStringVal( "", sdbGetErrMsg() ) ;
      }
      return SDB_OK ;
   }

   INT32 _sptGlobalFunc::setLastErrorMsg( const _sptArguments & arg,
                                          _sptReturnVal & rval,
                                          BSONObj & detail )
   {
      INT32 rc = SDB_OK ;
      string msg ;

      rc = arg.getString( 0, msg ) ;
      if ( SDB_OK == rc )
      {
         sdbSetErrMsg( msg.c_str() ) ;
      }
      else
      {
         detail = BSON( SPT_ERR << "The 1st param must be string" ) ;
      }
      return rc ;
   }

   INT32 _sptGlobalFunc::getLastError( const _sptArguments & arg,
                                       _sptReturnVal & rval,
                                       BSONObj & detail )
   {
      INT32 error = sdbGetErrno() ;
      rval.setNativeVal( "",  NumberInt, (const void*)&error ) ;
      return SDB_OK ;
   }

   INT32 _sptGlobalFunc::setLastError( const _sptArguments & arg,
                                       _sptReturnVal & rval,
                                       BSONObj & detail )
   {
      INT32 rc = SDB_OK ;
      INT32 errNum = SDB_OK ;

      rc = arg.getNative( 0, (void*)&errNum, SPT_NATIVE_INT32 ) ;
      if( SDB_OK == rc )
      {
         sdbSetErrno( errNum ) ;
      }
      else
      {
         detail = BSON( SPT_ERR << "The 1st param must be number" ) ;
      }
      return rc ;
   }

   INT32 _sptGlobalFunc::getLastErrorObj( const _sptArguments &arg,
                                          _sptReturnVal &rval,
                                          BSONObj &detail )
   {
      const CHAR *pObjData = sdbGetErrorObj() ;

      if ( pObjData )
      {
         try
         {
            BSONObj obj( pObjData ) ;
            rval.setBSONObj( "", obj ) ;
         }
         catch( std::exception & )
         {
         }
      }
      return SDB_OK ;
   }

   INT32 _sptGlobalFunc::setLastErrorObj( const _sptArguments &arg,
                                          _sptReturnVal &rval,
                                          BSONObj &detail )
   {
      INT32 rc = SDB_OK ;
      BSONObj obj ;

      rc = arg.getBsonobj( 0, obj ) ;
      if ( SDB_OK == rc )
      {
         sdbSetErrorObj( obj.objdata(), obj.objsize() ) ;
      }
      else
      {
         detail = BSON( SPT_ERR << "The 1st param must be object" ) ;
      }
      return rc ;
   }

   INT32 _sptGlobalFunc::sleep( const _sptArguments &arg,
                                _sptReturnVal &rval,
                                bson::BSONObj &detail )
   {
      INT32 rc = SDB_OK ;
      UINT32 time = 0 ;
      rc = arg.getNative( 0, &time, SPT_NATIVE_INT32 ) ;
      if ( SDB_OK != rc )
      {
         detail = BSON( SPT_ERR << "The 1st param must be number" ) ;
         goto error ;
      }

      ossSleepmillis( time ) ;
   done:
      return rc ;
   error:
      goto done ;
   }
}

