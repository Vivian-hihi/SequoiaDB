/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = sqlDelete.hpp

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

#ifndef SQLDELETE_HPP_
#define SQLDELETE_HPP_

#include "sqlDef.hpp"
#include "sqlGrammarBase.hpp"

namespace engine
{
   struct _sqlDelete : public grammar<_sqlDelete>
   {
      template <typename ScannerT>
      struct definition
      {
         rule<ScannerT> del, fullName;
         const rule<ScannerT> &start() const
         {
            return del ;
         }

         definition( _sqlDelete const &self )
         {
            fullName = leaf_node_d[SQL_RULE_FULLNAME] ;
            del = no_node_d[SQL_RULE_DELETE] >> fullName ;
         }
      } ;
   } ;

   typedef struct _sqlDelete SQL_DELETE_GRAMMAR ;
}

#endif

