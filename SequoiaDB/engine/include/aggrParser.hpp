/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = aggrParser.hpp

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
#ifndef AGGRPARSER_HPP__
#define AGGRPARSER_HPP__

#include "../bson/bsonobj.h"
#include "qgmPtrTable.hpp"
#include "qgmOptiTree.hpp"
#include "qgmParamTable.hpp"
#include "qgmOptiAggregation.hpp"

namespace engine
{
   class _qgmOptiTreeNode;
   class _qgmPtrTable;
   class _qgmParamTable;
   class aggrParser : public SDBObject
   {
   public:
      virtual ~aggrParser(){}
      // change to plan to parent node
      virtual INT32 parse( const bson::BSONElement &elem,
                           _qgmOptiTreeNode *&root,
                           _qgmPtrTable * pPtrTable,
                           _qgmParamTable *pParamTable,
                           const CHAR *pCollectionName );

   private:
      virtual INT32 buildNode( const bson::BSONElement &elem,
                              const CHAR *pCLName,
                              qgmOptiTreeNode *&pNode,
                              _qgmPtrTable *pTable,
                              _qgmParamTable *pParamTable ) = 0;
   };
}

#endif
