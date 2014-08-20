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

   Source File Name = sptUsrOmaAssit.cpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          18/08/2014  XJH Initial Draft

   Last Changed =

*******************************************************************************/

#include "sptUsrOmaAssit.hpp"
#include "client.h"

namespace engine
{

   /*
      _sptUsrOmaAssit implement
   */
   _sptUsrOmaAssit::_sptUsrOmaAssit()
   {
      _handle  = 0 ;
   }

   _sptUsrOmaAssit::~_sptUsrOmaAssit()
   {
   }

   INT32 _sptUsrOmaAssit::disconnect()
   {
      if ( 0 != _handle )
      {
         sdbDisconnect( _handle ) ;
         _handle = 0 ;
      }
      return SDB_OK ;
   }

   INT32 _sptUsrOmaAssit::connect( const CHAR * pHostName,
                                   const CHAR * pServiceName )
   {
      return sdbConnect( pHostName, pServiceName, "", "", &_handle ) ;
   }

   INT32 _sptUsrOmaAssit::syncSend( const CHAR * pBuff, INT32 buffLen,
                                    CHAR ** ppRecv, INT32 & recvLen )
   {
      // TODO:XUJIANHUI
      return SDB_OK ;
   }

}

