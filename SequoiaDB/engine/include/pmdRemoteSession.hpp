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

   Source File Name = pmdRemoteSession.hpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          06/05/2014  Xu Jianhui  Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef PMD_REMOTE_SESSION_HPP_
#define PMD_REMOTE_SESSION_HPP_

#include "core.hpp"
#include "oss.hpp"
#include "msg.h"
#include "netRouteAgent.hpp"

#include <map>
#include "../bson/bson.h"

using namespace bson ;
using namespace std ;

namespace engine
{

   /*
      _pmdSubSession define
   */
   class _pmdSubSession : public SDBObject
   {
      public:
         _pmdSubSession() ;
         ~_pmdSubSession() ;

         netIOVec*   getIODatas() { return &_ioDatas ; }
         void        clearIODatas() { _ioDatas.clear() ; }
         void        addIODatas( const netIOVec &ioVec )
         {
            for ( UINT32 i = 0 ; i < ioVec.size() ; ++i )
            {
               _ioDatas.push_back( ioVec[ i ] ) ;
            }
         }
         void        addIOData( const netIOV &io )
         {
            _ioDatas.push_back( io ) ;
         }
         UINT32      getIODataLen()
         {
            UINT32 len = 0 ;
            for ( UINT32 i = 0 ; i < _ioDatas.size() ; ++i )
            {
               len += _ioDatas[ i ].iovLen ;
            }
            return len ;
         }

         void        setReqMsg( MsgHeader *pReqMsg )
         {
            _pReqMsg = pReqMsg ;
         }
         MsgHeader*  getReqMsg() { return _pReqMsg ; }
         void        setRspMsg( MsgHeader *pRspMsg )
         {
            _pRspMsg = pRspMsg ;
         }
         MsgHeader*  getRspMsg() { return _pRspMsg ; }

         void        setNodeID( UINT64 nodeID ) { _nodeID = nodeID ; }
         UINT64      getNodeID() const { return _nodeID ; }

         void        setReqID( UINT64 reqID ) { _reqID = reqID ; }

      protected:
         UINT64                     _nodeID ;
         UINT64                     _reqID ;

         MsgHeader                  *_pReqMsg ;
         netIOVec                   _ioDatas ;
         MsgHeader                  *_pRspMsg ;
   } ;
   typedef _pmdSubSession pmdSubSession ;

   typedef map< UINT64, pmdSubSession >            MAP_SUB_SESSION ;
   typedef MAP_SUB_SESSION::iterator               MAP_SUB_SESSION_IT ;

   typedef map< UINT64, pmdSubSession* >           MAP_SUB_SESSIONPTR ;
   typedef MAP_SUB_SESSIONPTR::iterator            MAP_SUB_SESSIONPTR_IT ;

   typedef map< UINT64, UINT64 >                   MAP_REQ_TO_SUBSESSION ;
   typedef MAP_REQ_TO_SUBSESSION::iterator         MAP_REQ_TO_SUBSESSION_IT ;

   /*
      _pmdRemoteSession define
   */
   class _pmdRemoteSession : public SDBObject
   {
      public:
         _pmdRemoteSession( netRouteAgent *pAgent ) ;
         virtual ~_pmdRemoteSession() ;

         pmdSubSession* addCurSubSession( UINT64 nodeID ) ;


      public:
         INT32    sendMsg() ;

      protected:
         MAP_SUB_SESSION               _mapSubSession ;
         MAP_SUB_SESSIONPTR            _mapCurSubSession ;
         MAP_REQ_TO_SUBSESSION         _mapReq2SubSession ;

         netRouteAgent                 *_pAgent ;

   } ;
   typedef _pmdRemoteSession pmdRemoteSession ;


}

#endif //PMD_REMOTE_SESSION_HPP_

