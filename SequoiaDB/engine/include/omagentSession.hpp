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

   Source File Name = omagentSession.hpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          06/30/2014  TZB Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef OMAGENT_SESSION_HPP_
#define OMAGENT_SESSION_HPP_

#include "core.hpp"
#include "oss.hpp"

#include "pmdSession.hpp"
#include "omagent.hpp"

namespace engine
{
   // TODO: tanzhaobo
//   class _omaSession : public _pmdSession
   class _omaSession : public SDBObject
   {
      public:
         _omaSession ( SOCKET fd ) ;
         virtual ~_omaSession () ;

         virtual UINT64 identifyID() { return (UINT64)SDB_OK ; }
         virtual INT32 getServiceType() const { return SDB_OK ; }
         virtual SDB_SESSION_TYPE sessionType() const ;

         virtual void clear() {}
         virtual const CHAR* sessionName() const { return "" ; }

         INT32 run () ;

         INT32 _processMsg ( MsgHeader *msg ) ;

         INT32 _processOPMsg ( MsgHeader *msg, CHAR **ppBody,
                               INT32 &bodyLen, INT32 &returnNum ) ;

         INT32 _reply ( MsgOpReply* responseMsg, const CHAR *pBody,
                        INT32 bodyLen ) ;

         INT32 _buildReplyHeader( MsgHeader *msg ) ;

         INT32 _onQueryReqMsg( MsgHeader *msg, CHAR **ppBody,
                               INT32 &bodyLen, INT32 &returnNum ) ;

      private:
         MsgOpReply       _replyHeader ;
         BSONObj          _errorInfo ;

         BOOLEAN          _needRollback ;
         BOOLEAN          _needReply ;
   } ;

   typedef _omaSession omaSession ;
}



#endif // OMAGENT_SESSION_HPP_
