/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = qgmExtendSelectPlan.hpp

   Descriptive Name =

   When/how to use: this program may be used on binary and text-formatted
   versions of PMD component. This file contains functions for agent processing.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          09/14/2012  YW  Initial Draft

   Last Changed =

******************************************************************************/

#ifndef QGMEXTENDSELECTPLAN_HPP_
#define QGMEXTENDSELECTPLAN_HPP_

#include "qgmExtendPlan.hpp"
#include "qgmOptiAggregation.hpp"

namespace engine
{
   const UINT32 QGM_EXTEND_ORDERFILTER = 0 ;
   const UINT32 QGM_EXTEND_ORDERBY = 1 ;
   const UINT32 QGM_EXTEND_SPLITBY = 2 ;
   const UINT32 QGM_EXTEND_AGGR = 3 ;
   const UINT32 QGM_EXTEND_GROUPBY = 4 ;
   const UINT32 QGM_EXTEND_LOCAL = 5 ;

   class _qgmExtendSelectPlan : public _qgmExtendPlan
   {
   public:
      _qgmExtendSelectPlan() ;
      virtual ~_qgmExtendSelectPlan() ;

      void    clearConstraint() { _limit = -1 ; _skip = 0 ; }

   private:
      INT32 _extend( UINT32 id,
                     qgmOptiTreeNode *&extended ) ;

   protected:

   public:
      qgmAggrSelectorVec _funcSelector ;
      qgmOPFieldVec _groupby ;
      qgmOPFieldVec _orderby ;
      qgmOPFieldVec _original ;
      qgmDbAttr _splitby ;
      INT64         _limit ;
      INT64         _skip ;

   } ;

   typedef class _qgmExtendSelectPlan qgmExtendSelectPlan ;
}

#endif

