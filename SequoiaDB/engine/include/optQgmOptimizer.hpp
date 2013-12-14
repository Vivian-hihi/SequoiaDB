/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = optQgmOptimizer.hpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          23/04/2013  Xu Jianhui  Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef OPT_QGM_OPTIMIZER_HPP_
#define OPT_QGM_OPTIMIZER_HPP_

#include "qgmOptiTree.hpp"
#include "optQgmStrategy.hpp"

namespace engine
{
   typedef std::multimap< UINT32, UINT32 >         DEL_NODES ;

   class _optQgmOptimizer : public SDBObject
   {
      public:
         _optQgmOptimizer () ;
         ~_optQgmOptimizer () ;

      public:
         INT32    adjust( qgmOptTree &orgTree ) ;
         INT32    optimize( qgmOptTree &orgTree ) ;

      protected:
         INT32    _downAdjust( qgmOptTree &orgTree, BOOLEAN &adjust,
                               DEL_NODES  &delNodes ) ;
         INT32    _upAdjust( qgmOptTree &orgTree, BOOLEAN &adjust,
                             DEL_NODES  &delNodes ) ;

         INT32    _adjustByOprUnit( qgmOprUnitPtrVec * oprUnitVec,
                                    qgmOptTree &orgTree,
                                    qgmOptiTreeNode *curNode,
                                    BOOLEAN &adjust ) ;

         INT32    _onStrategy( qgmOprUnit *oprUnit,
                               qgmOptiTreeNode *curNode,
                               qgmOptiTreeNode *subNode,
                               OPT_QGM_SS_RESULT &result ) ;

         INT32    _processSSResult( qgmOptTree &orgTree,
                                    qgmOprUnit *oprUnit,
                                    qgmOptiTreeNode *curNode,
                                    qgmOptiTreeNode *subNode,
                                    OPT_QGM_SS_RESULT result,
                                    BOOLEAN &adjust ) ;

         INT32    _upbackOprUnit( qgmOprUnit * oprUnit,
                                  qgmOptTree &orgTree,
                                  qgmOptiTreeNode *curNode,
                                  BOOLEAN &adjust,
                                  DEL_NODES  &delNodes ) ;

      private:
         INT32    _formNewNode( qgmOptTree &orgTree,
                                qgmOprUnit *oprUnit,
                                qgmOptiTreeNode *curNode,
                                qgmOptiTreeNode *subNode,
                                qgmOptiTreeNode::PUSH_FROM from ) ;
         INT32    _prepareAdjust( qgmOptTree &orgTree ) ;
         INT32    _endAdjust( qgmOptTree &orgTree ) ;

   } ;
   typedef _optQgmOptimizer optQgmOptimizer ;

   optQgmOptimizer* getQgmOptimizer() ;

}

#endif //OPT_QGM_OPTIMIZER_HPP_
