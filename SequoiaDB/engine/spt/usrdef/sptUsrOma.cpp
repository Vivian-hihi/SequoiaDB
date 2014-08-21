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

   Source File Name = sptUsrOma.cpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          18/08/2014  XJH Initial Draft

   Last Changed =

*******************************************************************************/

#include "sptUsrOma.hpp"
#include "omagentDef.hpp"
#include "ossUtil.hpp"
#include "msgDef.h"
#include "../bson/bsonobj.h"

using namespace bson ;

namespace engine
{
   /*
      Function Define
   */
   JS_CONSTRUCT_FUNC_DEFINE( _sptUsrOma, construct )
   JS_DESTRUCT_FUNC_DEFINE( _sptUsrOma, destruct )
   JS_MEMBER_FUNC_DEFINE(_sptUsrOma, toString)
   JS_MEMBER_FUNC_DEFINE(_sptUsrOma, createCoord)
   JS_MEMBER_FUNC_DEFINE(_sptUsrOma, removeCoord)
   JS_MEMBER_FUNC_DEFINE(_sptUsrOma, startNode)
   JS_MEMBER_FUNC_DEFINE(_sptUsrOma, stopNode)
   JS_MEMBER_FUNC_DEFINE(_sptUsrOma, close)
   JS_STATIC_FUNC_DEFINE(_sptUsrOma, help)

   /*
      Function Map
   */
   JS_BEGIN_MAPPING( _sptUsrOma, "Oma" )
      JS_ADD_CONSTRUCT_FUNC( construct )
      JS_ADD_DESTRUCT_FUNC(destruct)
      JS_ADD_MEMBER_FUNC("toString", toString)
      JS_ADD_MEMBER_FUNC("createCoord", createCoord)
      JS_ADD_MEMBER_FUNC("removeCoord", removeCoord)
      JS_ADD_MEMBER_FUNC("startNode", startNode)
      JS_ADD_MEMBER_FUNC("stopNode", stopNode)
      JS_ADD_MEMBER_FUNC("close", close)
      JS_ADD_STATIC_FUNC("help", help)
   JS_MAPPING_END()

   /*
      _sptUsrOma Implement
   */
   _sptUsrOma::_sptUsrOma()
   {
      CHAR tmpName[ 10 ] = { 0 } ;
      ossSnprintf( tmpName, sizeof( tmpName ) - 1, "%u", SDBCM_DFT_PORT ) ;
      _hostname = "localhost" ;
      _svcname = tmpName ;
   }

   _sptUsrOma::~_sptUsrOma()
   {
   }

   INT32 _sptUsrOma::construct( const _sptArguments & arg,
                                _sptReturnVal & rval,
                                BSONObj & detail )
   {
      INT32 rc = SDB_OK ;

      if ( arg.argc() >= 1 )
      {
         rc = arg.getString( 0, _hostname ) ;
         if ( rc )
         {
            detail = BSON( SPT_ERR << "hostname must be string" ) ;
         }
         PD_RC_CHECK( rc, PDERROR, "Failed to get hostname, rc: %d", rc ) ;
      }
      if ( arg.argc() >= 2 )
      {
         rc = arg.getString( 1, _svcname ) ;
         if ( rc )
         {
            detail = BSON( SPT_ERR << "svcname must be string" ) ;
         }
         PD_RC_CHECK( rc, PDERROR, "Failed to get svcname, rc: %d", rc ) ;
      }

      rc = _assit.connect( _hostname.c_str(), _svcname.c_str() ) ;
      PD_RC_CHECK( rc, PDERROR, "Failed to connect %s:%s, rc: %d",
                   _hostname.c_str(), _svcname.c_str(), rc ) ;

      rval.setUsrObjectVal( "", this, SPT_CLASS_DEF( this ) ) ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _sptUsrOma::toString( const _sptArguments & arg,
                               _sptReturnVal & rval,
                               BSONObj & detail )
   {
      string name = _hostname ;
      name += ":" ;
      name += _svcname ;
      rval.setStringVal( "", name.c_str() ) ;
      return SDB_OK ;
   }

   INT32 _sptUsrOma::help( const _sptArguments & arg,
                           _sptReturnVal & rval,
                           BSONObj & detail )
   {
      stringstream ss ;
      ss << "Oma functions:" << endl
         << "   createCoord( svcname, dbpath, [config obj])" << endl
         << "   removeCoord( svcname )" << endl
         << "   startNode( svcname )" << endl
         << "   stopNode( svcname )" << endl
         << "   close()" << endl ;
      rval.setStringVal( "", ss.str().c_str() ) ;
      return SDB_OK ;
   }

   INT32 _sptUsrOma::destruct()
   {
      return _assit.disconnect() ;
   }

   INT32 _sptUsrOma::createCoord( const _sptArguments & arg,
                                  _sptReturnVal & rval,
                                  BSONObj & detail )
   {
      INT32 rc = SDB_OK ;
      string svcname ;
      string dbpath ;
      BSONObj config ;

      rc = arg.getString( 0, svcname ) ;
      if ( SDB_OUT_OF_BOUND == rc )
      {
         detail = BSON( SPT_ERR << "svcname must be config" ) ;
      }
      else if ( rc )
      {
         detail = BSON( SPT_ERR << "svcname must be string" ) ;
      }
      PD_RC_CHECK( rc, PDERROR, "Failed to get svcname, rc: %d", rc ) ;

      rc = arg.getString( 1, dbpath ) ;
      if ( SDB_OUT_OF_BOUND == rc )
      {
         detail = BSON( SPT_ERR << "dbpath must be config" ) ;
      }
      else if ( rc )
      {
         detail = BSON( SPT_ERR << "dbpath must be string" ) ;
      }
      PD_RC_CHECK( rc, PDERROR, "Failed to get dbpath, rc: %d", rc ) ;

      if ( arg.argc() >= 3 )
      {
         rc = arg.getBsonobj( 2, config ) ;
         if ( rc )
         {
            detail = BSON( SPT_ERR << "config must be object" ) ;
         }
         PD_RC_CHECK( rc, PDERROR, "Failed to get config, rc: %d", rc ) ;
      }

      rc = _assit.createCoord( svcname.c_str(), dbpath.c_str(),
                               config.objdata() ) ;
      PD_RC_CHECK( rc, PDERROR, "Failed to create coord[%s], rc: %d",
                   svcname.c_str(), rc ) ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _sptUsrOma::removeCoord( const _sptArguments & arg,
                                  _sptReturnVal & rval,
                                  BSONObj & detail )
   {
      INT32 rc = SDB_OK ;
      string svcname ;

      rc = arg.getString( 0, svcname ) ;
      if ( SDB_OUT_OF_BOUND == rc )
      {
         detail = BSON( SPT_ERR << "svcname must be config" ) ;
      }
      else if ( rc )
      {
         detail = BSON( SPT_ERR << "svcname must be string" ) ;
      }
      PD_RC_CHECK( rc, PDERROR, "Failed to get svcname, rc: %d", rc ) ;

      rc = _assit.removeCoord( svcname.c_str() ) ;
      PD_RC_CHECK( rc, PDERROR, "Failed to remove coord[%s], rc: %d",
                   svcname.c_str(), rc ) ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _sptUsrOma::startNode( const _sptArguments & arg,
                                _sptReturnVal & rval,
                                BSONObj & detail )
   {
      INT32 rc = SDB_OK ;
      string svcname ;

      rc = arg.getString( 0, svcname ) ;
      if ( SDB_OUT_OF_BOUND == rc )
      {
         detail = BSON( SPT_ERR << "svcname must be config" ) ;
      }
      else if ( rc )
      {
         detail = BSON( SPT_ERR << "svcname must be string" ) ;
      }
      PD_RC_CHECK( rc, PDERROR, "Failed to get svcname, rc: %d", rc ) ;

      rc = _assit.startNode( svcname.c_str() ) ;
      PD_RC_CHECK( rc, PDERROR, "Start node[%s] failed, rc: %d",
                   svcname.c_str(), rc ) ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _sptUsrOma::stopNode( const _sptArguments & arg,
                               _sptReturnVal & rval,
                               BSONObj & detail )
   {
      INT32 rc = SDB_OK ;
      string svcname ;

      rc = arg.getString( 0, svcname ) ;
      if ( SDB_OUT_OF_BOUND == rc )
      {
         detail = BSON( SPT_ERR << "svcname must be config" ) ;
      }
      else if ( rc )
      {
         detail = BSON( SPT_ERR << "svcname must be string" ) ;
      }
      PD_RC_CHECK( rc, PDERROR, "Failed to get svcname, rc: %d", rc ) ;

      rc = _assit.stopNode( svcname.c_str() ) ;
      PD_RC_CHECK( rc, PDERROR, "Stop node[%s] failed, rc: %d",
                   svcname.c_str(), rc ) ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _sptUsrOma::close( const _sptArguments & arg,
                            _sptReturnVal & rval,
                            BSONObj & detail )
   {
      return _assit.disconnect() ;
   }

}


