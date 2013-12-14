/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = sqlDropIndex.hpp

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

#ifndef SQLDROPINDEX_HPP_
#define SQLDROPINDEX_HPP_

#include "sqlDef.hpp"
#include "sqlGrammarBase.hpp"

namespace engine
{
   struct _sqlDropIndex : public grammar<_sqlDropIndex>
   {
      template <typename ScannerT>
      struct definition
      {
         rule<ScannerT> index, fullName, name ;
         const rule<ScannerT> &start() const
         {
            return index ;
         }

         definition( _sqlDropIndex const &self )
         {
            fullName = leaf_node_d[SQL_RULE_FULLNAME] ;
            name = leaf_node_d[+graph_p] ;
            index = no_node_d[SQL_RULE_DROPINDEX] >>
                     name >> no_node_d[SQL_RULE_ON] >> fullName ;
         }
      } ;
   } ;

   typedef struct _sqlDropIndex SQL_DROPIXM_GRAMMAR ;
}

#endif

