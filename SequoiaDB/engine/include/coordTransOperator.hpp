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

   Source File Name = coordTransOperator.hpp

   Descriptive Name =

   When/how to use:

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          13/04/2017  XJH Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef COORD_TRANS_OPERATOR_HPP__
#define COORD_TRANS_OPERATOR_HPP__

#include "coordOperator.hpp"

using namespace bson ;

namespace engine
{

   /*
      _coordTransOperator
   */
   class _coordTransOperator : public _coordOperator
   {
      public:
         _coordTransOperator() ;
         virtual ~_coordTransOperator() ;

         virtual BOOLEAN      needRollback() const ;

      public:
         virtual INT32        doOnGroups( coordSendMsgIn &inMsg,
                                          coordSendOptions &options,
                                          pmdEDUCB *cb,
                                          coordProcessResult &result ) ;

         virtual INT32        rollback( MsgHeader *pMsg,
                                        pmdEDUCB *cb,
                                        INT64 &contextID,
                                        rtnContextBuf *buf ) ;

      protected:
         /*
            Prepare for transaction msg, you can change the opCode for trans
         */
         virtual void               _prepareForTrans( pmdEDUCB *cb,
                                                      MsgHeader *pMsg ) = 0 ;

         virtual BOOLEAN            _isTrans( pmdEDUCB *cb, MsgHeader *pMsg ) ;

         virtual void               _onNodeReply( INT32 processType,
                                                  MsgOpReply *pReply,
                                                  pmdEDUCB *cb,
                                                  coordSendMsgIn &inMsg ) ;

      protected:
         INT32          buildTransSession( const CoordGroupList &groupLst,
                                           pmdEDUCB *cb,
                                           ROUTE_RC_MAP &newNodeMap ) ;
    
         INT32          releaseTransSession( SET_NODEID &nodes,
                                             pmdEDUCB *cb  ) ;

      protected:
         UINT64         _recvNum ;

   } ;
   typedef _coordTransOperator coordTransOperator ;

}

#endif //COORD_TRANS_OPERATOR_HPP__

