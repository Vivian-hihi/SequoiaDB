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
#include "omManager.hpp"

namespace engine
{

   /*
      _pmdRestSession implement
   */
   _pmdRestSession::_pmdRestSession( SOCKET fd )
   :_pmdLocalSession( fd )
   {
      _pFixBuff         = NULL ;
      _loginTime        = 0 ;
   }

   _pmdRestSession::~_pmdRestSession()
   {
      if ( _pFixBuff )
      {
         sdbGetOMManager()->releaseFixBuf( _pFixBuff ) ;
         _pFixBuff = NULL ;
      }
   }

   UINT64 _pmdRestSession::identifyID()
   {
      // TODO:XUJIANHUI
      return 0 ;
   }

   INT32 _pmdRestSession::_onAuth( MsgHeader * msg )
   {
      // TODO:XUJIANHUI
      return SDB_OK ;
   }

   INT32 _pmdRestSession::getFixBuffSize() const
   {
      return sdbGetOMManager()->getFixBufSize() ;
   }

   CHAR* _pmdRestSession::getFixBuff ()
   {
      if ( !_pFixBuff )
      {
         _pFixBuff = sdbGetOMManager()->allocFixBuf() ;
      }
      return _pFixBuff ;
   }

   void _pmdRestSession::restoreSession( restSessionInfo &sessionInfo )
   {
      _authOK     = sessionInfo._authOK ;
      _loginTime  = sessionInfo._attr._loginTime ;
      _userName   = sessionInfo._attr._userName ;

      //sessionInfo._isIn = 
   }

   void _pmdRestSession::saveSession( restSessionInfo &sessionInfo )
   {
   }

}

