/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = sptUsrSsh.cpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          31/03/2014  YW  Initial Draft

   Last Changed =

*******************************************************************************/

#include "sptUsrSsh.hpp"
#include "sptLibssh2Session.hpp"
#include "pd.hpp"
#include "ossMem.hpp"
#include <string>

using namespace bson ;
using namespace std ;

namespace engine
{
JS_MEMBER_FUNC_DEFINE( _sptUsrSsh, exec )
JS_MEMBER_FUNC_DEFINE( _sptUsrSsh, copy2Remote )
JS_MEMBER_FUNC_DEFINE( _sptUsrSsh, copyFromRemote )
JS_CONSTRUCT_FUNC_DEFINE( _sptUsrSsh, construct )
JS_DESTRUCT_FUNC_DEFINE( _sptUsrSsh, destruct )

JS_BEGIN_MAPPING( _sptUsrSsh, "Ssh" )
   JS_ADD_MEMBER_FUNC( "exec", exec )
   JS_ADD_MEMBER_FUNC( "push", copy2Remote )
   JS_ADD_MEMBER_FUNC( "pull", copyFromRemote )
   JS_ADD_CONSTRUCT_FUNC( construct )
   JS_ADD_DESTRUCT_FUNC( destruct )
JS_MAPPING_END()

   _sptUsrSsh::_sptUsrSsh()
   :_session( NULL )
   {

   }

   _sptUsrSsh::~_sptUsrSsh()
   {
      SAFE_OSS_DELETE( _session ) ;
   }

   INT32 _sptUsrSsh::construct( const _sptArguments &arg,
                                _sptReturnVal &rval,
                                bson::BSONObj &detail)
   {
      INT32 rc = SDB_OK ;
      string host ;
      string usr ;
      string passwd ;

      rc = arg.getString( 0, host ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to get host from arg" ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }

      rc = arg.getString( 1, usr ) ;
      if ( SDB_OK != rc && SDB_OUT_OF_BOUND != rc )
      {
         PD_LOG( PDERROR, "failed to get usr from arg" ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }

      rc = arg.getString( 2, passwd ) ;
      if ( SDB_OK != rc && SDB_OUT_OF_BOUND != rc )
      {
         PD_LOG( PDERROR, "failed to get passwd from arg" ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }

      _session = SDB_OSS_NEW _sptLibssh2Session( host.c_str(),
                                                usr.c_str(),
                                                passwd.c_str() ) ;
      if ( NULL == _session )
      {
         PD_LOG( PDERROR, "failed to allocate mem." ) ;
         rc = SDB_OOM ;
         goto error ;
      }

      rc = _session->open() ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to open ssh session:%d", rc ) ;
         goto error ;
      }

      rval.setUsrObjectVal( "", this, SPT_CLASS_DEF( this ) ) ;
   done:
      return rc ;
   error:
      SAFE_OSS_DELETE( _session ) ;
      detail = BSON( SPT_ERR << "new Ssh(); false" ) ;
      goto done ;      
   }

   INT32 _sptUsrSsh::destruct()
   {
      SAFE_OSS_DELETE( _session ) ;
      return SDB_OK ;
   }

   INT32 _sptUsrSsh::copy2Remote( const _sptArguments &arg,
                                  _sptReturnVal &rval,
                                  bson::BSONObj &detail )
   {
      INT32 rc = SDB_OK ;
      string local ;
      string dst ;
      INT32 mode = 0755 ;
      
      string errMsg ;

      rc = arg.getString( 0, local ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to get local file" ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }

      rc = arg.getString( 1, dst ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to get dst file" ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }

      rc = arg.getNative( 2, &mode ) ;
      if ( SDB_OK != rc && SDB_OUT_OF_BOUND != rc )
      {
         PD_LOG( PDERROR, "failed to get mode" ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }

      rc = _session->copy2Remote( SPT_CP_PROTOCOL_SCP,
                                  local.c_str(),
                                  dst.c_str(),
                                  mode ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to copy file:%d", rc ) ;
         _session->getLastError( errMsg ) ;
         goto error ;
      }

      
   done:
      return rc ;
   error:
      if ( !errMsg.empty() )
      {
         detail = BSON( SPT_ERR << errMsg ) ;
      }
      goto done ;
   }

   INT32 _sptUsrSsh::copyFromRemote( const _sptArguments &arg,
                                     _sptReturnVal &rval,
                                     bson::BSONObj &detail )
   {
      INT32 rc = SDB_OK ;
      string remote ;
      string local ;
      INT32 mode = 0 ;
      string errMsg ;

      rc = arg.getString( 0, remote ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to get local file" ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }

      rc = arg.getString( 1, local ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to get dst file" ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }

      rc = arg.getNative( 2, &mode ) ;
      if ( SDB_OK != rc && SDB_OUT_OF_BOUND != rc )
      {
         PD_LOG( PDERROR, "failed to get mode" ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }

      rc = _session->copyFromRemote( SPT_CP_PROTOCOL_SCP,
                                     remote.c_str(),
                                     local.c_str(),
                                     mode ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to copy file:%d", rc ) ;
         _session->getLastError( errMsg ) ;
         goto error ;
      }
   done:
      return rc ;
   error:
      if ( !errMsg.empty() )
      {
         detail = BSON( SPT_ERR << errMsg ) ;
      }
      goto done ;
   }

   INT32 _sptUsrSsh::exec( const _sptArguments &arg,
                           _sptReturnVal &rval,
                           bson::BSONObj &detail )
   {
      INT32 rc = SDB_OK ;
      SDB_ASSERT( NULL != _session, "can not be null" )
      INT32 exit = 0 ;
      string cmd ;
      string errMsg ;
      string sig ;
      
      rc = arg.getString( 0, cmd ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "exec should have one argument at least" ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }

      {
#define READ_LEN 8192
      CHAR buf[READ_LEN + 1] ;
      UINT32 read = 0 ;

      rc = _session->exec( cmd.c_str(), exit, buf, READ_LEN, read ) ;
      buf[read] = '\0' ;
      if ( SDB_OK != rc || SDB_OK != exit )
      {
         PD_LOG( PDERROR, "failed to read data from session:%d", rc ) ;
         rc = SDB_SPT_EVAL_FAIL ;
         if ( 0 == read )
         {
            _session->getLastError( errMsg ) ;
         }
         else
         {
            errMsg.assign( buf ) ;
         }
         goto error ;
      }

      rc = rval.setStringVal( "", buf ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to set string to return val." ) ;
         goto error ;
      }
      }

   done:
      return rc ;
   error:
      if ( !errMsg.empty() )
      {
         detail = BSON( SPT_ERR << errMsg ) ;
      }
      goto done ;
   }

}

