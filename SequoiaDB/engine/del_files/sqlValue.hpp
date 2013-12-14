/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = sqlValue.hpp

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

#ifndef SQLVALUE_HPP_
#define SQLVALUE_HPP_

#include "sqlDef.hpp"
#include "sqlGrammarBase.hpp"

namespace engine
{
   struct _sqlValue : public grammar<_sqlValue>
   {
      template <typename ScannerT>
      struct definition
      {
         rule<ScannerT> value, digital, var ;
         const rule<ScannerT> &start() const
         {
            return value ;
         }

         definition( _sqlValue const &self )
         {
            digital = SQL_RULE_DIGITAL ;
            var = leaf_node_d[digital
                              | (ch_p('"') >> digital >> ch_p('"'))
                              |  (ch_p('"') >> +alnum_p >> ch_p('"'))
                              |  +alnum_p] ;
            value = no_node_d[SQL_RULE_VALUES] >> no_node_d[ch_p('(')] >>
                    var % no_node_d[ch_p(',')] >>  no_node_d[ch_p(')')] ;
         }
      } ;
   } ;

   typedef struct _sqlValue SQL_VALUE_GRAMMAR ;
}

#endif

