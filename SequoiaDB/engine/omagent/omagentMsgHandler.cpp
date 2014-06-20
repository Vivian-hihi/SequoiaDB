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

   Source File Name = omagentMsgHandler.cpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          10/06/2012  Tan Zhaobo  Initial Draft

   Last Changed =

*******************************************************************************/

#include "omagentMsgHandler.hpp"
#include "omagent.hpp"

using namespace std;
namespace CLSMGR
{
   _omagentMsgHandler::_omagentMsgHandler ()
   {

   }

   _omagentMsgHandler::~_omagentMsgHandler ()
   {

   }

   INT32 _omagentMsgHandler::handleMsg ( const NET_HANDLE &handle,
                                         const _MsgHeader *header,
                                         const CHAR *msg )
   {
      INT32 rc = SDB_OK ;
      cout << "OK! In handleMsg." << endl ;
   done:
      return rc ;
   error:
      goto done;
   }

   INT32 _omagentMsgHandler::handleClose ( const NET_HANDLE *handle,
                                           _MsgRouteID id )
   {
      INT32 rc = SDB_OK ;
      cout << "OK! In handleClose." << endl ;
   done:
      return rc ;
   error:
      goto done;
   }

}
