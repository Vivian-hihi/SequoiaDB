/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = qgmPlDelete.hpp

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

#ifndef QGMPLDELETE_HPP_
#define QGMPLDELETE_HPP_

#include "qgmPlan.hpp"

namespace engine
{
   struct _qgmConditionNode ;

   class _qgmPlDelete : public _qgmPlan
   {
   public:
      _qgmPlDelete( const qgmDbAttr &collection,
                    _qgmConditionNode *condition ) ;
      virtual ~_qgmPlDelete() ;
   public:
      virtual string toString() const ;
   private:
      virtual INT32 _execute( _pmdEDUCB *eduCB ) ;

      virtual INT32 _fetchNext( qgmFetchOut &next )
      {
         return SDB_SYS ;
      }

   private:
      qgmDbAttr _collection ;
      BSONObj _condition ;
   } ;

   typedef class _qgmPlDelete qgmPlDelete ;
}

#endif

