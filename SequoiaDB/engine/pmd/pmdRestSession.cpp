/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = pmdRestSession.cpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          04/14/2014  Xu Jianhui  Initial Draft

   Last Changed =

*******************************************************************************/

#include "pmdRestSession.hpp"

namespace engine
{

   // message map
   BEGIN_OBJ_MSG_MAP(_pmdRestSession, _pmdLocalSession)

   END_OBJ_MSG_MAP()

   _pmdRestSession::_pmdRestSession( SOCKET fd )
   :_pmdLocalSession( fd )
   {
   }

   _pmdRestSession::~_pmdRestSession()
   {
   }

   UINT64 _pmdRestSession::identifyID()
   {
      return 0 ;
   }

   INT32 _pmdRestSession::_defaultMsgFunc( NET_HANDLE handle, MsgHeader *msg )
   {
      return SDB_OK ;
   }

   INT32 _pmdRestSession::_onAuth( MsgHeader * msg )
   {
      return SDB_OK ;
   }

   INT32 _pmdRestSession::_onOPMsg( NET_HANDLE handle, MsgHeader * msg )
   {
      return SDB_OK ;
   }
}

