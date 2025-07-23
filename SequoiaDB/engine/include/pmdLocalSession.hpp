/*******************************************************************************

   Copyright (C) 2011-Present SequoiaDB Ltd.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

   Source File Name = pmdLocalSession.hpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          04/04/2014  Xu Jianhui  Initial Draft

   Last Changed =

*******************************************************************************/
#ifndef PMD_LOCAL_SESSION_HPP_
#define PMD_LOCAL_SESSION_HPP_

#include "charsetConvertorInterface.hpp"
#include "msg.h"
#include "ossTypes.h"
#include "pmdSession.hpp"
#include "pmdDef.hpp"
#include "rtnContext.hpp"
#include "../bson/bson.h"
#include "msgConvertor.hpp"
#include "boost/move/unique_ptr.hpp"

using namespace bson ;

namespace engine
{
   /*
      _pmdLocalSession define
   */
   class _pmdLocalSession : public _pmdSession
   {
      public:
         _pmdLocalSession( SOCKET fd ) ;
         virtual ~_pmdLocalSession () ;

         virtual INT32            getServiceType() const ;
         virtual SDB_SESSION_TYPE sessionType() const ;

         virtual INT32            run() ;

         virtual INT32 checkPrivilegesForCmd( const CHAR *cmdName,
                                              const CHAR *pQuery,
                                              const CHAR *pSelector,
                                              const CHAR *pOrderby,
                                              const CHAR *pHint ) ;

         virtual INT32 checkPrivilegesForActionsOnExact( const CHAR *pCollectionName,
                                                         const authActionSet &actions ) ;

         virtual INT32 checkPrivilegesForActionsOnCluster( const authActionSet &actions ) ;

         virtual INT32 checkPrivilegesForActionsOnResource(
            const boost::shared_ptr< authResource > &resource,
            const authActionSet &actions );

         virtual INT32 getACL( boost::shared_ptr< const authAccessControlList > &acl ) ;

      protected:
         INT32          _processMsg( MsgHeader *msg ) ;
         INT32          _preprocessMsg( MsgHeader *&msg ) ;
         INT32          _onMsgBegin( MsgHeader *msg ) ;
         void           _onMsgEnd( INT32 result, MsgHeader *msg ) ;

         INT32          _recvSysInfoMsg( UINT32 msgSize, CHAR **ppBuff,
                                         INT32 &buffLen ) ;
         INT32          _processSysInfoRequest( const CHAR *msg ) ;

         BOOLEAN        _clientVersionMatch() const ;

         BOOLEAN        _msgConvertorEnabled() const ;

         INT32          _enableMsgConvertor() ;

         INT32          _reply( MsgOpReply* responseMsg, const CHAR *pBody,
                                INT32 bodyLen ) ;

         INT32          _replyInCompatibleMode( MsgOpReply *responseMsg,
                                                const CHAR *data,
                                                INT32 dataLen ) ;

         INT32          _replyInNormalMode( MsgOpReply *responseMsg,
                                            const CHAR *data,
                                            INT32 dataLen ) ;

      protected:
         virtual void            _onAttach () ;
         virtual void            _onDetach () ;

      private:

         // Build a new Message according to input MessageHeader and convertor
         INT32 _convertMsg( const MsgHeader *in, MsgHeader **out,
                            charsetConvertorInterface *convertor ) ;

         INT32 _convertReplyBody( rtnContextBuf &inReply,
                                  rtnContextBuf &outReply,
                                  charsetConvertorInterface *convertor ) ;

         INT32 _getClientCharset( Charset &clientCharset ) ;

         INT32 _getResultsCharset( Charset &resultsCharset ) ;

         INT32 _rebuildInsertMsg( const MsgHeader *in, MsgHeader **out,
                                  charsetConvertorInterface *convertor ) ;

         INT32 _rebuildUpdateMsg( const MsgHeader *in, MsgHeader **out,
                                  charsetConvertorInterface *convertor ) ;

         INT32 _rebuildSQLMsg( const MsgHeader *in, MsgHeader **out,
                               charsetConvertorInterface *convertor ) ;

         INT32 _rebuildDeleteMsg( const MsgHeader *in, MsgHeader **out,
                                  charsetConvertorInterface *convertor ) ;

         INT32 _rebuildAggrMsg( const MsgHeader *in, MsgHeader **out,
                                charsetConvertorInterface *convertor ) ;

         INT32 _rebuildQueryMsg( const MsgHeader *in, MsgHeader **out,
                                 charsetConvertorInterface *convertor ) ;

         INT32 _rebuildOpenLobMsg( const MsgHeader *in, MsgHeader **out,
                                   charsetConvertorInterface *convertor ) ;

         INT32 _rebuildRemoveLobMsg( const MsgHeader *in, MsgHeader **out,
                                     charsetConvertorInterface *convertor ) ;

         INT32 _rebuildTruncateLobMsg( const MsgHeader *in, MsgHeader **out,
                                       charsetConvertorInterface *convertor ) ;

         INT32 _rebuildCreateLobIDMsg( const MsgHeader *in, MsgHeader **out,
                                       charsetConvertorInterface *convertor ) ;

         INT32 _rebuildFetchSeqMsg( const MsgHeader *in, MsgHeader **out,
                                    charsetConvertorInterface *convertor ) ;

      protected:
         MsgOpReply           _replyHeader ;
         BOOLEAN              _needReply ;

         BSONObj              _errorInfo ;

         IMsgConvertor        *_inMsgConvertor ;   // For request from client.
         IMsgConvertor        *_outMsgConvertor ;  // For reply to client.

         boost::shared_ptr<const authAccessControlList> _acl;
         boost::movelib::unique_ptr<charsetConvertorInterface> _inConvertor ;
         boost::movelib::unique_ptr<charsetConvertorInterface> _outConvertor ;

   } ;
   typedef _pmdLocalSession pmdLocalSession ;

}

#endif //PMD_LOCAL_SESSION_HPP_


