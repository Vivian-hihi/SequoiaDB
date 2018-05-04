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

   Source File Name = fmpJSVM.cpp

   Descriptive Name =

   When/how to use:

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          06/19/2013  YW  Initial Draft

   Last Changed =

*******************************************************************************/

#include "fmpJSVM.hpp"
#include "spt.hpp"
#include "ossMem.hpp"
#include "sptContainer.hpp"
#include "sptSPScope.hpp"
#include "sptConvertorHelper.hpp"
#include "sptConvertor.hpp"
#include "sptObjDesc.hpp"
#include "sptSPObject.hpp"
#include "client.hpp"
#include "pd.hpp"

using namespace bson ;
using namespace engine ;

BSONObj GLOBAL_SDB ;

_fmpJSVM::_fmpJSVM()
:_engine( NULL ),
 _scope( NULL ),
 _cursor(NULL)
{
   _engine = SDB_OSS_NEW _sptContainer() ;
   _scope = _engine->newScope() ;
   if ( NULL == _scope )
   {
      PD_LOG( PDERROR, "failed to new scope" ) ;
      return ;
   }

   BSONObjBuilder builder ;
   builder.appendCode( FMP_FUNC_VALUE, "var db=new Sdb();" ) ;
   builder.append( FMP_FUNC_TYPE, FMP_FUNC_TYPE_JS ) ;
   GLOBAL_SDB = builder.obj() ;
   _setOK( TRUE ) ;
}

_fmpJSVM::~_fmpJSVM()
{
   SAFE_OSS_DELETE( _scope ) ;
   SAFE_OSS_DELETE( _engine ) ;
   /// cursor was get from JSObject, do not free it.
   _cursor = NULL ;
}

INT32 _fmpJSVM::initGlobalDB( BSONObj &res )
{
   INT32 rc = SDB_OK ;
   rc = eval( GLOBAL_SDB, res ) ;
   if ( SDB_OK != rc )
   {
      goto error ;
   }
done:
   return rc ;
error:
   goto done ;
}

INT32 _fmpJSVM::eval( const BSONObj &func,
                      BSONObj &res )
{
   INT32 rc = SDB_OK ;
   BSONElement ele = func.getField( FMP_FUNC_VALUE ) ;
   BSONElement type ;
   const _sptResultVal *pRval = NULL ;
   JSContext *pContext = NULL ;
   jsval val = JSVAL_VOID ;

   if ( ele.eoo() || Code != ele.type() )
   {
      PD_LOG( PDERROR, "invalid func type: %d", ele.type() ) ;
      rc = SDB_INVALIDARG ;
      res = BSON( FMP_ERR_MSG << "type of element must be Code" <<
                  FMP_RES_CODE << rc ) ;
      goto error ;
   }

   type = func.getField( FMP_FUNC_TYPE ) ;
   if ( type.eoo() || NumberInt != type.type() ||
        FMP_FUNC_TYPE_JS != type.Int() )
   {
      rc = SDB_INVALIDARG ;
      res = BSON( FMP_ERR_MSG << "type of func must be JS" <<
                  FMP_RES_CODE << rc ) ;
      goto error ;
   }

   rc = _transCode2Str( ele, _cmd ) ;
   if ( SDB_OK != rc )
   {
      PD_LOG( PDERROR, "failed to trans code to str:%d", rc ) ;
      res = BSON( FMP_ERR_MSG << "trans code to str failed" <<
                  FMP_RES_CODE << rc ) ;
      goto error ;
   }

   rc = _scope->eval( _cmd.c_str(), _cmd.length(),
                      NULL, 1, SPT_EVAL_FLAG_NONE,
                      &pRval ) ;
   if ( SDB_OK != rc )
   {
      const CHAR *pLastErr = _scope->getLastErrMsg() ;
      INT32 lastErrno = _scope->getLastError() ;

      if ( !*pLastErr && pRval->hasError() )
      {
         pLastErr = pRval->getErrrInfo() ;
      }
      res = BSON( FMP_ERR_MSG << pLastErr <<
                  FMP_RES_CODE << lastErrno ) ;
      goto error ;
   }
   val = *((jsval*)pRval->rawPtr()) ;
   pContext = ((sptSPScope*)_scope)->getContext() ;

   if ( JSVAL_IS_NULL( val ) )
   {
      res = BSON( FMP_RES_TYPE << FMP_RES_TYPE_VOID ) ;
   }
   else if ( JSVAL_IS_VOID( val ) )
   {
      res = BSON( FMP_RES_TYPE << FMP_RES_TYPE_VOID ) ;
   }
   else if ( JSVAL_IS_INT( val ) )
   {
      INT32 i = 0 ;
      if ( !JS_ValueToInt32( pContext, val, &i ) )
      {
         rc = SDB_SYS ;
         res = BSON( FMP_ERR_MSG << "failed to convert jsval to int32" <<
                     FMP_RES_CODE << rc ) ;
         goto error ;
      }
      res = BSON( FMP_RES_TYPE << FMP_RES_TYPE_NUMBER <<
                  FMP_RES_VALUE << i ) ;
   }
   else if ( JSVAL_IS_DOUBLE(val) )
   {
      jsdouble jsd = 0 ;
      FLOAT64 f = 0 ;
      if ( !JS_ValueToNumber( pContext, val, &jsd ))
      {
         rc = SDB_SYS ;
         res = BSON( FMP_ERR_MSG << "failed to convert jsval to float" <<
                     FMP_RES_CODE << rc ) ;
         goto error ;
      }
      f = jsd ;
      res = BSON( FMP_RES_TYPE << FMP_RES_TYPE_NUMBER <<
                  FMP_RES_VALUE << f ) ;
   }
   else if ( JSVAL_IS_STRING(val))
   {
      std::string s ;
      if ( SDB_OK != JSVal2String( pContext, val, s ) )
      {
         rc = SDB_SYS ;
         res = BSON( FMP_ERR_MSG << "failed to convert jsval to string" <<
                     FMP_RES_CODE << rc ) ;
         goto error ;
      }
      res = BSON( FMP_RES_TYPE << FMP_RES_TYPE_STR <<
                  FMP_RES_VALUE << s ) ;
   }
   else if ( JSVAL_IS_BOOLEAN(val) )
   {
      JSBool bp ;
      BSONObjBuilder builder ;
      if ( !JS_ValueToBoolean( pContext, val, &bp ) )
      {
         rc = SDB_SYS ;
         res = BSON( FMP_ERR_MSG << "failed to convert jsval to boolean" <<
                     FMP_RES_CODE << rc ) ;
         goto error ;
      }

      builder.append( FMP_RES_TYPE, FMP_RES_TYPE_BOOL ) ;
      builder.appendBool( FMP_RES_VALUE, bp ) ;
      res = builder.obj() ;
   }
   else if( JSVAL_IS_OBJECT( val ) )
   {
      JSObject *obj = NULL ;
      const sptObjDesc *desc = NULL ;
      string className ;
      if( !JS_ValueToObject( pContext, val, &obj ) )
      {
         rc = SDB_SYS ;
         res = BSON( FMP_ERR_MSG << "Failed to convert jsval to object" <<
                     FMP_RES_CODE << rc ) ;
         goto error ;
      }
      className = sptGetObjFactory()->getClassName( pContext, obj ) ;
      desc = sptGetObjFactory()->findObj( className ) ;
      if( desc )
      {
         string errMsg ;
         BSONObj val ;
         sptSPObject sptObj( pContext, obj ) ;
         rc = desc->fmpToBSON( sptObj, val, errMsg ) ;
         if( SDB_OK != rc )
         {
            res = BSON( FMP_ERR_MSG << errMsg
                        << FMP_RES_CODE << rc ) ;
            goto error ;
         }

         rc = desc->fmpToCursor( sptObj, &_cursor, errMsg );
         if( SDB_OK != rc )
         {
            res = BSON( FMP_ERR_MSG << errMsg <<
                        FMP_RES_CODE << rc ) ;
            goto error ;
         }
         if( NULL != _cursor )
         {
            res = BSON( FMP_RES_TYPE << FMP_RES_TYPE_RECORDSET ) ;
         }
         else
         {
            res = BSON( FMP_RES_TYPE << FMP_RES_TYPE_SPECIALOBJ <<
                        FMP_RES_VALUE << val <<
                        FMP_RES_CLASSNAME << desc->getJSClassName() ) ;
         }
      }
      else
      {
         BSONObj retObj ;
         sptConvertor convertor( pContext ) ;
         rc = convertor.toBson( obj, retObj ) ;
         if( SDB_OK != rc )
         {
            res = BSON( FMP_ERR_MSG << "Failed to convert js obj to bson" <<
                        FMP_RES_CODE << rc ) ;
            goto error ;
         }
         res = BSON( FMP_RES_TYPE << FMP_RES_TYPE_OBJ <<
                     FMP_RES_VALUE << retObj ) ;

      }
   }
   else
   {
      rc = SDB_INVALIDARG ;
      res = BSON( FMP_ERR_MSG << "unknown result type" <<
                  FMP_RES_CODE << rc ) ;
      goto error ;
   }
done:
   return rc ;
error:
   goto done ;
}

INT32 _fmpJSVM::fetch( BSONObj &res )
{
   INT32 rc = SDB_OK ;
   BSONObj record ;
   rc = cursorNextRecord( _cursor, record ) ;
   if ( SDB_DMS_EOC == rc )
   {
      res = BSON( FMP_RES_CODE << rc ) ;
      goto error ;
   }
   else if ( SDB_OK == rc )
   {
      res = BSON( FMP_RES_TYPE << FMP_RES_TYPE_OBJ <<
                  FMP_RES_VALUE << record ) ;
   }
   else
   {
      res = BSON( FMP_ERR_MSG << "failed to getnext" <<
                  FMP_RES_CODE << rc ) ;
      goto error ;
   }
done:
   return rc ;
error:
   goto done ;
}

INT32 _fmpJSVM::_transCode2Str( const BSONElement &ele,
                                std::string &str )
{
   INT32 rc = SDB_OK ;
   str = ele.code() ;
   return rc ;
}
