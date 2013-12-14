/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = qgmOptiSplit.cpp

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

#include "qgmOptiSplit.hpp"

namespace engine
{
   _qgmOptiSplit::_qgmOptiSplit( _qgmPtrTable *table,
                                 _qgmParamTable *param,
                                 const qgmDbAttr &splitby )
   :_qgmOptiTreeNode(QGM_OPTI_TYPE_SPLIT, table, param ),
    _splitby(splitby)
   {
      
   }

   _qgmOptiSplit::~_qgmOptiSplit()
   {

   }

   INT32 _qgmOptiSplit::outputStream( qgmOpStream &stream )
   {
      SDB_ASSERT( 1 == _children.size(), "impossible" )
      return (*(_children.begin()))->outputStream( stream ) ;
   }

   string _qgmOptiSplit::toString() const
   {
      stringstream ss ;
      ss << "{" << this->_qgmOptiTreeNode::toString() ;
      ss << ", split by[" << _splitby.toString() << "]}" ;
      return ss.str() ;
   }
}
