/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = sptSshSession.hpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          31/03/2014  YW  Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef SPT_SSHSESSION_HPP_
#define SPT_SSHSESSION_HPP_

#include "core.hpp"
#include "oss.hpp"
#include "ossSocket.hpp"
#include <string>

namespace engine
{
   enum SPT_CP_PROTOCOL
   {
      SPT_CP_PROTOCOL_SCP = 0,
      SPT_CP_PROTOCOL_FTP,
      SPT_CP_PROTOCOL_SFTP,
   } ;

   class _sptSshSession : public SDBObject
   {
   public:
      _sptSshSession( const CHAR *host,
                      const CHAR *usrname,
                      const CHAR *passwd ) ;
      virtual ~_sptSshSession() ;

   public:
      INT32 open() ;

      virtual INT32 exec( const CHAR *cmd ) = 0 ;

      virtual INT32 read( CHAR *buf, UINT32 len, UINT32 &readSize ) = 0 ;

      virtual void execDone() = 0 ;

      virtual INT32 copy2Remote( SPT_CP_PROTOCOL protocol,
                                 const CHAR *local,   /// full path
                                 const CHAR *dst,   /// full path
                                 INT32 mode ) = 0 ;

      virtual INT32 copyFromRemote( SPT_CP_PROTOCOL protocol,
                                    const CHAR *remote,
                                    const CHAR *local,
                                    INT32 mode ) = 0 ;

      virtual void getLastError( std::string &errMsg ) = 0 ;

   private:
      virtual INT32 _openSshSession() = 0 ;

   protected:
      std::string _host ;
      std::string _usr ;
      std::string _passwd ;
      _ossSocket *_sock ;
   } ;
   typedef class _sptSshSession sptSshSession ;
}

#endif

