/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = aggrParser.cpp

   Descriptive Name =

   When/how to use: this program may be used on binary and text-formatted
   versions of PMD component. This file contains functions for agent processing.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          09/04/2013  JHL  Initial Draft

   Last Changed =

******************************************************************************/
#include "aggrParser.hpp"
#include "qgmDef.hpp"
#include "qgmOptiSelect.hpp"
#include "aggrDef.hpp"
#include "msgDef.h"
#include "qgmOptiTree.hpp"

using namespace bson;

namespace engine
{
   INT32 aggrParser::parse( const BSONElement &elem,
                                 _qgmOptiTreeNode *&root,
                                 _qgmPtrTable * pPtrTable,
                                 _qgmParamTable *pParamTable,
                                 const CHAR *pCollectionName )
   {
      INT32 rc = SDB_OK;
      SDB_ASSERT( pPtrTable!=NULL, "pPtrTable can't be NULL!" );
      SDB_ASSERT( pParamTable!=NULL, "pParamTable can't be NULL!" );
      SDB_ASSERT( pCollectionName != NULL || root != NULL,
                  "collectionname can't be NULL in leaf-node" );
      _qgmOptiTreeNode *pNode = NULL;

      rc = buildNode( elem, pCollectionName, pNode, pPtrTable,
                     pParamTable );
      PD_RC_CHECK( rc, PDERROR, "failed to build the node(rc=%d)", rc );
      if ( root != NULL )
      {
         rc = pNode->addChild( root );
         PD_RC_CHECK( rc, PDERROR, "failed to add the child(rc=%d)", rc );
         root->_father = pNode;
      }

      root = pNode;
   done:
      return rc;
   error:
      SAFE_OSS_DELETE( pNode );
      goto done;
   }

}
