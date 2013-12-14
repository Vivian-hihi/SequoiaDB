/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = qgmConditionNode.hpp

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

#ifndef QGMCONDITIONNODE_HPP_
#define QGMCONDITIONNODE_HPP_

#include "qgmOptiTree.hpp"
#include "pd.hpp"

namespace engine
{
   struct _qgmConditionNode : public SDBObject
   {
      qgmDbAttr value ;

      /// when type > SQL_GRAMMAR::SQLMAX, var is effective.
      const BSONElement *var ;
      INT32 type ;
      _qgmConditionNode *left ;
      _qgmConditionNode *right ;

      _qgmConditionNode( INT32 t )
      :var(NULL),
       type( t ),
       left( NULL ),
       right( NULL )
      {

      }

      _qgmConditionNode( const _qgmConditionNode *node )
      {
         SDB_ASSERT( NULL != node, "impossible" )
         type = node->type ;
         value = node->value ;
         var = node->var ;
         left = NULL ;
         right = NULL ;
         if ( NULL != node->left )
         {
            left = SDB_OSS_NEW _qgmConditionNode(node->left) ;
            if ( NULL == left )
            {
               PD_LOG( PDERROR, "failed to allocate mem." ) ;
            }
         }

         if ( NULL != node->right )
         {
            right = SDB_OSS_NEW _qgmConditionNode(node->right) ;
            if ( NULL == right )
            {
               PD_LOG( PDERROR, "failed to allocate mem." ) ;
            }
         }
      }

      virtual ~_qgmConditionNode()
      {
         SAFE_OSS_DELETE( left ) ;
         SAFE_OSS_DELETE( right ) ;
      }

      void  dettach ()
      {
         left = NULL ;
         right = NULL ;
      }
   } ;
   typedef struct _qgmConditionNode qgmConditionNode ;
   typedef vector< qgmConditionNode* > qgmConditionNodePtrVec ;
   typedef vector< qgmConditionNode >  qgmConditionNodeVec ;
}

#endif

