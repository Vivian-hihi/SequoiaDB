/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = sptLibssh2Session.hpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          31/03/2014  YW  Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef SPT_LIBSSH2SESSION_HPP_
#define SPT_LIBSSH2SESSION_HPP_

#include "sptSshSession.hpp"
#include "libssh2.h"

class _OSS_FILE ;
namespace engine
{
   class _sptLibssh2Session : public _sptSshSession
   {
   public:
      _sptLibssh2Session( const CHAR *host,
                          const CHAR *usrname,
                          const CHAR *passwd ) ;
      virtual ~_sptLibssh2Session() ;

   public:
      virtual INT32 exec( const CHAR *cmd ) ;

      virtual INT32 read( CHAR *buf, UINT32 len, UINT32 &readSize ) ;

      virtual void execDone() ;

      virtual INT32 copy2Remote( SPT_CP_PROTOCOL protocol,
                                 const CHAR *local,
                                 const CHAR *dst,
                                 INT32 mode ) ;

      virtual INT32 copyFromRemote( SPT_CP_PROTOCOL protocol,
                                    const CHAR *remote,
                                    const CHAR *local,
                                    INT32 mode) ;

      virtual void getLastError( std::string &errMsg ) ;

   private:
      virtual INT32 _openSshSession() ;

      void _clearChannel() ;

      void _clearSession() ;

      INT32 _scpSend( const CHAR *local, const CHAR *dst, INT32 mode ) ;

      INT32 _scpRecv( const CHAR *remote, const CHAR *local, INT32 mode ) ;

      INT32 _write2Channel( LIBSSH2_CHANNEL *channel,
                            const CHAR *buf, SINT64 len ) ;

      INT32 _readFromChannel( LIBSSH2_CHANNEL *channel,
                              SINT64 len,
                              CHAR *buf ) ;

      INT32 _wirte2File( _OSS_FILE *file, const CHAR *buf, SINT64 len ) ;
   private:
      LIBSSH2_SESSION *_session ;
      LIBSSH2_CHANNEL *_channel ;
   } ;
   typedef class _sptLibssh2Session sptLibssh2Session ;
}

#endif

