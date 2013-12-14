/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = sqlIndexFields.hpp

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

#ifndef SQLINDEXFIELDS_HPP_
#define SQLINDEXFIELDS_HPP_

#include "sqlDef.hpp"
#include "sqlGrammarBase.hpp"

namespace engine
{
   struct _sqlIndexFields : public grammar<_sqlIndexFields>
   {
      template <typename ScannerT>
      struct definition
      {
         rule<ScannerT> fields, desc, field ;
         const rule<ScannerT> &start() const
         {
            return fields ;
         }

         definition( _sqlIndexFields const &self )
         {
            field = SQL_RULE_FIELD;
            desc = SQL_RULE_DESC |SQL_RULE_ASC ;
            fields = no_node_d[!ch_p('(')]
                    >> ( ( leaf_node_d[field] >> !desc )
                           % no_node_d[ch_p(',')])
                    >> no_node_d[!ch_p(')')] ;
         }
      } ;
   } ;

   typedef struct _sqlIndexFields SQL_IXMFIELDS_GRAMMAR ;
}

#endif

