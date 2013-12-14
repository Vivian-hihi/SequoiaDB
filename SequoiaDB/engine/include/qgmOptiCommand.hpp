/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = qgmOptiCommand.hpp

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

#ifndef QGMOPTICOMMAND_HPP_
#define QGMOPTICOMMAND_HPP_

#include "qgmOptiTree.hpp"

namespace engine
{
   class _qgmOptiCommand : public _qgmOptiTreeNode
   {
   public:
      _qgmOptiCommand( _qgmPtrTable *table,
                       _qgmParamTable *param )
      :_qgmOptiTreeNode( QGM_OPTI_TYPE_COMMAND,
                         table, param ),
       _commandType( SQL_GRAMMAR::SQLMAX ),
       _uniqIndex( FALSE )
      {

      }

      virtual ~_qgmOptiCommand(){}

   public:
      virtual INT32 outputStream( qgmOpStream &stream )
      {
         return SDB_SYS ;
      }

   public:
      INT32 _commandType ;
      qgmDbAttr _fullName ;
      qgmField _indexName ;
      qgmOPFieldVec _indexColumns ;
      qgmOPFieldVec _partition ;
      BOOLEAN _uniqIndex ;
   } ;
   typedef class _qgmOptiCommand qgmOptiCommand ;
}

#endif

