/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = sqlCrtSpace.hpp

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

#ifndef SQLCRTSPACE_HPP_
#define SQLCRTSPACE_HPP_

#include "sqlDef.hpp"
#include "sqlGrammarBase.hpp"

namespace engine
{
   struct _sqlCrtSpace : public grammar<_sqlCrtSpace>
   {
      template <typename ScannerT>
      struct definition
      {
         rule<ScannerT> crt, fullName;
         const rule<ScannerT> &start() const
         {
            return crt ;
         }

         definition( _sqlCrtSpace const &self )
         {
            fullName = leaf_node_d[+alnum_p] ;
            crt = no_node_d[SQL_RULE_CRTSPACE] >> fullName ;
         }
      } ;
   } ;

   typedef struct _sqlCrtSpace SQL_CRTSPACE_GRAMMAR ;

}

#endif

