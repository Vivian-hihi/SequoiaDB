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

   Source File Name = pmdRemoteSession.cpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          06/05/2014  Xu Jianhui  Initial Draft

   Last Changed =

*******************************************************************************/

#include "pmdRemoteSession.hpp"


#include "../bson/bson.h"

using namespace bson ;

namespace engine
{
   /*
      pmdSubSession implement
   */
   _pmdSubSession::_pmdSubSession()
   {
      _nodeID     = 0 ;
      _reqID      = 0 ;
      _pReqMsg    = NULL ;
      _pRspMsg    = NULL ;
   }

   _pmdSubSession::~_pmdSubSession()
   {
   }

   /*
      _pmdRemoteSession implement
   */
   _pmdRemoteSession::_pmdRemoteSession( netRouteAgent *pAgent )
   {
      _pAgent = pAgent ;
   }

   _pmdRemoteSession::~_pmdRemoteSession()
   {
      _pAgent = NULL ;
   }

   pmdSubSession* _pmdRemoteSession::addCurSubSession( UINT64 nodeID )
   {
      pmdSubSession &subSession = _mapSubSession[ nodeID ] ;
      if ( subSession.getNodeID() != nodeID )
      {
         subSession.setNodeID( nodeID ) ;
      }
      _mapCurSubSession[ nodeID ] = &subSession ;
      return &subSession ;
   }

}

