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
#include "sptHelp.hpp"
#include "pdTraceAnalysis.hpp"

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
JS_GLOBAL_FUNC_DEFINE_NORESET( _sptGlobalFunc, showClass )
JS_GLOBAL_FUNC_DEFINE_NORESET( _sptGlobalFunc, showClassfull)
JS_GLOBAL_FUNC_DEFINE_NORESET( _sptGlobalFunc, forceGC )
JS_GLOBAL_FUNC_DEFINE_NORESET( _sptGlobalFunc, displayManual )
JS_GLOBAL_FUNC_DEFINE_NORESET( _sptGlobalFunc, displayMethod )
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
   JS_ADD_GLOBAL_FUNC( "globalHelp", globalHelp )
   JS_ADD_GLOBAL_FUNC( "displayMethod", displayMethod )
   JS_ADD_GLOBAL_FUNC( "displayManual", displayManual )
   JS_ADD_GLOBAL_FUNC( "showClass", showClass )
   JS_ADD_GLOBAL_FUNC( "showClassfull", showClassfull )
   JS_ADD_GLOBAL_FUNC( "forceGC", forceGC )
JS_MAPPING_END()

   INT32 _sptGlobalFunc::getLastErrorMsg( const _sptArguments &arg,
                                          _sptReturnVal &rval,
                                          bson::BSONObj &detail )
   {
      if ( NULL != sdbGetErrMsg() )
      {
         rval.getReturnVal().setValue( sdbGetErrMsg() ) ;
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
      rval.getReturnVal().setValue( error ) ;
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
            rval.getReturnVal().setValue( obj ) ;
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
      pdTraceParser traceParser ;

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

      rc = traceParser.init( input.c_str(),
                             output.c_str(),
                             (pdTraceFormatType)formatType ) ;
      if ( rc )
      {
         goto error ;
      }

      rc = traceParser.parse() ;
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

      if ( arg.argc() == 0 )
      {
         rc = sptHelp::getInstance().displayGlobalMethod() ;
         if ( rc )
         {
            goto error ;
         }
      }
      else if ( arg.argc() >= 1 )
      {
         string fuzzyFuncName ;
         rc = arg.getString( 0, fuzzyFuncName ) ;
         if ( rc )
         {
            detail = BSON( SPT_ERR << "The 1st param must be a function name" ) ;
            goto error ;
         }
         rc = sptHelp::getInstance().displayManual( fuzzyFuncName, "", FALSE ) ;
         if ( rc )
         {
            goto error ;
         }
      }
      
   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _sptGlobalFunc::displayMethod( const _sptArguments &arg,
                                        _sptReturnVal &rval,
                                        BSONObj &detail )
   {
      INT32 rc = SDB_OK ;

      if ( arg.argc() < 2 )
      {
         rc = SDB_INVALIDARG ;
         goto error ;
      }
      else
      {
         string className ;
         INT32 isInstance = 0 ;
         
         rc = arg.getString( 0, className ) ;
         if ( rc )
         {
            detail = BSON( SPT_ERR << "The 1st param must be the class name" ) ;
            goto error ;
         }
         rc = arg.getNative( 1, (void *)(&isInstance), SPT_NATIVE_INT32 ) ;
         if ( rc )
         {
            detail = BSON( SPT_ERR << "The 2nd param must be a bool value" ) ;
            goto error ;
         }
         rc = sptHelp::getInstance().displayMethod( className, 
                                                    (BOOLEAN)isInstance ) ;
         if ( rc )
         {
            goto error ;
         }         
      }
      
   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _sptGlobalFunc::displayManual( const _sptArguments &arg,
                                        _sptReturnVal &rval,
                                        BSONObj &detail )
   {
      INT32 rc = SDB_OK ;

      if ( arg.argc() < 3 )
      {
         rc = SDB_INVALIDARG ;
         goto error ;
      }
      else
      {
         string fuzzyFuncName ;
         string matcher ;
         INT32 isInstance = 0 ;
         
         rc = arg.getString( 0, fuzzyFuncName ) ;
         if ( rc )
         {
            detail = BSON( SPT_ERR << 
                           "The 1st param must be the name of the function" ) ;
            goto error ;
         }
         rc = arg.getString( 1, matcher ) ;
         if ( rc )
         {
            detail = BSON( SPT_ERR << 
                           "The 2nd param must be the name of the class" ) ;
            goto error ;
         }
         rc = arg.getNative( 2, (void *)(&isInstance), SPT_NATIVE_INT32 ) ;
         if ( rc )
         {
            detail = BSON( SPT_ERR << "The 3rd param must be a bool value" ) ;
            goto error ;
         }
         
         rc = sptHelp::getInstance().displayManual( fuzzyFuncName, 
                                                    matcher, 
                                                    (INT32)isInstance ) ;
         if ( rc )
         {
            goto error ;
         }
      }
      
   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _sptGlobalFunc::showClass( const _sptArguments &arg,
                                    _sptReturnVal &rval,
                                    BSONObj &detail )
   {
      string className ;
      arg.getString( 0, className ) ;
      return _showClassInner( className, FALSE, rval, detail ) ;
   }

   INT32 _sptGlobalFunc::showClassfull( const _sptArguments &arg,
                                        _sptReturnVal &rval,
                                        BSONObj &detail )
   {
      string className ;
      arg.getString( 0, className ) ;
      return _showClassInner( className, TRUE, rval, detail ) ;
   }

   INT32 _sptGlobalFunc::_showClassInner( const string &className,
                                          BOOLEAN showHide,
                                          _sptReturnVal &rval,
                                          BSONObj &detail )
   {
      set<string> names ;
      stringstream ss ;

      if ( !className.empty() )
      {
         /// static functions
         sptGetObjFactory()->getClassStaticFuncNames( (JSContext*)sdbGetThreadContext(),
                                                      className,
                                                      names,
                                                      showHide ) ;
         if ( names.size() > 0 )
         {
            ss << className << "'s static functions:" << endl ;
            set<string>::iterator it = names.begin() ;
            while( it != names.end() )
            {
               ss << "   " << *it << "()" << endl ;
               ++it ;
            }
            names.clear() ;
         }
         /// member functions
         sptGetObjFactory()->getClassFuncNames( (JSContext*)sdbGetThreadContext(),
                                                className,
                                                names,
                                                showHide ) ;
         ss << className << "'s member functions:" << endl ;
      }
      else
      {
         sptGetObjFactory()->getClassNames( names, showHide ) ;
         ss << "All classes:" << endl ;
         set<string>::iterator it = names.begin() ;
         while( it != names.end() )
         {
            ss << "   " << *it << endl ;
            ++it ;
         }
         /// get global function
         names.clear() ;
         sptGetObjFactory()->getClassStaticFuncNames( (JSContext*)sdbGetThreadContext(),
                                                      "",
                                                      names,
                                                      showHide ) ;
         ss << "Global functions:" << endl ;
      }

      set<string>::iterator it = names.begin() ;
      while( it != names.end() )
      {
         ss << "   " << *it << "()" << endl ;
         ++it ;
      }

      rval.getReturnVal().setValue( ss.str() ) ;
      return SDB_OK ;
   }

   INT32 _sptGlobalFunc::forceGC( const _sptArguments &arg,
                                  _sptReturnVal &rval,
                                  BSONObj &detail )
   {
      JS_GC( (JSContext*)sdbGetThreadContext() ) ;
      return SDB_OK ;
   }

}

