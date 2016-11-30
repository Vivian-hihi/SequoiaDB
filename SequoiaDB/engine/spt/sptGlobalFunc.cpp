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
#include "ossProc.hpp"
#include "utilStr.hpp"
#include "pdTrace.hpp"

#ifdef SDB_SHELL
   #include "sptParseTroff.hpp"

   #if defined (_WINDOWS)
      #define TF_REL_PATH "..\\doc\\manual\\"
   #else
      #define TF_REL_PATH "../doc/manual/"
   #endif // _WINDOWS
#endif // SDB_SHELL

using namespace bson ;

namespace engine
{
JS_GLOBAL_FUNC_DEFINE_NORESET( _sptGlobalFunc, getLastErrorMsg )
JS_GLOBAL_FUNC_DEFINE_NORESET( _sptGlobalFunc, getLastError )
JS_GLOBAL_FUNC_DEFINE_NORESET( _sptGlobalFunc, getLastErrorObj )
JS_GLOBAL_FUNC_DEFINE_NORESET( _sptGlobalFunc, setLastErrorMsg )
JS_GLOBAL_FUNC_DEFINE_NORESET( _sptGlobalFunc, setLastError )
JS_GLOBAL_FUNC_DEFINE_NORESET( _sptGlobalFunc, setLastErrorObj )
JS_GLOBAL_FUNC_DEFINE_NORESET( _sptGlobalFunc, print )
JS_GLOBAL_FUNC_DEFINE( _sptGlobalFunc, sleep )
JS_GLOBAL_FUNC_DEFINE( _sptGlobalFunc, traceFmt )
JS_GLOBAL_FUNC_DEFINE( _sptGlobalFunc, globalHelp )

JS_BEGIN_MAPPING( _sptGlobalFunc, "" )
   JS_ADD_GLOBAL_FUNC( "getLastErrMsg", getLastErrorMsg )
   JS_ADD_GLOBAL_FUNC( "setLastErrMsg", setLastErrorMsg )
   JS_ADD_GLOBAL_FUNC( "getLastError", getLastError )
   JS_ADD_GLOBAL_FUNC( "setLastError", setLastError )
   JS_ADD_GLOBAL_FUNC( "getLastErrObj", getLastErrorObj )
   JS_ADD_GLOBAL_FUNC( "setLastErrObj", setLastErrorObj )
   JS_ADD_GLOBAL_FUNC( "sleep", sleep )
   JS_ADD_GLOBAL_FUNC( "print", print )
   JS_ADD_GLOBAL_FUNC( "traceFmt", traceFmt )
   JS_ADD_GLOBAL_FUNC( "man", globalHelp )
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

   INT32 _sptGlobalFunc::print( const _sptArguments &arg,
                                _sptReturnVal &rval,
                                BSONObj &detail )
   {
      INT32 rc = SDB_OK ;
      string text ;

      if ( arg.argc() > 0 )
      {
         rc = arg.getString( 0, text, FALSE ) ;
         if ( SDB_OK == rc )
         {
            ossPrintf( "%s", text.c_str() ) ;
         }
         else
         {
            detail = BSON( SPT_ERR <<
                           "Convert the 1st param to string failed" ) ;
         }
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

   INT32 _sptGlobalFunc::traceFmt( const _sptArguments &arg,
                                   _sptReturnVal &rval,
                                   BSONObj &detail )
   {
      INT32 rc = SDB_OK ;
      INT32 formatType = 0 ;
      string input ;
      string output ;

      /// 1st
      rc = arg.getNative( 0, (void*)&formatType, SPT_NATIVE_INT32 ) ;
      if ( rc )
      {
         detail = BSON( SPT_ERR << "The 1st param must be number" ) ;
         goto error ;
      }
      /// check param
      if ( PD_TRACE_FORMAT_TYPE_FLOW != formatType &&
           PD_TRACE_FORMAT_TYPE_FORMAT != formatType )
      {
         rc = SDB_INVALIDARG ;
         detail = BSON( SPT_ERR << "The 1st param value must be 0 or 1" ) ;
         goto error ;
      }

      /// 2nd
      rc = arg.getString( 1, input ) ;
      if ( rc )
      {
         detail = BSON( SPT_ERR << "The 2nd param must be string" ) ;
         goto error ;
      }
      if ( input.empty() )
      {
         rc = SDB_INVALIDARG ;
         detail = BSON( SPT_ERR << "The 2nd param can not be empty" ) ;
         goto error ;
      }

      /// 3rd
      rc = arg.getString( 2, output ) ;
      if ( rc )
      {
         detail = BSON( SPT_ERR << "The 3rd param must be string" ) ;
         goto error ;
      }
      if ( output.empty() )
      {
         rc = SDB_INVALIDARG ;
         detail = BSON( SPT_ERR << "The 3rd param can not be empty" ) ;
         goto error ;
      }

      rc = pdTraceCB::format( input.c_str(), output.c_str(),
                              (_pdTraceFormatType)formatType ) ;
      if ( rc )
      {
         goto error ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _sptGlobalFunc::globalHelp( const _sptArguments &arg,
                                     _sptReturnVal &rval,
                                     BSONObj &detail )
   {
      INT32 rc = SDB_OK ;
      string cata ;
      string cmd ;
      CHAR cmdPath[ OSS_MAX_PATHSIZE + 1 ] = { 0 } ;

      rc = arg.getString( 0, cata ) ;
      if ( rc )
      {
         detail = BSON( SPT_ERR << "The 1st param must be string" ) ;
         goto error ;
      }

      if ( arg.argc() >= 2 )
      {
         rc = arg.getString( 1, cmd ) ;
         if ( rc )
         {
            detail = BSON( SPT_ERR << "The 2nd param should be string" ) ;
            goto error ;
         }
      }
#ifdef SDB_SHELL
      rc = ossGetEWD( cmdPath, OSS_MAX_PATHSIZE ) ;
      if ( rc )
      {
         goto error ;
      }
      rc = utilCatPath( cmdPath, OSS_MAX_PATHSIZE, TF_REL_PATH ) ;
      if ( rc )
      {
         goto error ;
      }
      rc = manHelp::getInstance( cmdPath ).getFileHelp( cata.c_str(),
                                                        cmd.empty() ?
                                                        NULL : cmd.c_str() ) ;
      if ( rc )
      {
         goto error ;
      }
#endif //SDB_SHELL

   done:
      return rc ;
   error:
      goto done ;
   }

}

