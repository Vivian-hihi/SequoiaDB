/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = qgmOptiUpdate.hpp

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

#ifndef QGMOPTIUPDATE_HPP_
#define QGMOPTIUPDATE_HPP_

#include "qgmOptiTree.hpp"

namespace engine
{
   class _qgmOptiUpdate : public _qgmOptiTreeNode
   {
   public:
      _qgmOptiUpdate( _qgmPtrTable *table, _qgmParamTable *param )
      :_qgmOptiTreeNode( QGM_OPTI_TYPE_UPDATE,
                         table, param ),
       _condition( NULL )
      {

      }

      virtual ~_qgmOptiUpdate()
      {
         SAFE_OSS_DELETE( _condition ) ;
      }

   public:
      virtual INT32 outputStream( qgmOpStream &stream )
      {
         return SDB_SYS ;
      }

      virtual string toString() const
      {
         return "update" ;
      }

   public:
      _qgmDbAttr _collection ;
      qgmDbAttrVec _columns ;
      qgmOPFieldVec _values ;
      _qgmConditionNode *_condition ;
   } ;
   typedef class _qgmOptiUpdate qgmOptiUpdate ;
}

#endif

