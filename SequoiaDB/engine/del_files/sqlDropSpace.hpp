/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = sqlDropSpace.hpp

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

*******************************************************************************/

#ifndef SQLDROPSPACE_HPP_
#define SQLDROPSPACE_HPP_

#include "sqlDef.hpp"
#include "sqlGrammarBase.hpp"

namespace engine
{
   struct _sqlDropSpace : public grammar<_sqlDropSpace>
   {
      template <typename ScannerT>
      struct definition
      {
         rule<ScannerT> drop, fullName;
         const rule<ScannerT> &start() const
         {
            return drop ;
         }

         definition( _sqlDropSpace const &self )
         {
            fullName = leaf_node_d[+alnum_p] ;
            drop = no_node_d[SQL_RULE_DROPSPACE] >> fullName ;
         }
      } ;
   } ;

   typedef struct _sqlDropSpace SQL_DROPSPACE_GRAMMAR ;
}

#endif

