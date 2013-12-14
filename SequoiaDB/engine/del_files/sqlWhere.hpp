/*******************************************************************************
   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = sqlWhere.hpp

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

#ifndef SQLWHERE_HPP_
#define SQLWHERE_HPP_

#include "sqlDef.hpp"
#include "sqlGrammarBase.hpp"

namespace engine
{
   struct _sqlWhere : public grammar<_sqlWhere>
   {
      template <typename ScannerT>
      struct definition
      {
         rule<ScannerT> field, digital, var, factor, condition, wdomain;
         const rule<ScannerT> &start() const
         {
            return wdomain ;
         }

         definition( _sqlWhere const &self )
         {
            field = leaf_node_d[SQL_RULE_FIELD] ;
            digital = SQL_RULE_DIGITAL ;
            var = leaf_node_d[digital
                              | (ch_p('"') >> digital >> ch_p('"'))
                              |  (ch_p('"') >> +alnum_p >> ch_p('"'))
                              |  +alnum_p] ;
            /// ">=" and "<=" must before "<" and ">"
            factor = (field >>
                     root_node_d[str_p(SQL_SYMBOL_GTE)|str_p(SQL_SYMBOL_LTE)
                                 |str_p(SQL_SYMBOL_NE) |str_p(SQL_SYMBOL_EG)
                                 |str_p(SQL_SYMBOL_LT)|str_p(SQL_SYMBOL_GT)]
                     >> var )
                     | (inner_node_d[str_p(SQL_SYMBOL_LBRACKETS) >> condition >>
                       str_p(SQL_SYMBOL_RBRACKETS)] ) ;
            condition = factor >>
                        *( (root_node_d[as_lower_d[str_p(SQL_SYMBOL_AND)]]
                           >> factor)
                           | (root_node_d[as_lower_d[str_p(SQL_SYMBOL_OR)]]
                           >> factor) ) ;
            wdomain = no_node_d[as_lower_d[str_p(SQL_SYMBOL_WHERE)]]
                      >> condition ;
         }
      } ;
   } ;

   typedef struct _sqlWhere SQL_WHERE_GRAMMAR ;
}

#endif

