/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = sqlFrom.hpp

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

#ifndef SQLFROM_HPP_
#define SQLFROM_HPP_

#include "sqlDef.hpp"
#include "sqlGrammarBase.hpp"

namespace engine
{
   struct _sqlFrom : public grammar<_sqlFrom>
   {
      template <typename ScannerT>
      struct definition
      {
         rule<ScannerT> from, fullname ;
         const rule<ScannerT> &start() const
         {
            return from ;
         }

         definition( _sqlFrom const &self )
         {
            fullname = leaf_node_d[SQL_RULE_FULLNAME] ;
            from = no_node_d[SQL_RULE_FROM] >> fullname ;
         }
      } ;
   } ;

   typedef struct _sqlFrom SQL_FROM_GRAMMAR ;
}

#endif

