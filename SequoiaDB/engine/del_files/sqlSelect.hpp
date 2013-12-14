/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = sqlParser.hpp

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

#ifndef SQLSELECT_HPP_
#define SQLSELECT_HPP_

#include "sqlDef.hpp"
#include "sqlGrammarBase.hpp"

namespace engine
{
   struct _sqlSelect : public grammar<_sqlSelect>
   {
      template <typename ScannerT>
      struct definition
      {
         rule<ScannerT> select, field, selector ;

         const rule<ScannerT> &start() const
         {
            return  select ;
         }

         definition( _sqlSelect const &self )
         {
            field = SQL_RULE_FIELD ;
            /// we do not care about ',' and '*'
            selector = ( leaf_node_d[field] % no_node_d[ch_p(',')])| no_node_d[ch_p('*')] ;
            select = no_node_d[SQL_RULE_SELECT] >> selector ;
         }
      } ;
   } ;

   typedef struct _sqlSelect SQL_SELECT_GRAMMAR ;
}

#endif

