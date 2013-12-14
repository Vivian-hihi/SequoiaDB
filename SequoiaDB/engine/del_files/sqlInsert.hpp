/*******************************************************************************
   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = sqlInsert.hpp

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

#ifndef SQLINSERT_HPP_
#define SQLINSERT_HPP_

#include "sqlDef.hpp"
#include "sqlGrammarBase.hpp"

namespace engine
{
   struct _sqlInsert : public grammar<_sqlInsert>
   {
      template <typename ScannerT>
      struct definition
      {
         rule<ScannerT> insert, field, selector, fullName ;
         const rule<ScannerT> &start() const
         {
            return insert ;
         }

         definition( _sqlInsert const &self )
         {
            fullName = leaf_node_d[SQL_RULE_FULLNAME] ;
            insert = no_node_d[SQL_RULE_INSERT] >> fullName;
         }
      } ;
   } ;

   typedef struct _sqlInsert SQL_INSERT_GRAMMAR ;
}

#endif

