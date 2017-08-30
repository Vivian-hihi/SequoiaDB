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

   Source File Name = rtnContextTS.hpp

   Descriptive Name = RunTime Text Search Context

   When/how to use: this program may be used on binary and text-formatted
   versions of Runtime component. This file contains structure for Runtime
   Context.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          08/30/2017  YSD Split from rtnContextData.hpp

   Last Changed =

*******************************************************************************/
#ifndef RTN_CONTEXTTS_HPP__
#define RTN_CONTEXTTS_HPP__

#include "rtnContext.hpp"
#include "rtnQueryOptions.hpp"
#include "pmdRemoteSession.hpp"

namespace engine
{
   // Remote session handler. Used by _rtnContextTS.
   class _rtnRSHandler : public IRemoteSessionHandler
   {
      public:
         _rtnRSHandler() ;
         virtual ~_rtnRSHandler() ;
      public:
         virtual INT32  onSendFailed( _pmdRemoteSession *pSession,
                                      _pmdSubSession **ppSub,
                                      INT32 flag ) ;

         virtual void   onReply( _pmdRemoteSession *pSession,
                                 _pmdSubSession **ppSub,
                                 const MsgHeader *pReply,
                                 BOOLEAN isPending ) ;

         virtual INT32  onSendConnect( _pmdSubSession *pSub,
                                       const MsgHeader *pReq,
                                       BOOLEAN isFirst ) ;
   } ;
   typedef _rtnRSHandler rtnRSHandler ;

   // Context for text search data.
   class _rtnContextTS : public _rtnContextBase
   {
      DECLARE_RTN_CTX_AUTO_REGISTER()

      public:
         _rtnContextTS( INT64 contextID, UINT64 eduID ) ;
         virtual ~_rtnContextTS() ;

      public:
         virtual std::string name() const ;
         virtual RTN_CONTEXT_TYPE getType() const ;
         virtual _dmsStorageUnit* getSU() ;

         INT32 open( const rtnQueryOptions &options, pmdEDUCB *eduCB ) ;

      protected:
         virtual INT32 _prepareData( _pmdEDUCB *cb ) ;

      private:
         INT32 _prepareRemoteSession( _pmdEDUCB *eduCB ) ;
         INT32 _getMoreFromRemote( pmdEDUCB *eduCB ) ;
         INT32 _sendToRemote( const MsgHeader *msg ) ;
         INT32 _waitAndProcessRemoteReply() ;

      private:
         pmdEDUCB*            _eduCB ;
         rtnQueryOptions      _options ;
         pmdRemoteSessionSite *_remoteSessionSite ;
         pmdRemoteSession     *_remoteSession ;
         rtnRSHandler         _rsHandler ;
         SINT64               _subCtxID ;    // Context to fetch data from local
                                             // after get new query from ES.
   } ;
   typedef _rtnContextTS rtnContextTS ;
}

#endif /* RTN_CONTEXTTS_HPP__ */

