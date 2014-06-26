#ifndef OMAGENT_SESSION_HPP_
#define OMAGENT_SESSION_HPP_

#include "core.hpp"
#include "oss.hpp"

#include "pmdSession.hpp"



namespace CLSMGR
{

   class _omagentSession : public _pmdSession
   {
      public:
         _omagentSession ( SOCKET fd ) ;
         virtual ~_omagentSession () ;

         INT32 run () ;

         INT32 _processMsg ( MsgHeader *msg ) ;

         INT32 _processOPMsg ( MsgHeader *msg, const CHAR **ppBody,
                               INT32 &bodyLen, INT32 &returnNum ) ;

         INT32 _reply ( MsgOpReply* responseMsg, const CHAR *pBody,
                        INT32 bodyLen ) ;

         INT32 _buildReplyHeader( MsgHeader *msg ) ;

         INT32 _onQueryReqMsg( MsgHeader *msg, omagentObjBuff &objBuff ) ;

      private:
         MsgOpReply       _replyHeader ;
         omagentObjBuff   _objBuff ;
         BSONObj          _errorInfo ;

         BOOLEAN          _needRollback ;
         BOOLEAN          _needReply ;
   } ;


}



#endif // OMAGENT_SESSION_HPP_
