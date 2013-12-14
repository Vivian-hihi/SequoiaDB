/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = sqlSet.hpp

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

#ifndef SQLSET_HPP_
#define SQLSET_HPP_

#include "sqlDef.hpp"
#include "sqlGrammarBase.hpp"

namespace engine
{
   struct _sqlSet : public grammar<_sqlSet>
   {
      template <typename ScannerT>
      struct definition
      {
         rule<ScannerT> set, fields, field, digital, var ;
         const rule<ScannerT> &start() const
         {
            return set ;
         }

         definition( _sqlSet const &self )
         {
            field = leaf_node_d[SQL_RULE_FIELD ];
            digital = SQL_RULE_DIGITAL ;
            var = leaf_node_d[digital
                              | (ch_p('"') >> digital >> ch_p('"'))
                              |  (ch_p('"') >> +alnum_p >> ch_p('"'))
                              |  +alnum_p] ;
            fields = field >> root_node_d[ch_p('=')] >> var ;
            set = no_node_d[SQL_RULE_SET] >>
                 ( fields % no_node_d[ch_p(',')] ) ;
         }
      } ;
   } ;

   typedef struct _sqlSet SQL_SET_GRAMMAR ;
}

#endif

