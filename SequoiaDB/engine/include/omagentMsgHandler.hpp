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

   Source File Name = omagentMsgHandler.hpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          10/06/2014  Tan Zhaobo  Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef OMAGENT_MSG_HANDLER_HPP_
#define OMAGENT_MSG_HANDLER_HPP_

#include "netDef.hpp"
#include "netRouteAgent.hpp"
#include "netMsgHandler.hpp"

using namespace engine;

namespace CLSMGR
{
   class _omagentMsgHandler : public _netMsgHandler
   {
      public:
         _omagentMsgHandler () ;
         virtual ~_omagentMsgHandler () ;

         virtual INT32 handleMsg ( const NET_HANDLE &handle,
                                   const _MsgHeader *header,
                                   const CHAR *msg ) ;
         virtual INT32 handleClose ( const NET_HANDLE *handle,
                                     _MsgRouteID id ) ;
   };
}


#endif // OMAGENT_MSG_HANDLER_HPP_
