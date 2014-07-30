#ifndef OMAGENT_SESSION_HPP_
#define OMAGENT_SESSION_HPP_

#include "core.hpp"
#include "oss.hpp"

#include "pmdSession.hpp"
#include "omagent.hpp"

using namespace engine ;

namespace CLSMGR
{

//   class _omagentSession : public _pmdSession
   class _omagentSession : public SDBObject
   {
      public:
         _omagentSession ( SOCKET fd ) ;
         virtual ~_omagentSession () ;

         virtual UINT64 identifyID() { return (UINT64)SDB_OK ; }
         virtual INT32 getServiceType() const { return SDB_OK ; }

         virtual void clear() {}
         virtual const CHAR* sessionName() const { return "" ; }

         INT32 run () ;

         INT32 _processMsg ( MsgHeader *msg ) ;

         INT32 _processOPMsg ( MsgHeader *msg, CHAR **ppBody,
                               INT32 &bodyLen, INT32 &returnNum ) ;

//         INT32 _processOPMsg ( MsgHeader *msg, omagentObjBuff &objBuff ) ;

         INT32 _reply ( MsgOpReply* responseMsg, const CHAR *pBody,
                        INT32 bodyLen ) ;

         INT32 _buildReplyHeader( MsgHeader *msg ) ;

//         INT32 _onQueryReqMsg( MsgHeader *msg, omagentObjBuff &objBuff ) ;
         INT32 _onQueryReqMsg( MsgHeader *msg, CHAR **ppBody,
                               INT32 &bodyLen, INT32 &returnNum ) ;

      private:
         MsgOpReply       _replyHeader ;
//         omagentObjBuff   _objBuff ;
         BSONObj          _errorInfo ;

         BOOLEAN          _needRollback ;
         BOOLEAN          _needReply ;
   } ;

   typedef _omagentSession omagentSession ;
}



#endif // OMAGENT_SESSION_HPP_
