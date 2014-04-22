/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = pmdSession.hpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          04/04/2014  Xu Jianhui  Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef PMD_SESSION_HPP_
#define PMD_SESSION_HPP_

#include "core.hpp"
#include "oss.hpp"
#include "ossSocket.hpp"
#include "sdbInterface.hpp"
#include "msg.h"
#include "pmdDef.hpp"

#include <map>
#include <string>
#include "../bson/bson.h"

using namespace bson ;

namespace engine
{

   class _pmdEDUCB ;

   /*
      _pmdSession define
   */
   class _pmdSession : public _ISession
   {
      typedef std::multimap<INT32,CHAR*>     CATCH_MAP ;
      typedef CATCH_MAP::iterator            CATCH_MAP_IT ;

      public:
         _pmdSession( SOCKET fd ) ;
         virtual ~_pmdSession() ;

         virtual void            clear() ;

         virtual const CHAR*     sessionName() const ;

      public:
         UINT64      sessionID () const { return _eduID ; }
         EDUID       eduID () const { return _eduID ; }
         _pmdEDUCB*  eduCB () const { return _pEDUCB ; }
         ossSocket*  socket () { return &_socket ; }

         void        attach( _pmdEDUCB * cb ) ;
         void        detach() ;

         CHAR*       getBuff( INT32 len ) ;
         INT32       getBuffLen () const { return _buffLen ; }

         INT32       allocBuff( INT32 len, CHAR **ppBuff, INT32 &buffLen ) ;
         void        releaseBuff( CHAR *pBuff, INT32 buffLen ) ;

         void        disconnect() ;
         INT32       sendData( const CHAR *pData, INT32 size,
                               INT32 timeout = -1,
                               BOOLEAN block = TRUE,
                               INT32 *pSentLen = NULL,
                               INT32 flags = 0 ) ;
         INT32       recvData( CHAR *pData, INT32 size,
                               INT32 timeout = -1,
                               BOOLEAN block = TRUE,
                               INT32 *pRecvLen = NULL,
                               INT32 flags = 0 ) ;

      protected:

         BOOLEAN     _allocFromCatch( INT32 len, CHAR **ppBuff,
                                      INT32 &buffLen ) ;

      protected:

         _pmdEDUCB                        *_pEDUCB ;
         EDUID                            _eduID ;
         ossSocket                        _socket ;
         std::string                      _sessionName ;

      protected:
         CHAR                             *_pBuff ;
         INT32                            _buffLen ;

         CATCH_MAP                        _catchMap ;
         INT64                            _totalCatchSize ;
         INT64                            _totalMemSize ;

   } ;
   typedef _pmdSession pmdSession ;

   class _SDB_DMSCB ;
   class _dpsLogWrapper ;
   class _SDB_RTNCB ;
   class _rtnContextBuf ;

   /*
      _pmdLocalSession define
   */
   class _pmdLocalSession : public _pmdSession
   {
      public:
         _pmdLocalSession( SOCKET fd ) ;
         virtual ~_pmdLocalSession () ;

         virtual INT32     sessionType() const { return PMD_SESSION_LOCAL ; }
         virtual UINT64    identifyID() ;

         virtual INT32     run() ;

      protected:
         INT32          _processMsg( MsgHeader *msg ) ;
         virtual INT32  _onMsgBegin( MsgHeader *msg ) ;
         virtual void   _onMsgEnd( INT32 result, MsgHeader *msg ) ;

         virtual INT32  _processOPMsg( MsgHeader *msg, INT64 &contextID,
                                       const CHAR **ppBody, INT32 &bodyLen,
                                       INT32 &returnNum ) ;

         virtual INT32  _onAuth( MsgHeader *msg ) ;

         INT32          _recvSysInfoMsg( UINT32 msgSize, CHAR **ppBuff,
                                         INT32 &buffLen ) ;
         INT32          _processSysInfoRequest( const CHAR *msg ) ;

         INT32          _replay( MsgOpReply* responseMsg, const CHAR *pBody,
                                 INT32 bodyLen ) ;

      // message process functions
      protected:

         INT32 _onInsertReqMsg( MsgHeader *msg ) ;
         INT32 _onUpdateReqMsg( MsgHeader *msg ) ;
         INT32 _onDelReqMsg( MsgHeader *msg ) ;
         INT32 _onInterruptMsg( MsgHeader *msg ) ;
         INT32 _onMsgReqMsg( MsgHeader *msg ) ;
         INT32 _onQueryReqMsg( MsgHeader *msg, INT64 &contextID ) ;
         INT32 _onGetMoreReqMsg( MsgHeader *msg, _rtnContextBuf &buffObj,
                                 INT32 &startingPos, INT64 &contextID ) ;
         INT32 _onKillContextsReqMsg( MsgHeader *msg ) ;
         INT32 _onSQLMsg( MsgHeader *msg, INT64 &contextID ) ;
         INT32 _onTransBeginMsg () ;
         INT32 _onTransCommitMsg () ;
         INT32 _onTransRollbackMsg () ;
         INT32 _onAggrReqMsg( MsgHeader *msg, INT64 &contextID ) ;

      protected:
         BOOLEAN              _authOK ;
         _SDB_DMSCB           *_pDMSCB ;
         _dpsLogWrapper       *_pDPSCB ;
         _SDB_RTNCB           *_pRTNCB ;

         MsgOpReply           _replayHeader ;
         BOOLEAN              _needReply ;

         BSONObj              _errorInfo ;

   } ;
   typedef _pmdLocalSession pmdLocalSession ;

}

#endif //PMD_SESSION_HPP_

