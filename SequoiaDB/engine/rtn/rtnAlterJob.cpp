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

   Source File Name = rtnAlterJob.cpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          08/05/2015  YW  Initial Draft

   Last Changed =

*******************************************************************************/

#include "rtnAlterJob.hpp"
#include "pd.hpp"
#include "rtnTrace.hpp"
#include "pdTrace.hpp"
#include "msgDef.hpp"

using namespace bson ;

namespace engine
{
   _rtnAlterJob::_rtnAlterJob()
   :_type( RTN_ALTER_INVALID ),
    _name( NULL ),
    _v( 0 )
   {

   }

   _rtnAlterJob::~_rtnAlterJob()
   {

   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__RTNALTERJOB_INIT, "_rtnAlterJob::init" )
   INT32 _rtnAlterJob::init( const bson::BSONObj &obj,
                             BOOLEAN verifyTask )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__RTNALTERJOB_INIT ) ;
      clear() ;
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

         _type = _getObjType( e.valuestrsafe() ) ;
         if ( RTN_ALTER_INVALID == _type )
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
         _tasks = e.embeddedObject() ;

         /// options is not necessary.
         e = _obj.getField( FIELD_NAME_OPTIONS ) ;
         if ( Object == e.type() )
         {
            _optionsObj = e.embeddedObject() ;
            _extractOptions( _optionsObj ) ;
         }
      }
      catch ( std::exception &e )
      {
         PD_LOG( PDERROR, "unexpected error heppened:%s", e.what() ) ;
         rc = SDB_SYS ;
         goto error ;
      }

      if ( verifyTask )
      {
         rc = _verifyTasks() ;
         if ( SDB_OK != rc )
         {
            PD_LOG( PDERROR, "failed to verify tasks:%d", rc ) ;
            goto error ;
         }
      } 
   done:
      PD_TRACE_EXITRC( SDB__RTNALTERJOB_INIT, rc ) ;
      return rc ;
   error:
      clear() ;
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__RTNALTERJOB_CLEAR, "_rtnAlterJob::clear" )
   void _rtnAlterJob::clear()
   {
      PD_TRACE_ENTRY( SDB__RTNALTERJOB_CLEAR ) ;
      _type = RTN_ALTER_INVALID ;
      _name = NULL ;
      _v = 0 ;
      _optionsObj = BSONObj() ;
      _tasks = BSONObj() ;
      _obj = BSONObj() ;
      _options.reset() ;
      PD_TRACE_EXIT( SDB__RTNALTERJOB_CLEAR ) ;
      return ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__RTNALTERJOB__EXTRACTOPTIONS, "_rtnAlterJob::_extractOptions" )
   void _rtnAlterJob::_extractOptions( const bson::BSONObj &obj )
   {
      PD_TRACE_ENTRY( SDB__RTNALTERJOB__EXTRACTOPTIONS ) ;
      _options.ignoreException = obj.getBoolField( FIELD_NAME_IGNORE_EXCEPTION ) ;
      PD_TRACE_EXIT( SDB__RTNALTERJOB__EXTRACTOPTIONS ) ;
      return ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__RTNALTERJOB__GETOBJTYPE, "_rtnAlterJob::_getObjType" )
   RTN_ALTER_TYPE _rtnAlterJob::_getObjType( const CHAR *name ) const
   {
      RTN_ALTER_TYPE type = RTN_ALTER_INVALID ;
      PD_TRACE_ENTRY( SDB__RTNALTERJOB__GETOBJTYPE ) ;
      string lower( name ) ;
      boost::algorithm::to_lower( lower ) ;
      if ( 0 == ossStrcmp( SDB_ALTER_CL, lower.c_str() ) )
      {
         type = RTN_ALTER_TYPE_CL ;
      }
      else
      {
         PD_LOG( PDERROR, "invalid type:%s", name ) ;
      }
      PD_TRACE_EXIT( SDB__RTNALTERJOB__GETOBJTYPE ) ;
      return type ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__RTNALTERJOB__VERIFYTASKS, "_rtnAlterJob::_verifyTasks" )
   INT32 _rtnAlterJob::_verifyTasks()
   {
      return SDB_OK ;
   }
}

