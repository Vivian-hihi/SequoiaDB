/*******************************************************************************


   Copyright (C) 2011-2017 SequoiaDB Ltd.

   This program is free software: you can redistribute it and/or modify
   it under the term of the GNU Affero General Public License, version 3,
   as published by the Free Software Foundation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warrenty of
   MARCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU Affero General Public License for more details.

   You should have received a copy of the GNU Affero General Public License
   along with this program. If not, see <http://www.gnu.org/license/>.

   Source File Name = seAdptAgentSession.hpp

   Descriptive Name = Agent session on search engine adapter.

   When/how to use: this program may be used on binary and text-formatted
   versions of PMD component. This file contains main function for sdbcm,
   which is used to do cluster managing.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          04/14/2017  YSD  Initial Draft

   Last Changed =

*******************************************************************************/
#ifndef SEADPT_AGENT_SESSION_HPP_
#define SEADPT_AGENT_SESSION_HPP_

#include "pmdAsyncSession.hpp"
#include "utilCommObjBuff.hpp"
#include "seAdptMgr.hpp"
#include "utilESClt.hpp"
#include "seAdptContext.hpp"

namespace engine
{
   class _seAdptAgentSession : public _pmdAsyncSession
   {
      DECLARE_OBJ_MSG_MAP()
   public:
      _seAdptAgentSession( UINT64 sessionID ) ;
      virtual ~_seAdptAgentSession() ;

      virtual EDU_TYPES eduType() const ;
      virtual SDB_SESSION_TYPE sessionType() const ;
      virtual const CHAR* className() const { return "SEAdptAgent" ; }

      virtual void onRecieve( const NET_HANDLE netHandle, MsgHeader * msg ) ;
      virtual BOOLEAN timeout( UINT32 interval ) ;
      virtual void onTimer( UINT64 timerID, UINT32 interval ) ;

   protected:
      virtual void _onAttach() ;
      virtual void _onDetach() ;

      INT32 _onOPMsg ( NET_HANDLE handle, MsgHeader * msg ) ;
      BOOLEAN _isCommand ( const CHAR *name ) ;
      INT32 _onQueryReqMsg( MsgHeader *msg,
                            utilCommObjBuff &objBuff ) ;
      INT32 _onGetMoreReqMsg( MsgHeader *msg,
                              utilCommObjBuff &objBuff ) ;
      INT32 _onRewriteQuery( MsgHeader *msg,
                             utilCommObjBuff &objBuff,
                             pmdEDUCB *eduCB = NULL ) ;

      INT32 _onRewriteQueryMore( MsgHeader *msg,
                                 utilCommObjBuff &objBuff,
                                 pmdEDUCB *eduCB = NULL ) ;

      INT32 _onKillContextReqMsg( NET_HANDLE handle,
                                  MsgHeader *msg ) ;
      INT32 _onRebuildIdxReqMsg() ;

      INT32 _reply( MsgOpReply *header, const CHAR *buff, UINT32 size ) ;
      INT32 _defaultMsgFunc ( NET_HANDLE handle, MsgHeader *msg ) ;

      INT32 _onAuthReq( NET_HANDLE handle, MsgHeader *msg ) ;

   private:
      INT32 _getIndexAndType( const CHAR *pCollectionName,
                              const CHAR *pHint,
                              UINT32 groupID,
                              std::string &indexName,
                              std::string &typeName ) ;
      INT32 _getQueryCond( const BSONObj &matcher, std::string &queryStr ) ;
      INT32 _buildInCond( const utilCommObjBuff &objBuff, BSONObj &condition ) ;

      INT32 _doOnSE() ;
   private:
      utilESCltMgr *_seCltMgr ;
      utilESClt *_esClt ;
      std::string _scrollID ;
      utilCommObjBuff _resultObjs ;
      seAdptContextBase *_context ;
   } ;
   typedef _seAdptAgentSession seAdptAgentSession ;
}

#endif /* SEADPT_AGENT_SESSION_HPP_ */

