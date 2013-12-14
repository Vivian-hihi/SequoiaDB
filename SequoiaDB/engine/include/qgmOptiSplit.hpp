/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = qgmOptiSplit.hpp

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

******************************************************************************/

#ifndef QGMOPTISPLIT_HPP_
#define QGMOPTISPLIT_HPP_

#include "qgmOptiTree.hpp"

namespace engine
{
   class _qgmOptiSplit : public qgmOptiTreeNode
   {
   public:
      _qgmOptiSplit( _qgmPtrTable *table,
                     _qgmParamTable *param,
                     const _qgmDbAttr &split ) ;
      virtual ~_qgmOptiSplit() ;

   public:
      virtual INT32 outputStream( qgmOpStream &stream ) ;
   
      virtual string toString() const ;

      virtual BOOLEAN isEmpty() { return FALSE ;}

   public:
      qgmDbAttr _splitby ;    
   } ;
   typedef class _qgmOptiSplit qgmOptiSplit ;
}

#endif

