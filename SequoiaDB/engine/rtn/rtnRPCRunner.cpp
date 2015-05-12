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

   Source File Name = rtnRPCRunner.cpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          08/05/2015  YW  Initial Draft

   Last Changed =

*******************************************************************************/

#include "rtnRPCRunner.hpp"
#include "pdTrace.hpp"
#include "pd.hpp"
#include "rtnTrace.hpp"
#include "pmdEDU.hpp"
#include "msgDef.hpp"
#include "dpsLogWrapper.hpp"

using namespace bson ;

namespace engine
{
   _rtnRPCFuncList _rtnRPCRunner::_funcList ;

   _rtnRPCRunner::_rtnRPCRunner()
   :_type( RTN_RPC_INVALID ),
    _name( NULL ),
    _v( 0 ),
    /// only to init bsonobjiterator. we will reset it later.
    /// so do not use _i when _obj is reset.
    _i( _obj )
   {

   }

   _rtnRPCRunner::~_rtnRPCRunner()
   {
      clear() ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__RTNRPCRUNNER_INIT, "_rtnRPCRunner::init" ) 
   INT32 _rtnRPCRunner::init( const BSONObj &obj )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__RTNRPCRUNNER_INIT ) ;
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
         if ( SDB_RPC_VERSION != _v )
         {
            PD_LOG( PDERROR, "invalid version:%d", _v ) ;
            rc = SDB_INVALIDARG ;
            goto error ;
         }
         
         e = _obj.getField( FIELD_NAME_RPC_TYPE ) ;
         if ( String != e.type() )
         {
            PD_LOG( PDERROR, "invalid type of field [%s],request obj: %s",
                    FIELD_NAME_RPC_TYPE, _obj.toString( FALSE, TRUE ).c_str() ) ;
            rc = SDB_INVALIDARG ;
            goto error ;
         }

         rc = _getObjType( e.valuestrsafe(), _type ) ;
         if ( SDB_OK != rc )
         {
            PD_LOG( PDERROR, "invalid rpc type:%s", e.valuestrsafe() ) ;
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

         e = _obj.getField( FIELD_NAME_RPC ) ;
         if ( Array != e.type() )
         {
            PD_LOG( PDERROR, "invalid type of field [%s],request obj: %s",
                    FIELD_NAME_RPC, _obj.toString( FALSE, TRUE ).c_str() ) ;
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
      PD_TRACE_EXITRC( SDB__RTNRPCRUNNER_INIT, rc ) ;
      return rc ;
   error:
      clear() ;
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__RTNRPCRUNNER_CLEAR, "_rtnRPCRunner::clear" )
   void _rtnRPCRunner::clear()
   {
      PD_TRACE_ENTRY( SDB__RTNRPCRUNNER_INIT ) ;
      _type = RTN_RPC_INVALID ;
      _name = NULL ;
      _v = 0 ;
      _args = BSONObj() ;
      _rpc = BSONObj() ;
      _hint = BSONObj () ;
      _obj = BSONObj() ;
      _i = BSONObjIterator( _obj ) ;
      _options.reset() ;
      PD_TRACE_EXIT( SDB__RTNRPCRUNNER_INIT ) ;
      return ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__RTNRPCRUNNER_RUN, "_rtnRPCRunner::run" )
   INT32 _rtnRPCRunner::run( _pmdEDUCB *cb, _dpsLogWrapper *dpsCB )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__RTNRPCRUNNER_RUN ) ;
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
            PD_LOG( PDERROR, "invalid rpc:%s",
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
            PD_LOG( PDERROR, "failed to run rpc:%d", rc ) ;
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
      PD_TRACE_EXITRC( SDB__RTNRPCRUNNER_RUN, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__RTNRPCRUNNER__RUN, "_rtnRPCRunner::_run" )
   INT32 _rtnRPCRunner::_run( const bson::BSONObj &rpc,
                              _pmdEDUCB *cb,
                              _dpsLogWrapper *dpsCB )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__RTNRPCRUNNER__RUN ) ;
      RTN_RPC_FUNC func = NULL ;
      BSONElement args ;
      BSONElement name = rpc.getField( FIELD_NAME_NAME ) ;
      if ( String != name.type() )
      {
         PD_LOG( PDERROR, "invalid rpc name:%s",
                 rpc.toString( FALSE, TRUE ).c_str() ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }

      rc = _getFunc( name.valuestr(), func ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to get func of rpc[%s], rc:%d",
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
         PD_LOG( PDERROR, "rpc returned error:%d", rc ) ;
         goto error ;
      }
   done:
      PD_TRACE_EXITRC( SDB__RTNRPCRUNNER__RUN, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   INT32 _rtnRPCRunner::_getFunc( const CHAR *name,
                                  RTN_RPC_FUNC &func )
   {
      return _funcList.getFunc( _type, name, func ) ;
   }

   INT32 _rtnRPCRunner::_getObjType( const CHAR *name,
                                      RTN_RPC_TYPE &type )
   {
      return _funcList.getObjType( name, type ) ;
   }
}


