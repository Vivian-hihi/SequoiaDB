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

   Source File Name = coordRemoteHandle.cpp

   Descriptive Name =

   When/how to use:

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          03/17/2017  XJH Initial Draft

   Last Changed =

*******************************************************************************/

#include "coordRemoteHandle.hpp"

namespace engine
{

   /*
      _coordRemoteHandleBase implement
   */
   _coordRemoteHandleBase::_coordRemoteHandleBase()
   {
   }

   _coordRemoteHandleBase::~_coordRemoteHandleBase()
   {
   }

   INT32 _coordRemoteHandleBase::onSendFailed( _pmdRemoteSession *pSession,
                                               _pmdSubSession **ppSub,
                                               INT32 flag )
   {
      /// TODO:XUJIANHUI
      return SDB_OK ;
   }

   void _coordRemoteHandleBase::onReply( _pmdRemoteSession *pSession,
                                         _pmdSubSession **ppSub,
                                         const MsgHeader *pReply,
                                         BOOLEAN isPending )
   {
      /// TODO:XUJIANHUI
   }

   INT32 _coordRemoteHandleBase::onSendConnect( _pmdSubSession *pSub,
                                                const MsgHeader *pReq,
                                                BOOLEAN isFirst )
   {
      /// TODO:XUJIANHUI
      return SDB_OK ;
   }


}

