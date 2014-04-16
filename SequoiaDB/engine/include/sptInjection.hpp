/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = sptInjection.hpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          31/03/2014  YW  Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef SPT_INJECTION_HPP_
#define SPT_INJECTION_HPP_

namespace engine
{
   #define JS_MEMBER_FUNC_DEFINE( className, funcName )\
           static JSBool __##funcName( JSContext *cx , uintN argc , jsval *vp )\
           {\
              JSBool ret = JS_TRUE ; \
              INT32 rc = SDB_OK ; \
              typedef INT32 (className::*FUNC)(const _sptParamContainer &,\
                                              _sptReturnVal &,\
                                               bson::BSONObj &);\
              rc = sptInvoker::callMemberFunc<className, FUNC>\
                               (cx, argc, vp, &className::funcName ) ;\
              if ( SDB_OK != rc )\
              {\
                 ret = JS_FALSE ;\
                 goto error ; \
              }\
           done:\
              return ret ;\
           error:\
              goto done ;\
           }

   #define JS_CONSTRUCT_FUNC_DEFINE( className, funcName )\
           static JSBool __##funcName( JSContext *cx , uintN argc , jsval *vp )\
           {\
              JSBool ret = JS_TRUE ; \
              INT32 rc = SDB_OK ; \
              typedef INT32 (className::*FUNC)(const _sptParamContainer &,\
                                              _sptReturnVal &,\
                                               bson::BSONObj &);\
              rc = sptInvoker::callConstructFunc<className, FUNC>\
                               (cx, argc, vp, &className::funcName ) ;\
              if ( SDB_OK != rc )\
              {\
                 ret = JS_FALSE ;\
                 goto error ; \
              }\
           done:\
              return ret ;\
           error:\
              goto done ;\
           }

   #define JS_DESTRUCT_FUNC_DEFINE( className, funcName ) \
            static void __##funcName( JSContext *cx ,  JSObject *obj )\
            {\
              typedef INT32 (className::*FUNC)() ;\
              sptInvoker::callDestructFunc<className, FUNC>\
                               (cx, obj, &className::funcName ) ;\
            }

   #define JS_RESOLVE_FUNC_DEFINE( className, funcName )\
           static JSBool __##funcName(JSContext *cx , JSObject *obj , jsid id ,\
                            uintN flags , JSObject ** objp)\
            {\
              JSBool ret = JS_TRUE ; \
              INT32 rc = SDB_OK ; \
              typedef INT32 (className::*FUNC)(const CHAR *idValue,\
                                               _sptReturnVal &,\
                                                bson::BSONObj &);\
              rc = sptInvoker::callResolveFunc<className, FUNC>\
                               (cx, obj, id, flags, objp, &className::funcName ) ;\
              if ( SDB_OK != rc )\
              {\
                 ret = JS_FALSE ;\
                 goto error ; \
              }\
           done:\
              return ret ;\
           error:\
              goto done ;\
           }


   #define JS_DECLARE_CLASS( className )\
           public: \
           static className *crtInstance(){ return SDB_OSS_NEW className();}\
           static void releaseInstance( className *instance ){ SAFE_OSS_DELETE(instance); } \
           class __objDesc : public _sptObjDesc\
           { \
           public: \
              __objDesc() ; \
              virtual ~__objDesc(){}\
           }; \
           static __objDesc __desc ; \
           private:

   #define JS_BEGIN_MAPPING(className, jsClassName) \
           className::__objDesc className::__desc ; \
           className::__objDesc::__objDesc() \
           {\
              setClassName(jsClassName) ;

   #define JS_ADD_MEMBER_FUNC( jsFuncName, funcName ) \
           _funcMap.addMemberFunc( jsFuncName, __##funcName ) ;

   #define JS_ADD_CONSTRUCT_FUNC( funcName ) \
           _funcMap.setConstructor( __##funcName ) ;

   #define JS_ADD_RESOLVE_FUNC( funcName ) \
           _funcMap.setResolver( __##funcName ) ;

   #define JS_ADD_DESTRUCT_FUNC( funcName ) \
           _funcMap.setDestructor( __##funcName ) ;

   #define JS_MAPPING_END() }
                      
}

#endif

