/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = sqlInsertFields.hpp

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

#ifndef SQLINSERTFIELDS_HPP_
#define SQLINSERTFIELDS_HPP_

#include "sqlGrammarBase.hpp"

namespace engine
{
   struct _sqlInsertFields : public grammar<_sqlInsertFields>
   {
      template <typename ScannerT>
      struct definition
      {
         rule<ScannerT> fields, field ;
         const rule<ScannerT> &start() const
         {
            return fields ;
         }

         definition( _sqlInsertFields const &self )
         {
            field = leaf_node_d[SQL_RULE_FIELD] ;
            fields = no_node_d[ch_p('(')] >> +(field % no_node_d[ch_p(',')]) >>
                     no_node_d[ch_p(')')] ;
         }
      } ;
   } ;

   typedef struct _sqlInsertFields  SQL_IFIELDS_GRAMMAR ;
}

#endif

