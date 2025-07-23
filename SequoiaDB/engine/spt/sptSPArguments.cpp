/*******************************************************************************

   Copyright (C) 2011-Present SequoiaDB Ltd.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

   Source File Name = sptSPArguments.cpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          31/03/2014  YW  Initial Draft

   Last Changed =

*******************************************************************************/
#include "sptSPArguments.hpp"
#include "charsetConvertorFactory.hpp"
#include "charsetConvertorInterface.hpp"
#include "charsetDef.hpp"
#include "charsetUtils.hpp"
#include "ossErr.h"
#include "ossTypes.h"
#include "sptSPDef.hpp"
#include "pd.hpp"
#include "sptConvertor.hpp"
#include "sptObjDesc.hpp"
#include "sptSPVal.hpp"
#include "sptSPObject.hpp"
#include "sptDBNumberLong.hpp"
#include <cstddef>

using namespace bson ;

namespace engine
{
   _sptSPArguments::_sptSPArguments( JSContext *context, uintN argc,
                                     jsval *vp, JSObject *pObj )
   :_context(context),
    _argc(argc),
    _vp(vp),
    _pObject( NULL )
   {
      SDB_ASSERT( NULL != _context && NULL != _vp, "can not be NULL" ) ;
      if ( pObj )
      {
         _pObject = SDB_OSS_NEW sptSPObject( _context, pObj ) ;
         SDB_ASSERT( _pObject, "Alloc out-of-memory" ) ;
      }

      sptPrivateData *privateData = NULL ;
      privateData = getPrivateData() ;
      if ( privateData && privateData->getScope() )
      {
         sptScope* scope = privateData->getScope() ;
         std::string clientCharset = scope->getResultsCharset() ;
         Charset clientCS = charsetParse( clientCharset ) ;
         _inputConvertor = charsetConvertorFactory::get( clientCS,
                                                         CHARSET_UTF8 ) ;
         if ( clientCS != CHARSET_UTF8 && NULL == _inputConvertor )
         {
            SDB_ASSERT( 0, "Failed to get input data charset convertor" ) ;
         }
         _outputConvertor = charsetConvertorFactory::get( CHARSET_UTF8,
                                                          clientCS ) ;
         if ( clientCS != CHARSET_UTF8 && NULL == _outputConvertor )
         {
            SDB_ASSERT( 0, "Failed to get output data charset convertor" ) ;
         }
      }
   }

   _sptSPArguments::~_sptSPArguments()
   {
      if ( _pObject )
      {
         SDB_OSS_DEL _pObject ;
         _pObject = NULL ;
      }
      _context = NULL ;
      _vp = NULL ;
   }

   INT32 _sptSPArguments::getString( UINT32 pos,
                                     std::string &value,
                                     BOOLEAN strict ) const
   {
      INT32 rc = SDB_OK ;
      sptSPVal spVal ;
      jsval *val = NULL ;
      std::string convertedStr ;

      _errMsg.clear() ;

      if ( _argc <= pos )
      {
         rc = SDB_OUT_OF_BOUND ;
         goto error ;
      }

      val = _getValAtPos( pos ) ;
      if ( NULL == val )
      {
         _errMsg = "Failed to get val at pos" ;
         rc = SDB_SYS ;
         goto error ;
      }

      spVal.reset( _context, *val ) ;

      /// strict for String
      if ( strict )
      {
         if ( !spVal.isString() )
         {
            _errMsg = "Paramter is not string" ;
            rc = SDB_INVALIDARG ;
            goto error ;
         }
      }

      rc = spVal.toString( convertedStr ) ;
      if ( rc )
      {
         _errMsg = "Failed to convert a jsval to string" ;
         goto error ;
      }

      rc = _convert( convertedStr, value ) ;
      if ( rc )
      {
         goto error ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }


   jsval *_sptSPArguments::_getValAtPos( UINT32 pos ) const
   {
      return JS_ARGV( _context, _vp ) + pos ;
   }

   template<typename T>
   INT32 _sptSPArguments::_convert( const T &in, T &out ) const
   {
      INT32 rc  = SDB_OK ;
      if ( _outputConvertor )
      {
         rc = _outputConvertor->convert(in, out) ;
         if ( rc )
         {
            _errMsg = "Failed to convert charset for string or BSON" ;
            goto error ;
         }
      }
      else if ( NULL == _outputConvertor ) // No need to convert
      {
         out = in ;
      }
   done:
      return rc ;
   error:
      goto done ;
   }

   template<typename T>
   INT32 _sptSPArguments::_convertArray( const vector< T > &in,
                                         vector< T > &out) const
   {
      INT32 rc = SDB_OK ;
      // Convert charset for string or BSONObj
      for ( size_t i = 0; i < in.size(); i++ )
      {
         T convertedObj ;
         rc = _convert< T > (in[i], convertedObj ) ;
         if ( rc )
         {
            goto error ;
         }
         out.push_back( convertedObj ) ;
      }
   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _sptSPArguments::getBsonobj( UINT32 pos,
                                      bson::BSONObj &value,
                                      BOOLEAN strict,
                                      BOOLEAN allowNull ) const
   {
      INT32 rc = SDB_OK ;
      jsval *val = NULL ;
      sptSPVal spVal ;
      sptConvertor convertor( _context, strict ) ;

      _errMsg.clear() ;

      if ( _argc <= pos )
      {
         rc = SDB_OUT_OF_BOUND ;
         goto error ;
      }

      val = _getValAtPos( pos ) ;
      if ( NULL == val )
      {
         _errMsg = "Failed to get val at pos" ;
         rc = SDB_SYS ;
         goto error ;
      }

      spVal.reset( _context, *val ) ;

      if ( spVal.isNull() )
      {
         if ( !allowNull )
         {
            rc = SDB_INVALIDARG ;
            goto error ;
         }
         else
         {
            goto done ;
         }
      }
      else if ( !spVal.isObject() )
      {
         _errMsg = "Parameter is not a object" ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }
      else
      {
         bson::BSONObj obj;
         rc = convertor.toBson( &spVal, obj ) ;
         if ( SDB_OK != rc )
         {
            _errMsg = convertor.getErrMsg() ;
            goto error ;
         }

         rc = _convert< BSONObj > ( obj, value ) ;
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

   INT32 _sptSPArguments::getArray( UINT32 pos, vector< bson::BSONObj > &value,
                                    BOOLEAN strict ) const
   {
      INT32 rc = SDB_OK ;
      JSObject *jsObj = NULL ;
      jsval *val = NULL ;
      sptConvertor convertor( _context, strict ) ;
      vector< bson::BSONObj > convertedObjs ;

      _errMsg.clear() ;

      if ( _argc <= pos )
      {
         rc = SDB_OUT_OF_BOUND ;
         goto error ;
      }

      val = _getValAtPos( pos ) ;
      if ( NULL == val )
      {
         _errMsg = "Failed to get val at pos" ;
         rc = SDB_SYS ;
         goto error ;
      }

      if ( !JSVAL_IS_OBJECT( *val ) )
      {
         _errMsg = "Parameter is not Object" ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }

      jsObj = JSVAL_TO_OBJECT( *val ) ;
      if ( NULL == jsObj )
      {
         _errMsg = "Failed to convert jsval to Object" ;
         rc = SDB_SYS ;
         goto error ;
      }

      rc = convertor.toObjArray( jsObj, convertedObjs ) ;
      if ( SDB_OK != rc )
      {
         _errMsg = convertor.getErrMsg() ;
         goto error ;
      }

      rc = _convertArray( convertedObjs, value ) ;
      if ( rc )
      {
         goto error ;
      }
   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _sptSPArguments::getArray( UINT32 pos, vector< string > &value,
                                    BOOLEAN strict ) const
   {
      INT32 rc = SDB_OK ;
      JSObject *jsObj = NULL ;
      jsval *val = NULL ;
      sptConvertor convertor( _context, strict ) ;
      vector< string > convertedStr ;

      _errMsg.clear() ;

      if ( _argc <= pos )
      {
         rc = SDB_OUT_OF_BOUND ;
         goto error ;
      }

      val = _getValAtPos( pos ) ;
      if ( NULL == val )
      {
         _errMsg = "Failed to get val at pos" ;
         rc = SDB_SYS ;
         goto error ;
      }

      if ( !JSVAL_IS_OBJECT( *val ) )
      {
         _errMsg = "Parameter is not Object" ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }

      jsObj = JSVAL_TO_OBJECT( *val ) ;
      if ( NULL == jsObj )
      {
         _errMsg = "Failed to convert jsval to object" ;
         rc = SDB_SYS ;
         goto error ;
      }

      rc = convertor.toStrArray( jsObj, convertedStr ) ;
      if ( SDB_OK != rc )
      {
         _errMsg = convertor.getErrMsg() ;
         goto error ;
      }

      rc = _convertArray( convertedStr, value ) ;
      if ( rc )
      {
         goto error ;
      }
   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _sptSPArguments::getUserObj( UINT32 pos, const _sptObjDesc &objDesc,
                                      const void** value ) const
   {
      INT32 rc = SDB_OK ;
      JSObject *jsObj = NULL ;
      jsval *val = NULL ;

      _errMsg.clear() ;

      if ( _argc <= pos )
      {
         rc = SDB_OUT_OF_BOUND ;
         goto error ;
      }

      val = _getValAtPos( pos ) ;
      if ( NULL == val )
      {
         _errMsg = "Failed to get val at pos" ;
         rc = SDB_SYS ;
         goto error ;
      }

      if ( !JSVAL_IS_OBJECT( *val ) )
      {
         _errMsg = "Parameter is not Object" ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }

      jsObj = JSVAL_TO_OBJECT( *val ) ;
      if ( NULL == jsObj )
      {
         _errMsg = "Failed to convert jsval to object" ;
         rc = SDB_SYS ;
         goto error ;
      }

      if( string( objDesc.getJSClassName() ) !=
          sptGetObjFactory()->getClassName( _context, jsObj ) )
      {
         rc = SDB_INVALIDARG ;
         _errMsg = "Object is not the instance of " ;
         _errMsg += objDesc.getJSClassName() ;
         goto error ;
      }

      *value = JS_GetPrivate( _context, jsObj ) ;
      if( *value == NULL )
      {
         rc = SDB_SYS ;
         _errMsg = "Faild to convert jsobj to user Object" ;
         goto error ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _sptSPArguments::getBoolean( UINT32 pos,
                                      BOOLEAN &value,
                                      BOOLEAN strict ) const
   {
      INT32 rc = SDB_OK ;
      sptSPVal spVal ;
      jsval *val = NULL ;

      _errMsg.clear() ;

      if ( _argc <= pos )
      {
         rc = SDB_OUT_OF_BOUND ;
         goto error ;
      }

      val = _getValAtPos( pos ) ;
      if ( NULL == val )
      {
         _errMsg = "Failed to get val at pos" ;
         rc = SDB_SYS ;
         goto error ;
      }

      spVal.reset( _context, *val ) ;

      /// strict for Boolean
      if ( strict )
      {
         if ( !spVal.isBoolean() )
         {
            _errMsg = "Paramter is not boolean" ;
            rc = SDB_INVALIDARG ;
            goto error ;
         }
      }

      rc = spVal.toBoolean( value ) ;
      if ( rc )
      {
         _errMsg = "Failed to convert a jsval to boolean" ;
         goto error ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   sptPrivateData* _sptSPArguments::getPrivateData( ) const
   {
      return ( sptPrivateData* )JS_GetContextPrivate( _context ) ;
   }

   // Charset convertor used to convert data from sdb server to
   // spidermonkey, Charset of output data from spidermonkey is UTF8
   charsetConvertorInterface* _sptSPArguments::getInputDataConvertor() const
   {
      return _inputConvertor.get() ;
   }

   // Charset convertor used to convert data from spidermonkey to
   // sdb server, Charset of output data from spidermonkey is UTF8
   charsetConvertorInterface* _sptSPArguments::getOutputDataConvertor() const
   {
      return _outputConvertor.get() ;
   }

   BOOLEAN _sptSPArguments::isString( UINT32 pos ) const
   {
      jsval *val = NULL ;
      if ( _argc > pos && NULL != ( val = _getValAtPos( pos ) ) &&
           JSVAL_IS_STRING( *val ) )
      {
         return TRUE ;
      }
      return FALSE ;
   }

   BOOLEAN _sptSPArguments::isNull( UINT32 pos ) const
   {
      jsval *val = NULL ;
      if ( _argc > pos && NULL != ( val = _getValAtPos( pos ) ) &&
           JSVAL_IS_NULL( *val ) )
      {
         return TRUE ;
      }
      return FALSE ;
   }

   BOOLEAN _sptSPArguments::isVoid( UINT32 pos ) const
   {
      jsval *val = NULL ;
      if ( _argc > pos && NULL != ( val = _getValAtPos( pos ) ) &&
           JSVAL_IS_VOID( *val ) )
      {
         return TRUE ;
      }
      return FALSE ;
   }

   BOOLEAN _sptSPArguments::isInt( UINT32 pos ) const
   {
      jsval *val = NULL ;
      if ( _argc > pos && NULL != ( val = _getValAtPos( pos ) ) &&
           JSVAL_IS_INT( *val ) )
      {
         return TRUE ;
      }
      return FALSE ;
   }

   BOOLEAN _sptSPArguments::isLong( UINT32 pos ) const
   {
      return isUserObj( pos, sptDBNumberLong::__desc ) ;
   }

   BOOLEAN _sptSPArguments::isDouble( UINT32 pos ) const
   {
      jsval *val = NULL ;
      if ( _argc > pos && NULL != ( val = _getValAtPos( pos ) ) &&
           JSVAL_IS_DOUBLE( *val ) )
      {
         return TRUE ;
      }
      return FALSE ;
   }

   BOOLEAN _sptSPArguments::isNumber( UINT32 pos ) const
   {
      jsval *val = NULL ;
      if ( ( _argc > pos && NULL != ( val = _getValAtPos( pos ) ) &&
             JSVAL_IS_NUMBER( *val ) ) || isLong( pos ) )
      {
         return TRUE ;
      }
      return FALSE ;
   }

   BOOLEAN _sptSPArguments::isObject( UINT32 pos ) const
   {
      jsval *val = NULL ;
      if ( _argc > pos && NULL != ( val = _getValAtPos( pos ) ) &&
           JSVAL_IS_OBJECT( *val ) )
      {
         return TRUE ;
      }
      return FALSE ;
   }

   BOOLEAN _sptSPArguments::isBoolean( UINT32 pos ) const
   {
      jsval *val = NULL ;
      if ( _argc > pos && NULL != ( val = _getValAtPos( pos ) ) &&
           JSVAL_IS_BOOLEAN( *val ) )
      {
         return TRUE ;
      }
      return FALSE ;
   }

   BOOLEAN _sptSPArguments::isUserObj( UINT32 pos,
                                       const _sptObjDesc &objDesc ) const
   {
      jsval *val = NULL ;
      JSObject *jsObj = NULL ;

      if ( _argc > pos &&
           NULL != ( val = _getValAtPos( pos ) ) &&
           JSVAL_IS_OBJECT( *val ) &&
           NULL != ( jsObj = JSVAL_TO_OBJECT( *val ) ) &&
           string( objDesc.getJSClassName() ) ==
           sptGetObjFactory()->getClassName( _context, jsObj ) )
      {
         return TRUE ;
      }
      return FALSE ;
   }

   BOOLEAN _sptSPArguments::isArray( UINT32 pos ) const
   {
      jsval *val = NULL ;
      JSObject *jsObj = NULL ;
      if ( _argc > pos && NULL != ( val = _getValAtPos( pos ) ) &&
           JSVAL_IS_OBJECT( *val ) &&
           NULL != ( jsObj = JSVAL_TO_OBJECT( *val ) ) &&
           JS_IsArrayObject( _context, jsObj ) )
      {
         return TRUE ;
      }
      return FALSE ;
   }

   string _sptSPArguments::getUserObjClassName( UINT32 pos ) const
   {
      jsval *val = NULL ;
      JSObject *jsObj = NULL ;

      if( _argc > pos &&
           NULL != ( val = _getValAtPos( pos ) ) &&
           JSVAL_IS_OBJECT( *val ) &&
           NULL != ( jsObj = JSVAL_TO_OBJECT( *val ) ) )
      {
         return sptGetObjFactory()->getClassName( _context, jsObj ) ;
      }
      return "" ;
   }

   string _sptSPArguments::getErrMsg() const
   {
      return _errMsg ;
   }

   BOOLEAN _sptSPArguments::hasErrMsg() const
   {
      return _errMsg.empty() ? FALSE : TRUE ;
   }

   #define NATIVE_VALUE_EQ( pData, type, value ) \
      do \
      { \
         switch( type ) \
         { \
            case SPT_NATIVE_CHAR : \
               *(CHAR*)pData = ( CHAR )( value ) ; \
               break ; \
            case SPT_NATIVE_INT16 : \
               *(INT16*)pData = ( INT16 )( value ) ; \
               break ; \
            case SPT_NATIVE_INT32 : \
               *(INT32*)pData = ( INT32 )( value ) ; \
               break ; \
            case SPT_NATIVE_INT64 : \
               *(INT64*)pData = ( INT64 )( value ) ; \
               break ; \
            case SPT_NATIVE_FLOAT32 : \
               *(FLOAT32*)pData = ( FLOAT32 )( value ) ; \
               break ; \
            case SPT_NATIVE_FLOAT64 : \
               *(FLOAT64*)pData = ( FLOAT64 )( value ) ; \
               break ; \
            default : \
               _errMsg = "unexpect type of the parameter" ; \
               rc = SDB_INVALIDARG ; \
               goto error ; \
         } \
      } while ( 0 )


   INT32 _sptSPArguments::getNative( UINT32 pos, void *value,
                                     SPT_NATIVE_TYPE type ) const
   {
      INT32 rc = SDB_OK ;
      jsval *val = NULL ;
      JSObject *jsObj = NULL ;

      _errMsg.clear() ;

      if ( _argc <= pos )
      {
         rc = SDB_OUT_OF_BOUND ;
         goto error ;
      }

      val = _getValAtPos( pos ) ;
      if ( NULL == val )
      {
         _errMsg = "Failed to get val at pos" ;
         rc = SDB_SYS ;
         goto error ;
      }

      if ( JSVAL_IS_INT( *val ) )
      {
         NATIVE_VALUE_EQ( value, type, JSVAL_TO_INT( *val ) ) ;
      }
      else if ( JSVAL_IS_BOOLEAN( *val ) )
      {
         NATIVE_VALUE_EQ( value, type, JSVAL_TO_BOOLEAN( *val ) ) ;
      }
      else if ( JSVAL_IS_DOUBLE( *val ) )
      {
         NATIVE_VALUE_EQ( value, type, JSVAL_TO_DOUBLE( *val ) ) ;
      }
      else if ( JSVAL_IS_OBJECT( *val ) &&
                NULL != ( jsObj = JSVAL_TO_OBJECT( *val ) ) &&
                string( SPT_NUMBERLONG_NAME ) ==
                      sptGetObjFactory()->getClassName( _context, jsObj ) )
      {
         sptDBNumberLong *numberLong =
               (sptDBNumberLong *) JS_GetPrivate( _context, jsObj ) ;
         if( NULL == numberLong )
         {
            rc = SDB_SYS ;
            _errMsg = "Faild to convert jsobj to user Object" ;
            goto error ;
         }
         NATIVE_VALUE_EQ( value, type, numberLong->getValue() ) ;
      }
      else
      {
         _errMsg = "Parameter is not a native value" ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }
}

