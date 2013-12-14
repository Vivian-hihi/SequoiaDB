/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = aggrSort.hpp

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
#ifndef AGGRSORT_HPP__
#define AGGRSORT_HPP__

#include "aggrParser.hpp"

namespace engine
{
   class aggrSortParser : public aggrParser
   {
   private:
      INT32 buildNode( const bson::BSONElement &elem, const CHAR *pCLName,
                     qgmOptiTreeNode *&pNode, _qgmPtrTable *pTable,
                     _qgmParamTable *pParamTable );

      INT32 buildOrderBy( const bson::BSONElement &elem, qgmOPFieldVec &orderBy,
                            _qgmPtrTable *pTable, const CHAR *pCLName );
   };
}

#endif


