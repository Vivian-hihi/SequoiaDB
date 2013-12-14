/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = qgmExtendPlan.hpp

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

#ifndef QGMEXTENDPLAN_HPP_
#define QGMEXTENDPLAN_HPP_

#include "qgmOptiTree.hpp"
#include <queue>

namespace engine
{

   typedef INT32 QGM_EXTEND_ID ;
   typedef std::map<QGM_EXTEND_ID, qgmOptiTreeNode* >   QGM_EXTEND_TABLE ;

   class _qgmExtendPlan : public SDBObject
   {
   public:
      _qgmExtendPlan() ;
      virtual ~_qgmExtendPlan() ;

   public:
      INT32 extend( qgmOptiTreeNode *&extended ) ;

      /// when local is not NULL, it will be defined as local.
      INT32 insertPlan( UINT32 id, qgmOptiTreeNode *ex = NULL ) ;

      inline qgmOptiTreeNode *getNode( UINT32 id )
      {
         QGM_EXTEND_TABLE::iterator it = _table.find( id ) ;
         return ( it == _table.end() ) ? NULL : it->second ;
      }

      inline void pushAlias( const qgmField &alias )
      {
         _aliases.push( alias ) ;
      }

   private:
      virtual INT32 _extend( UINT32 id,
                             qgmOptiTreeNode *&extended ) = 0 ;

   protected:
      QGM_EXTEND_TABLE _table ;
      qgmOptiTreeNode  *_local ;
      UINT32 _localID ;
      std::queue<qgmField> _aliases ;

   } ;

   typedef class _qgmExtendPlan qgmExtendPlan ;
}

#endif

