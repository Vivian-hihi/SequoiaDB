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

*******************************************************************************/

#include "spt.hpp"
#include "ossTypes.h"
#include "jsapi.h"
#include "ossUtil.h"
#include "pd.hpp"
#include "ossMem.hpp"
#include "pdTrace.hpp"
#include "sptTrace.hpp"

#define VERIFY(cond) if ( ! (cond) ) goto error

#define SAFE_JS_FREE( cx, p ) \
   do { if ( p ) { JS_free( ( cx ), ( p ) ) ; ( p ) = NULL ; } } while ( 0 )

#include "js_in_cpp.hpp"

// In shell env, we use two global environments for
// gShellReturnCode: the return code that we supposed to return to shell
// gReadNothing: whether the last cursor returned anything before hitting EOC
//
// Those variables should only be used by SHELL since it's single threaded
// Therefore we don't bother to inject them into evaluate() return code, since
// nothing will be intereted in this EXCEPT shell
// So we use this ugly hack to setup a global variable whenever evaluate() is
// called.
//
// Those variables should be examed by two scenarios
// 1) interactive mode and batch mode. In this case, gShellReturnCode should be
// checked BEFORE the final return to shell
// 2) front-end and backend mode. In this case, gShellREturnCode should be
// examed by sdbbp, and inject the return code into the response message so that
// front-end process knows which rc to return
#if defined (SDB_SHELL)
INT32 gShellReturnCode ;
BOOLEAN gReadNothing ;
#endif

JSBool InitDbClasses( JSContext *cx, JSObject *obj ) ;

namespace engine {

static ScriptEngine * globalEngine ;

PD_TRACE_DECLARE_FUNCTION ( SDB_SE_GLBSE, "ScriptEngine::globalScriptEngine" )
ScriptEngine * ScriptEngine::globalScriptEngine()
{
   PD_TRACE_ENTRY ( SDB_SE_GLBSE );
   if ( ! globalEngine )
   {
      globalEngine = SDB_OSS_NEW ScriptEngine ;
      VERIFY( globalEngine );

      VERIFY( globalEngine->init() );
   }
done :
   PD_TRACE_EXIT ( SDB_SE_GLBSE );
   return globalEngine ;
error :
   SAFE_OSS_DELETE( globalEngine );
   globalEngine = NULL ;
   goto done ;
}

void ScriptEngine::purgeGlobalScriptEngine()
{
   SAFE_OSS_DELETE( globalEngine );
   globalEngine = NULL ;
}

ScriptEngine::ScriptEngine() :
   _runtime( NULL )
{
#if defined (SDB_SHELL)
   gShellReturnCode = SDB_OK ;
   gReadNothing     = FALSE ;
#endif
}

ScriptEngine::~ScriptEngine()
{
   if ( _runtime ) JS_DestroyRuntime( _runtime );
   JS_ShutDown();
}

BOOLEAN ScriptEngine::init()
{
   _runtime = JS_NewRuntime( 8 * 1024 * 1024 );
   return _runtime ? TRUE : FALSE ;
}

PD_TRACE_DECLARE_FUNCTION ( SDB_SE_NEWSCOPE, "ScriptEngine::newScope" )
Scope *ScriptEngine::newScope()
{
   PD_TRACE_ENTRY ( SDB_SE_NEWSCOPE );
   Scope *scope = SDB_OSS_NEW Scope ;
   VERIFY( scope );

   VERIFY( scope->init() );

done :
   PD_TRACE_EXIT ( SDB_SE_NEWSCOPE );
   return scope ;
error :
   SAFE_OSS_DELETE( scope );
   scope = NULL ;
   goto done ;
}

// Scope

static JSClass global_class = {
   "Global",                     // class name
   JSCLASS_GLOBAL_FLAGS,         // flags
   JS_PropertyStub,              // addProperty
   JS_PropertyStub,              // delProperty
   JS_PropertyStub,              // getProperty
   JS_StrictPropertyStub,        // setProperty
   JS_EnumerateStub,             // enumerate
   JS_ResolveStub,               // resolve
   JS_ConvertStub,               // convert
   JS_FinalizeStub,              // finalize
   JSCLASS_NO_OPTIONAL_MEMBERS   // optional members
};

static void reportError(JSContext *cx, const char *msg, JSErrorReport *report)
{
   ossPrintf( "%s:%d %s\n" ,
              report->filename ? report->filename : "(nofile)" ,
              report->lineno ,
              msg ) ;
}

Scope::Scope() :
   _context( NULL ),
   _global( NULL )
{
}

Scope::~Scope()
{
   if ( _context ) JS_DestroyContext( _context );
   _context = NULL ;
   _global = NULL ;
}

PD_TRACE_DECLARE_FUNCTION ( SDB_SCOPE_INIT, "Scope::init" )
BOOLEAN Scope::init()
{
   BOOLEAN ret = FALSE ;
   PD_TRACE_ENTRY ( SDB_SCOPE_INIT );

   SDB_ASSERT( globalEngine, "Script engine has not been initialized" );
   SDB_ASSERT( ! _context && ! _global, "Can't init a scope twice" );

   _context = JS_NewContext( globalEngine->_runtime, 8 * 1024 );
   VERIFY( _context );

   JS_SetOptions( _context, JSOPTION_VAROBJFIX );
   JS_SetVersion( _context, JSVERSION_LATEST );
   JS_SetErrorReporter( _context, reportError );

   _global = JS_NewCompartmentAndGlobalObject( _context, &global_class, NULL );
   VERIFY( _global );

   VERIFY( JS_InitStandardClasses( _context, _global ) );

   VERIFY( InitDbClasses( _context, _global ) ) ;

   VERIFY ( SDB_OK == evalInitScripts ( this ) ) ;

   ret = TRUE ;

done :
   PD_TRACE_EXIT ( SDB_SCOPE_INIT );
   return ret ;
error :
   goto done ;
}

// caller should free the return pointer using SAFE_JS_FREE
PD_TRACE_DECLARE_FUNCTION ( SDB_CONVJS2STR, "convertJsvalToString" )
static CHAR *convertJsvalToString ( JSContext *cx , jsval val )
{
   PD_TRACE_ENTRY ( SDB_CONVJS2STR );
   JSString *  str   = NULL ;
   CHAR *      cstr  = NULL ;

   str = JS_ValueToString ( cx , val ) ;
   if ( ! str )
      goto error ;

   // cstr is freed by caller
   cstr = JS_EncodeString ( cx , str ) ;
   if ( ! cstr )
      goto error ;

done :
   PD_TRACE_EXIT ( SDB_CONVJS2STR );
   return cstr ;
error :
   goto done ;
}

PD_TRACE_DECLARE_FUNCTION ( SDB_SCOPE_EVALUATE, "Scope::evaluate" )
INT32 Scope::evaluate ( const CHAR *code , UINT32 len , const CHAR *filename ,
                        UINT32 lineno , CHAR ** result )
{
   PD_TRACE_ENTRY ( SDB_SCOPE_EVALUATE );
   jsval        rval           = JSVAL_VOID ;
   jsval        exception      = JSVAL_VOID ;
   CHAR *       cstrException  = NULL ;
   CHAR *       cstr           = NULL ;
   INT32        rc             = SDB_OK ;

   SDB_ASSERT ( _context && _global, "this scope has not been initilized" ) ;
   SDB_ASSERT ( code , "Invalid arguments" ) ;

   if ( ! JS_EvaluateScript ( _context , _global ,
                              code , len > 0 ? len : ossStrlen ( code ) ,
                              filename ? filename : "(default)" , lineno ,
                              &rval ) )
   {
      rc = SDB_SPT_EVAL_FAIL ;
      goto error ;
   }

#if defined (SDB_SHELL)
   // if context does not show exception pending but we have gShellReturnCode
   // set, that means the error is caught by try/catch, so we should reset
   // gShellReturnCode to SDB_OK
   if ( gShellReturnCode && ! JS_IsExceptionPending( _context ) )
   {
      gShellReturnCode = SDB_OK ;
   }
#endif

   if ( ! result )
      goto done ;

   // cstr is freed in done:
   // cstr is freed by JSFree, so we have to use strdup instead of ossStrdup
   if ( JSVAL_IS_VOID ( rval ) )
#if defined (_LINUX)
      cstr = strdup ( "" ) ;
#elif defined (_WINDOWS)
      cstr = _strdup ( "" ) ;
#endif
   else
      cstr = convertJsvalToString ( _context , rval ) ;

   if ( ! cstr )
   {
      rc = SDB_SYS ;
      goto error ;
   }

   *result = ossStrdup ( cstr ) ;
   if ( !( result && result[0] != '\0') )
   {
      rc = SDB_OOM ;
      PD_LOG ( PDERROR , "memory allcation fail" ) ;
      goto error ;
   }

done :
   SAFE_JS_FREE ( _context , cstr ) ;
   PD_TRACE_EXITRC ( SDB_SCOPE_EVALUATE, rc );
#if defined (SDB_SHELL)
   // we can rely on gShellReturnCode because SHELL is a single-threaded process
   // only
   // gShellReturnCode is NOT VALID for any multi-threaded environment
   //
   // gShellReturnCode was set to SDB_RC from dbClasses.
   // Here we are going to map it into shell return code
   // The rule is
   // SDB_SYS : SDB_RETURNCODE_SYSTEM
   // SDB_OK  : SDB_RETURNCODE_OK
   // SDB_DMS_EOC && gReadNothing : SDB_RETURNCODE_EMPTY
   // SDB_DMS_EOC && !gReadNothing : SDB_OK
   // Others  : SDB_RETURNCODE_ERROR
   switch ( gShellReturnCode )
   {
   case SDB_SYS :
      gShellReturnCode = SDB_RETURNCODE_SYSTEM ;
      break ;
   case SDB_OK :
      gShellReturnCode = SDB_RETURNCODE_OK ;
      break ;
   case SDB_DMS_EOC :
      gShellReturnCode = (gReadNothing)?SDB_RETURNCODE_EMPTY:SDB_OK ;
      gReadNothing = FALSE ;
      break ;
   default :
      gShellReturnCode = SDB_RETURNCODE_ERROR ;
   }
#endif
   return rc ;
error :
   if ( JS_GetPendingException ( _context , &exception ) )
   {
      cstrException = convertJsvalToString ( _context , exception ) ;
      if ( cstrException )
      {
         ossPrintf ( "Uncaught exception: %s\n" , cstrException ) ;
         SAFE_JS_FREE ( _context , cstrException ) ;
      }
      else
      {
         JS_ClearPendingException ( _context ) ;
      }
   }
   goto done ;
}

PD_TRACE_DECLARE_FUNCTION ( SDB_SCOPE_EVALUATE2, "Scope::evaluate2" )
INT32 Scope::evaluate2 ( const CHAR *code, UINT32 len, UINT32 lineno,
                        jsval *rval, CHAR **errMsg )
{
   PD_TRACE_ENTRY ( SDB_SCOPE_EVALUATE2 );
   INT32 rc = SDB_OK ;
   jsval exception = JSVAL_VOID ;
   CHAR *cstrException = NULL ;
   SDB_ASSERT ( _context && _global, "this scope has not been initilized" ) ;
   SDB_ASSERT ( code , "Invalid arguments" ) ;

   if ( ! JS_EvaluateScript ( _context, _global,
                              code, len, NULL,
                              lineno, rval ) )
   {
      rc = SDB_SPT_EVAL_FAIL ;
      goto error ;
   }

done:
   PD_TRACE_EXITRC ( SDB_SCOPE_EVALUATE2, rc );
   return rc ;
error:
   if ( JS_IsExceptionPending( _context ) &&
        JS_GetPendingException ( _context , &exception ) )
   {
      cstrException = convertJsvalToString ( _context , exception ) ;
      if ( cstrException )
      {
         ossPrintf ( "Uncaught exception: %s\n" , cstrException ) ;

         /// what to do when oom?
         *errMsg = ossStrdup( cstrException ) ;
         SAFE_JS_FREE ( _context , cstrException ) ;
      }
      else
      {
         JS_ClearPendingException ( _context ) ;
      }
   }
   goto done ;
}

} // namespace engine

