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

   Source File Name = sptSPScope.cpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          31/03/2014  YW  Initial Draft

   Last Changed =

*******************************************************************************/

#include "sptSPScope.hpp"
#include "sptObjDesc.hpp"
#include "pd.hpp"
#include "ossUtil.hpp"
#include "sptSPDef.hpp"
#include "sptBsonobj.hpp"
#include "sptGlobalFunc.hpp"

const UINT32 RUNTIME_SIZE = 8 * 1024 * 1024 ;

const UINT32 FUNC_ARRAY_SIZE = 50 ;

namespace engine
{
   _sptSPScope::_sptSPScope()
   :_runtime( NULL ),
    _context( NULL )
   {

   }

   _sptSPScope::~_sptSPScope()
   {
      shutdown() ;
   }

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


   INT32 _sptSPScope::start()
   {
      INT32 rc = SDB_OK ;
      if ( NULL != _runtime )
      {
         PD_LOG( PDERROR, "scope has already been started up" ) ;
         rc = SDB_SYS ;
         goto error ;
      }

      _runtime = JS_NewRuntime( RUNTIME_SIZE );
      if ( NULL == _runtime )
      {
         PD_LOG( PDERROR, "failed to init js runtime" ) ;
         rc = SDB_SYS ;
         goto error ;
      }

      _context = JS_NewContext( _runtime, RUNTIME_SIZE / 8 );
      if ( NULL == _context )
      {
         PD_LOG( PDERROR, "failed to init js context" ) ;
         rc = SDB_SYS ;
         goto error ;
      }

      JS_SetOptions( _context, JSOPTION_VAROBJFIX );
      JS_SetVersion( _context, JSVERSION_LATEST );
      JS_SetErrorReporter( _context, reportError );

      _global = JS_NewCompartmentAndGlobalObject( _context, &global_class, NULL );
      if ( NULL == _global )
      {
         PD_LOG( PDERROR, "failed to init js global object" ) ;
         rc = SDB_SYS ;
         goto error ;
      }

      if ( !JS_InitStandardClasses( _context, _global ) )
      {
         PD_LOG( PDERROR, "failed to init standard class" ) ;
         rc = SDB_SYS ;
         goto error ;
      }

      rc = loadUsrDefObj( &(_sptBsonobj::__desc) ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to load bsonobj:%d", rc ) ;
         goto error ;
      }

      rc = loadUsrDefObj( &(_sptGlobalFunc::__desc) ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to load bsonobj:%d", rc ) ;
         goto error ;
      }
   done:
      return rc ;
   error:
      shutdown() ;
      goto done ;
   }

   void _sptSPScope::shutdown()
   {
      
      if ( NULL != _context )
      {
         void *p = JS_GetContextPrivate( _context ) ;
         if ( NULL != p )
         {
            SDB_OSS_FREE( p ) ;
         }

         JS_SetContextPrivate( _context, NULL ) ;

         JS_EndRequest(_context) ;
         JS_DestroyContext( _context ) ;
         _context = NULL ;
      }

      if ( NULL != _runtime )
      {
         JS_DestroyRuntime( _runtime ) ;
         _runtime = NULL ;
         JS_ShutDown() ;
      }

      _global = NULL ;

   }

   INT32 _sptSPScope::_loadUsrDefObj( _sptObjDesc *desc )
   {
      INT32 rc = SDB_OK ;
      if ( !desc->getIgnore() )
      {
         rc = _loadUsrClass( desc ) ;
         if ( SDB_OK != rc )
         {
            goto error ;
         }
      }

      rc = _loadGlobal( desc ) ;
      if ( SDB_OK != rc )
      {
         goto error ;
      }
   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _sptSPScope::_loadGlobal( _sptObjDesc *desc )
   {
      INT32 rc = SDB_OK ;
      const _sptFuncMap &fMap = desc->getFuncMap() ;
      const sptFuncMap::NORMAL_FUNCS &funcs =
                                     fMap.getGlobalFuncs() ;
      JSFunctionSpec *specs = new JSFunctionSpec[funcs.size() + 1] ;
      if ( NULL == specs )
      {
         PD_LOG( PDERROR, "failed to allocate mem." ) ;
         rc = SDB_OOM ;
         goto error ;
      }

      {
      UINT32 i = 0 ;
      sptFuncMap::NORMAL_FUNCS::const_iterator itr = funcs.begin() ;
      for ( ; i < funcs.size() ; i++, itr++ )
      {
         specs[i].name = itr->first.c_str() ;
         specs[i].call = itr->second ;
         specs[i].nargs = 0 ;
         specs[i].flags = 0 ;
      }
      specs[i].name = NULL ;
      specs[i].call = NULL ;
      specs[i].nargs = 0 ;
      specs[i].flags = 0 ;

      if ( !JS_DefineFunctions( _context, _global, specs ) )
      {
         PD_LOG( PDERROR, "failed to define global functions" ) ;
         rc = SDB_SYS ;
         goto error ;
      }
      }
   done:
      delete []specs ;
      return rc ;
   error:
      goto done ;
   }

   INT32 _sptSPScope::_loadUsrClass( _sptObjDesc *desc )
   {
      INT32 rc = SDB_OK ;
      const CHAR *objName = desc->getJSClassName() ;
      const _sptFuncMap &fMap = desc->getFuncMap() ;
      JS_INVOKER::MEMBER_FUNC construct = fMap.getConstructor() ;
      JS_INVOKER::DESTRUCT_FUNC destruct = fMap.getDestructor() ;
      JS_INVOKER::RESLOVE_FUNC resolve = fMap.getResolver() ;
      
      uint32 flags = NULL == resolve ?
                     JSCLASS_HAS_PRIVATE :
                     JSCLASS_HAS_PRIVATE | JSCLASS_NEW_RESOLVE ;

      JSResolveOp resolveOp = NULL == resolve ?
                              JS_ResolveStub : (JSResolveOp)resolve ;

      SDB_ASSERT( NULL != destruct, "destructor can not be NULL" )

      JSClass cDef = { ( CHAR * )objName,
                    flags,
                    JS_PropertyStub, 
                    JS_PropertyStub,
                    JS_PropertyStub,
                    JS_StrictPropertyStub,
                    JS_EnumerateStub,       
                    resolveOp,
                    JS_ConvertStub,
                    destruct,
                    JSCLASS_NO_OPTIONAL_MEMBERS } ;

      desc->setClassDef( cDef ) ;

      const sptFuncMap::NORMAL_FUNCS &memberFuncs = fMap.getMemberFuncs() ;
      const sptFuncMap::NORMAL_FUNCS &staticFuncs = fMap.getStaticFuncs() ;

      JSFunctionSpec *fSpecs = NULL ;
      JSFunctionSpec *sfSpecs = NULL ;
 
      /// +1 for FS_END
      fSpecs = new JSFunctionSpec[memberFuncs.size() + 1] ;
      if ( NULL == fSpecs )
      {
         PD_LOG( PDERROR, "failed to allocate mem." ) ;
         rc = SDB_OOM ;
         goto error ;
      }

      sfSpecs = new JSFunctionSpec[staticFuncs.size() + 1] ;
      if ( NULL == sfSpecs )
      {
         PD_LOG( PDERROR, "failed to allocate mem." ) ;
         rc = SDB_OOM ;
         goto error ;
      }

      {
      UINT32 i = 0 ;
      sptFuncMap::NORMAL_FUNCS::const_iterator itr = memberFuncs.begin() ;
      for ( ; i < memberFuncs.size() ; i++, itr++ )
      {
         fSpecs[i].name = itr->first.c_str() ;
         fSpecs[i].call = itr->second ;
         fSpecs[i].nargs = 0 ;
         fSpecs[i].flags = 0 ;
      }
      fSpecs[i].name = NULL ;
      fSpecs[i].call = NULL ;
      fSpecs[i].nargs = 0 ;
      fSpecs[i].flags = 0 ;

      i = 0 ;
      itr = staticFuncs.begin() ;
      for ( ; i < staticFuncs.size() ; i++, itr++ )
      {
         sfSpecs[i].name = itr->first.c_str() ;
         sfSpecs[i].call = itr->second ;
         sfSpecs[i].nargs = 0 ;
         sfSpecs[i].flags = 0 ;
      }
      sfSpecs[i].name = NULL ;
      sfSpecs[i].call = NULL ;
      sfSpecs[i].nargs = 0 ;
      sfSpecs[i].flags = 0 ;

      if ( !JS_InitClass( _context, _global, 0, (JSClass *)desc->getClassDef(),
                          construct, 0, 0, fSpecs,
                          0, sfSpecs ) )
      {
         PD_LOG( PDERROR, "failed to call js_initclass" ) ;
         rc = SDB_SYS ;
         goto error ;
      }
      }
   done:
      delete []fSpecs ;
      delete []sfSpecs ;
      return rc ;
   error:
      goto done ;
   }

   INT32 _sptSPScope::eval( const CHAR *code, UINT32 len,
                            const CHAR *filename,
                            UINT32 lineno,
                            bson::BSONObj &detail )
   {
      INT32 rc = SDB_OK ;
      SDB_ASSERT ( _context && _global, "this scope has not been initilized" )
      SDB_ASSERT( NULL != code || 0 < len, "code can not be empty" )
      jsval *rval = NULL ;
      jsval exception = JSVAL_VOID ;
      if ( !JS_EvaluateScript( _context, _global, code,
                               len, filename, lineno, rval ) )
      {
         rc = SDB_SPT_EVAL_FAIL ;
         PD_LOG( PDERROR, "failed to eval js code" ) ;
         goto error ;
      }
   done:
      return rc ;
   error:
      if ( JS_IsExceptionPending( _context ) &&
           JS_GetPendingException ( _context , &exception ) )
      {
         bson::BSONObjBuilder builder ;
         CHAR *strException = NULL ;
         JSString *jsstr = JS_ValueToString( _context, exception ) ;
         if ( NULL != jsstr )
         {
            strException = JS_EncodeString ( _context, jsstr ) ;
         }

         if ( NULL != strException )
         {
            ossPrintf ( "Uncaught exception: %s\n" , strException ) ;
            detail = BSON( "exception" << strException ) ;
            SAFE_JS_FREE( _context, strException ) ;
         }

         JS_ClearPendingException ( _context ) ;
      }
      goto done ;
   }

}

