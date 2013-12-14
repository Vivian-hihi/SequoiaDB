/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = qgmOptiSort.hpp

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

#ifndef QGMOPTISORT_HPP_
#define QGMOPTISORT_HPP_

#include "qgmOptiTree.hpp"

namespace engine
{
   class _qgmOptiSort : public _qgmOptiTreeNode
   {
   public:
      _qgmOptiSort( _qgmPtrTable *table, _qgmParamTable *param ) ;
      virtual ~_qgmOptiSort() ;

      virtual INT32        init () ;
      qgmOPFieldVec* getOrderby() ;

   public:
      virtual INT32 outputStream( qgmOpStream &stream ) ;
      virtual BOOLEAN   isEmpty() ;

      virtual string toString() const ;

   protected:
      virtual INT32 _pushOprUnit( qgmOprUnit *oprUnit, PUSH_FROM from ) ;
      virtual INT32 _removeOprUnit( qgmOprUnit *oprUnit ) ;
      virtual INT32 _updateChange( qgmOprUnit *oprUnit ) ;

   public:
      INT32 append( const qgmOPFieldVec &field,
                    BOOLEAN keepRelegation = TRUE ) ;

   public:
      qgmOPFieldVec      _orderby ;

   } ;
   typedef class _qgmOptiSort qgmOptiSort ;

}

#endif

