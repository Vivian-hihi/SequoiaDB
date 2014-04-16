/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = sptInvoker.cpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          31/03/2014  YW  Initial Draft

   Last Changed =

*******************************************************************************/

#include "sptInvoker.hpp"
#include "ossUtil.hpp"

using namespace bson ;

namespace engine
{
   INT32 _sptInvoker::_getValFromProperty( JSContext *cx,
                                           const sptProperty &pro,
                                           jsval &val )
   {
      INT32 rc = SDB_OK ;
      if ( String == pro.getType() )
      {
         JSString *jsstr = JS_NewStringCopyN( cx, pro.getString(),
                                         ossStrlen( pro.getString() ) ) ;
         if ( NULL == jsstr )
         {
            PD_LOG( PDERROR, "failed to create a js string" ) ;
            rc = SDB_SYS ;
            goto error ;
         }

         val = STRING_TO_JSVAL( jsstr ) ;
      }
      else if ( Bool == pro.getType() )
      {
         val = BOOLEAN_TO_JSVAL( pro.getBool() ) ;
      }
      else if ( NumberInt == pro.getType() )
      {
         val = INT_TO_JSVAL( pro.getINT32() ) ;
      }
      else if ( NumberDouble == pro.getType() )
      {
         val = DOUBLE_TO_JSVAL( pro.getFLOAT64() ) ;
      }
      else
      {
         PD_LOG( PDERROR, "the type %d is not surpported yet.", pro.getType() ) ;
         rc = SDB_SYS ;
         goto error ;
      }
   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _sptInvoker::_callbackDone( JSContext *cx, JSObject *obj,
                                    const _sptReturnVal &rval,
                                    const bson::BSONObj &detail,
                                    jsval *rvp )
   {
      INT32 rc = SDB_OK ;
      const sptProperty &rpro = rval.getVal() ;
      jsval val = JSVAL_VOID ;

      if ( EOO == rpro.getType() )
      {
         *rvp = JSVAL_VOID ;
         goto done ;
      }
      else if ( Object == rpro.getType() )
      {
         JSObject *jsObj = JS_NewObject ( cx, (JSClass *)(rval.getClassDef()), 0 , 0 ) ;
         if ( NULL == jsObj )
         {
            /// WARNING: it will cause mem leak, because we can not delete rpro.getObj().
            PD_LOG( PDERROR, "faile to new js object" ) ;
            rc = SDB_OOM ;
            goto error ;
         }

         JS_SetPrivate( cx, jsObj, rpro.getObj() ) ;

         if ( !rval.getValProperties().empty() )
         {
            rc = _sptInvoker::setProperty( cx, jsObj, rval.getValProperties() ) ;
            if ( SDB_OK != rc )
            {
               goto error ;
            }
         }

         val = OBJECT_TO_JSVAL( jsObj ) ;
      }
      else
      {
         rc = _getValFromProperty( cx, rpro, val ) ;
         if ( SDB_OK != rc )
         {
            goto error ;
         }
      }

      if ( !rpro.getName().empty() )
      {
         if ( !JS_SetProperty( cx, obj,
                               rpro.getName().c_str(),
                               &val ))
         {
            PD_LOG( PDERROR, "failed to set obj to parent obj" ) ;
            rc = SDB_SYS ;
            goto error ;
         }
      }

      *rvp = val ;
   done:
      return rc ;
   error:
      goto done ;
   } 

   INT32 _sptInvoker::setProperty( JSContext *cx,
                                   JSObject *obj,
                                   const SPT_PROPERTIES &properties )
   {
      INT32 rc = SDB_OK ;
      SPT_PROPERTIES::const_iterator itr = properties.begin() ;
      for ( ; itr != properties.end(); itr++ )
      {
         SDB_ASSERT( !itr->getName().empty(), "name can not be empty" )
         jsval val = JSVAL_VOID ;
         rc = _getValFromProperty( cx, *itr, val ) ;
         if ( SDB_OK != rc )
         {
            goto error ;
         }
        
         if ( !JS_SetProperty( cx, obj, itr->getName().c_str(), &val ) )
         {
            PD_LOG( PDERROR, "failed to set property of obj" ) ;
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

