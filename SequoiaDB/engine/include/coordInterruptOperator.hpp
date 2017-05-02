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

   Source File Name = coordInterruptOperator.hpp

   Descriptive Name =

   When/how to use:

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          05/02/2017  XJH Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef COORD_INTERRUPT_OPERATOR_HPP__
#define COORD_INTERRUPT_OPERATOR_HPP__

#include "coordOperator.hpp"

namespace engine
{

   /*
      _coordInterruptHandler define
   */
   class _coordInterruptHandler : public _coordRemoteHandlerBase
   {
      public:
         _coordInterruptHandler() ;
         virtual ~_coordInterruptHandler() ;

      public:
         /*
            if return SDB_OK, will continue
            else, send failed
         */
         virtual INT32  onSendConnect( _pmdSubSession *pSub,
                                       const MsgHeader *pReq,
                                       BOOLEAN isFirst ) ;
   } ;
   typedef _coordInterruptHandler coordInterruptHandler ;

   /*
      _coordInterrupt define
   */
   class _coordInterrupt : public _coordOperator
   {
      public:
         _coordInterrupt() ;
         virtual ~_coordInterrupt() ;

         virtual INT32 execute( MsgHeader *pMsg,
                                pmdEDUCB *cb,
                                INT64 &contextID,
                                rtnContextBuf *buf ) ;
      private:
         void     _sendInterrupt( pmdRemoteSession *pSession,
                                  pmdEDUCB *cb,
                                  SET_ROUTEID &routeMap ) ;
   } ;
   typedef _coordInterrupt coordInterrupt ;

}

#endif // COORD_INTERRUPT_OPERATOR_HPP__
