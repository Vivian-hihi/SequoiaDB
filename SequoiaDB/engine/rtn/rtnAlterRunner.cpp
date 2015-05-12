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

   Source File Name = rtnAlterRunner.cpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          08/05/2015  YW  Initial Draft

   Last Changed =

*******************************************************************************/

#include "rtnAlterRunner.hpp"
#include "pdTrace.hpp"
#include "pd.hpp"
#include "rtnTrace.hpp"
#include "pmdEDU.hpp"
#include "msgDef.hpp"
#include "dpsLogWrapper.hpp"

using namespace bson ;

namespace engine
{
   _rtnAlterFuncList _rtnAlterRunner::_funcList ;

   _rtnAlterRunner::_rtnAlterRunner()
   :_type( RTN_ALTER_INVALID ),
    _name( NULL ),
    _v( 0 ),
    /// only to init bsonobjiterator. we will reset it later.
    /// so do not use _i when _obj is reset.
    _i( _obj )
   {

   }

   _rtnAlterRunner::~_rtnAlterRunner()
   {
      clear() ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__RTNALTERRUNNER_INIT, "_rtnAlterRunner::init" ) 
   INT32 _rtnAlterRunner::init( const BSONObj &obj )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__RTNALTERRUNNER_INIT ) ;
      try
      {
         _obj = obj.copy() ;
         BSONElement e = _obj.getField( FIELD_NAME_VERSION ) ;
         if ( !e.isNumber() )
         {
            PD_LOG( PDERROR, "invalid type of field [%s], request obj: %s",
                    FIELD_NAME_VERSION, _obj.toString( FALSE, TRUE ).c_str() ) ;
            rc = SDB_INVALIDARG ;
            goto error ;
         }

         _v = e.Number() ;
         if ( SDB_ALTER_VERSION != _v )
         {
            PD_LOG( PDERROR, "invalid version:%d", _v ) ;
            rc = SDB_INVALIDARG ;
            goto error ;
         }
         
         e = _obj.getField( FIELD_NAME_ALTER_TYPE ) ;
         if ( String != e.type() )
         {
            PD_LOG( PDERROR, "invalid type of field [%s],request obj: %s",
                    FIELD_NAME_ALTER_TYPE, _obj.toString( FALSE, TRUE ).c_str() ) ;
            rc = SDB_INVALIDARG ;
            goto error ;
         }

         rc = _getObjType( e.valuestrsafe(), _type ) ;
         if ( SDB_OK != rc )
         {
            PD_LOG( PDERROR, "invalid alter type:%s", e.valuestrsafe() ) ;
            goto error ;
         }

         e = _obj.getField( FIELD_NAME_NAME ) ;
         if ( String != e.type() )
         {
            PD_LOG( PDERROR, "invalid type of field [%s], request obj: %s",
                    FIELD_NAME_NAME, _obj.toString( FALSE, TRUE ).c_str() ) ;
            rc = SDB_INVALIDARG ;
            goto error ;
         }
         _name = e.valuestrsafe() ;

         e = _obj.getField( FIELD_NAME_ALTER ) ;
         if ( Array != e.type() )
         {
            PD_LOG( PDERROR, "invalid type of field [%s],request obj: %s",
                    FIELD_NAME_ALTER, _obj.toString( FALSE, TRUE ).c_str() ) ;
            rc = SDB_INVALIDARG ;
            goto error ;
         }
         _rpc = e.embeddedObject() ;
         _i = BSONObjIterator( _rpc ) ;

         /// args is not necessary.
         e = _obj.getField( FIELD_NAME_ARGS ) ;
         if ( Object == e.type() )
         {
            _args = e.embeddedObject() ;
            e = _args.getField( CMD_ADMIN_PREFIX FIELD_NAME_OPTIONS ) ;
            if ( Object == e.type() )
            {
               _hint = e.embeddedObject() ;
            }
         }
      }
      catch ( std::exception &e )
      {
         PD_LOG( PDERROR, "unexpected error heppened:%s", e.what() ) ;
         rc = SDB_SYS ;
         goto error ;
      }
   done:
      PD_TRACE_EXITRC( SDB__RTNALTERRUNNER_INIT, rc ) ;
      return rc ;
   error:
      clear() ;
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__RTNALTERRUNNER_CLEAR, "_rtnAlterRunner::clear" )
   void _rtnAlterRunner::clear()
   {
      PD_TRACE_ENTRY( SDB__RTNALTERRUNNER_INIT ) ;
      _type = RTN_ALTER_INVALID ;
      _name = NULL ;
      _v = 0 ;
      _args = BSONObj() ;
      _rpc = BSONObj() ;
      _hint = BSONObj () ;
      _obj = BSONObj() ;
      _i = BSONObjIterator( _obj ) ;
      _options.reset() ;
      PD_TRACE_EXIT( SDB__RTNALTERRUNNER_INIT ) ;
      return ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__RTNALTERRUNNER_RUN, "_rtnAlterRunner::run" )
   INT32 _rtnAlterRunner::run( _pmdEDUCB *cb, _dpsLogWrapper *dpsCB )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__RTNALTERRUNNER_RUN ) ;
      if ( !_inited() )
      {
         PD_LOG( PDERROR, "runner has not been initialized yet" ) ;
         rc = SDB_SYS ;
         goto error ;
      }

      while ( _i.more() )
      {
         BSONElement e = _i.next() ;
         if ( Object != e.type() )
         {
            PD_LOG( PDERROR, "invalid alter:%s",
                    e.toString( FALSE, TRUE ).c_str() ) ;
            if ( _options.ignoreException )
            {
               continue ;
            }
            else
            {
               rc = SDB_INVALIDARG ;
               goto error ; 
            }
         }

         rc = _run( e.embeddedObject(), cb,  dpsCB ) ;
         if ( SDB_OK != rc )
         {
            PD_LOG( PDERROR, "failed to run alter:%d", rc ) ;
            if ( _options.ignoreException )
            {
               rc = SDB_OK ;
               continue ;
            }
            else
            {
               goto error ;
            }
         }
      }
   done:
      PD_TRACE_EXITRC( SDB__RTNALTERRUNNER_RUN, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__RTNALTERRUNNER__RUN, "_rtnAlterRunner::_run" )
   INT32 _rtnAlterRunner::_run( const bson::BSONObj &rpc,
                              _pmdEDUCB *cb,
                              _dpsLogWrapper *dpsCB )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__RTNALTERRUNNER__RUN ) ;
      RTN_ALTER_FUNC func = NULL ;
      BSONElement args ;
      BSONElement name = rpc.getField( FIELD_NAME_NAME ) ;
      if ( String != name.type() )
      {
         PD_LOG( PDERROR, "invalid alter name:%s",
                 rpc.toString( FALSE, TRUE ).c_str() ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }

      rc = _getFunc( name.valuestr(), func ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to get func of alter[%s], rc:%d",
                 name.valuestr(), rc ) ;
         goto error ;
      }

      rc = (*func)( _name,
                    _args,
                    rpc.getObjectField( FIELD_NAME_ARGS ),
                    cb,
                    dpsCB ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "alter returned error:%d", rc ) ;
         goto error ;
      }
   done:
      PD_TRACE_EXITRC( SDB__RTNALTERRUNNER__RUN, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   INT32 _rtnAlterRunner::_getFunc( const CHAR *name,
                                  RTN_ALTER_FUNC &func )
   {
      return _funcList.getFunc( _type, name, func ) ;
   }

   INT32 _rtnAlterRunner::_getObjType( const CHAR *name,
                                      RTN_ALTER_TYPE &type )
   {
      return _funcList.getObjType( name, type ) ;
   }
}


