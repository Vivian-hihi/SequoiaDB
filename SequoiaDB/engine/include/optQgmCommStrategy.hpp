/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = optQgmCommStrategy.hpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          27/04/2013  Xu Jianhui  Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef OPT_QGM_COMM_STRATEGY_HPP_
#define OPT_QGM_COMM_STRATEGY_HPP_

#include "optQgmStrategy.hpp"

namespace engine
{

   class _optQgmAcceptSty : public optQgmStrategyBase
   {
      public:
         _optQgmAcceptSty() {}
         virtual ~_optQgmAcceptSty() {}

      public:
         virtual INT32  calcResult( qgmOprUnit *oprUnit,
                                    qgmOptiTreeNode *curNode,
                                    qgmOptiTreeNode *subNode,
                                    OPT_QGM_SS_RESULT &result )
         {
            result = OPT_SS_ACCEPT ;
            return SDB_OK ;
         }

         virtual const CHAR* strategyName() const
         {
            return "ACEETP-Stragegy" ;
         }
   };
   typedef _optQgmAcceptSty optQgmAcceptSty ;

   class _optQgmRefuseSty : public optQgmStrategyBase
   {
      public:
         _optQgmRefuseSty() {}
         virtual ~_optQgmRefuseSty() {}

      public:
         virtual INT32  calcResult( qgmOprUnit *oprUnit,
                                    qgmOptiTreeNode *curNode,
                                    qgmOptiTreeNode *subNode,
                                    OPT_QGM_SS_RESULT &result )
         {
            result = OPT_SS_REFUSE ;
            return SDB_OK ;
         }

         virtual const CHAR* strategyName() const
         {
            return "REFUSE-Strategy" ;
         }
   };
   typedef _optQgmRefuseSty optQgmRefuseSty ;

   class _optQgmTakeOverSty : public optQgmStrategyBase
   {
      public:
         _optQgmTakeOverSty() {}
         virtual ~_optQgmTakeOverSty() {}

      public:
         virtual INT32  calcResult( qgmOprUnit *oprUnit,
                                    qgmOptiTreeNode *curNode,
                                    qgmOptiTreeNode *subNode,
                                    OPT_QGM_SS_RESULT &result )
         {
            result = OPT_SS_TAKEOVER ;
            return SDB_OK ;
         }

         virtual const CHAR* strategyName() const
         {
            return "TAKEOVER-Strategy" ;
         }
   };
   typedef _optQgmTakeOverSty optQgmTakeOverSty ;

}

#endif //OPT_QGM_COMM_STRATEGY_HPP_

