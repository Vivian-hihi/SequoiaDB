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

   Source File Name = sptUsrRemoteAssit.hpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          18/07/2016  WJM Initial Draft

   Last Changed =

*******************************************************************************/


#ifndef SPT_USRREMOTE_ASSIT_HPP__
#define SPT_USRREMOTE_ASSIT_HPP__

#include "msg.h"
#include "oss.hpp"


namespace engine
{
   /*
      sptUsrRemoteAssit define
   */
   class _sptUsrRemoteAssit : public SDBObject
   {
      public:
         _sptUsrRemoteAssit() ;

         ~_sptUsrRemoteAssit() ;

      public:
         INT32       connect( const CHAR *pHostName,
                              const CHAR *pServiceName ) ;

         INT32       disconnect() ;

         INT32       runCommand( const CHAR *pString,
                                 SINT32 flag,
                                 UINT64 reqID,
                                 SINT64 numToSkip,
                                 SINT64 numToReturn,
                                 const CHAR *arg1,
                                 const CHAR *arg2,
                                 const CHAR *arg3,
                                 const CHAR *arg4,
                                 CHAR **ppRetBuffer ) ;

      private:
         INT32       _sendAndRecv( const MsgHeader *sendMsg,
                                   MsgHeader **recvMsg, INT32 *size,
                                   BOOLEAN needRecv,
                                   BOOLEAN endianConvert ) ;

         INT32       _sendMsg( const MsgHeader *msg, BOOLEAN endianConvert ) ;

         INT32       _recvMsg( MsgHeader **msg, INT32 *msgLength,
                               BOOLEAN endianConvert ) ;

         INT32       _send( const CHAR *pMsg, INT32 msgLength ) ;

         INT32       _reallocBuffer( CHAR **ppBuffer, INT32 *bufferSize,
                                     INT32 newSize ) ;

         INT32       _extract( MsgHeader *msg, INT32 msgSize,
                               SINT64 *contextID,
                               BOOLEAN *result, BOOLEAN endianConvert ) ;

         INT32       _getRetBuffer( CHAR *pRetMsg, CHAR **ppRetBuffer ) ;

      private:
         ossValuePtr _handle ;

   } ;
   typedef _sptUsrRemoteAssit sptUsrRemoteAssit ;
}
#endif