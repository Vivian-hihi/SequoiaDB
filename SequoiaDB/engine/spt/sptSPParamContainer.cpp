/******************************************************************************


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

   Source File Name = sptSPParamContainer.cpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          31/03/2014  YW  Initial Draft

   Last Changed =

*******************************************************************************/

#include "sptSPParamContainer.hpp"
#include "sptSPDef.hpp"
#include "pd.hpp"
#include <typeinfo>

using namespace bson ;

const UINT32 MAX_SKIP_LEN  = 15 ;
const UINT32 MAX_RULE_LEN = MAX_SKIP_LEN + 2 ;

namespace engine
{
   extern INT32 utilGetBsonRawFromCtx( JSContext *cx, JSObject *obj, CHAR **raw ) ;

   _sptSPParamContainer::_sptSPParamContainer( JSContext *context, uintN argc, jsval *vp )
   :_context(context),
    _argc(argc),
    _vp(vp)
   {
      SDB_ASSERT( NULL != _context && NULL != _vp, "can not be NULL" )
   }

   _sptSPParamContainer::~_sptSPParamContainer()
   {
      _context = NULL ;
      _vp = NULL ;
   }

   template <typename T>
   INT32 _sptSPParamContainer::_getNative( UINT32 skip, BOOLEAN optional,
                                           BSONType type, T &value )
   {
      INT32 rc = SDB_OK ;
      CHAR rule[MAX_RULE_LEN + 1] = {0} ;
      JSBool ret = TRUE ;

       _getRule( skip, optional, type, rule );

      ret = JS_ConvertArguments( _context, _argc,
                                 JS_ARGV ( _context, _vp ),
                                 rule, &value ) ;
      if ( !ret )
      {
         PD_LOG( PDERROR, "failed to convert arguments" ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }
   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _sptSPParamContainer::getString( UINT32 skip,
                                          BOOLEAN optional,
                                          std::string &value )
   {
      INT32 rc = SDB_OK ;
      CHAR rule[MAX_RULE_LEN + 1] = {0} ;
      JSBool ret = TRUE ;
      JSString *jsStr = NULL ;
      CHAR *str = NULL ;

      _getRule( skip, optional, bson::String, rule );
      ret = JS_ConvertArguments ( _context , _argc , JS_ARGV ( _context , _vp ) ,
                                  rule , &jsStr ) ;
      if ( !ret || NULL == jsStr )
      {
         PD_LOG( PDERROR, "failed to convert string argument" ) ;
         rc = SDB_SYS ;
         goto error ;
      }

      str = JS_EncodeString ( _context , jsStr ) ;
      if ( NULL == str )
      {
         PD_LOG( PDERROR, "failed to convert a js str to a normal str" ) ;
         rc = SDB_SYS ;
         goto error ;
      }

      value.assign( str ) ;
      
   done:
      SAFE_JS_FREE( _context, str ) ;
      return rc ;
   error:
      goto done ;
   }
   
   INT32 _sptSPParamContainer::getBSONObj( UINT32 skip,
                                           BOOLEAN optional,
                                           bson::BSONObj &value )
   {
      INT32 rc = SDB_OK ;
      CHAR rule[MAX_RULE_LEN + 1] = {0} ;
      JSBool ret = TRUE ;
      JSObject *jsObj = NULL ;
      CHAR *rawData = NULL ;

      _getRule( skip, optional, bson::Object, rule );
      ret = JS_ConvertArguments ( _context , _argc , JS_ARGV ( _contex , _vp ) ,
                                  rule , &jsObj ) ;
      if ( !ret || NULL == jsObj )
      {
         PD_LOG( PDERROR, "failed to convert object argument" ) ;
         rc = SDB_SYS ;
         goto error ;
      }

      
      rc = utilGetBsonRawFromCtx( _context, jsObj, &rawData ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to convert js object to bson object:%d", rc ) ;
         goto error ;
      }

      value = bson::BSONObj( rawData ) ;
      value.getOwned() ;
   done:
      free( rawData ) ;
      return rc ;
   error:
      goto done ;
   }

   void _sptSPParamContainer::_getRule( UINT32 skip,
                                        BOOLEAN optional,
                                        bson::BSONType type,
                                        CHAR rule[] )
   {
      SDB_ASSERT( skip <= MAX_SKIP_LEN,
                  "what a ugly function which even needs so many arguments" )
      UINT32 i = 0 ;
      for ( ; i < skip; i++ )
      {
         rule[i] = '*' ;
      }

      if ( optional )
      {
         rule[i++] = '/' ;
      }

      if ( NumberInt == type )
      {
         rule[i++] = 'i' ;
      }
      else if ( String == type )
      {
         rule[i++] = 'S' ;
      }
      else if ( NumberDouble == type )
      {
         rule[i++] = 'I' ;
      }
      else if ( Object == type )
      {
         rule[i++] = 'o' ;
      }
      else if ( Bool == type )
      {
         rule[i++] = 'b' ;
      }
      else
      {
         SDB_ASSERT( FALSE, "do not surpport this type" )
      }

      rule[i] = '\0' ;

      PD_LOG( PDDEBUG, "we get a rule like: %s", rule ) ;
      return ;
   }
}

