/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = sptSshSession.cpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          31/03/2014  YW  Initial Draft

   Last Changed =

*******************************************************************************/

#include "sptSshSession.hpp"
#include "pd.hpp"

#define SPT_SSH_PORT 22

namespace engine
{
   _sptSshSession::_sptSshSession( const CHAR *host,
                                   const CHAR *usrname,
                                   const CHAR *passwd  )
   :_sock( NULL )
   {
      SDB_ASSERT( NULL != host && NULL != usrname && NULL != passwd,
                  "can not be null" )
      _host.assign( host ) ;
      _usr.assign( usrname ) ;
      _passwd.assign( passwd ) ;
      _sock = SDB_OSS_NEW _ossSocket( _host.c_str(),
                                      SPT_SSH_PORT );
   }

   _sptSshSession::~_sptSshSession()
   {
      if ( NULL != _sock )
      {
         _sock->close() ;
         SAFE_OSS_DELETE( _sock ) ;
      }
   }

   INT32 _sptSshSession::open()
   {
      INT32 rc = SDB_OK ;
      
      _sock = SDB_OSS_NEW _ossSocket( _host.c_str(), SPT_SSH_PORT ) ;
      if ( NULL == _sock )
      {
         PD_LOG( PDERROR, "failed to allocate mem." ) ;
         rc = SDB_OOM ;
         goto error ;
      }

      rc = _sock->initSocket() ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to init socket:%d", rc ) ;
         goto error ;
      }

      rc = _sock->connect() ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "can not connect to host:%s, rc:%d", _host.c_str(), rc ) ;
         goto error ;
      }

      rc = _openSshSession() ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to open ssh session:%d", rc ) ;
         goto error ;
      }
   done:
      return rc ;
   error:
      if ( NULL != _sock )
      {
         _sock->close() ;
         SAFE_OSS_DELETE( _sock ) ;
      }
      goto done ;
   }
}

