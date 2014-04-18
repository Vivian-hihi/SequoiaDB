/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = qgmPlanContainer.hpp

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

#ifndef QGMPLANCONTAINER_HPP_
#define QGMPLANCONTAINER_HPP_

#include "qgmPlan.hpp"
#include "qgmParamTable.hpp"
#include "qgmPtrTable.hpp"

namespace engine
{
   class _pmdEDUCB ;

   class _qgmPlanContainer : public SDBObject
   {
   public:
      _qgmPlanContainer() ;
      virtual ~_qgmPlanContainer() ;

   public:
      OSS_INLINE qgmParamTable *paramTable()
      {
         return &_paramT ;
      }

      OSS_INLINE qgmPtrTable *ptrTable()
      {
         return &_ptrT ;
      }

      OSS_INLINE qgmPlan *&plan()
      {
         return _plan ;
      }

      OSS_INLINE void close()
      {
         if ( NULL != _plan )
         {
            _plan->close() ;
         }
         return ;
      }

      OSS_INLINE QGM_PLAN_TYPE type()
      {
         return NULL == _plan ?
                QGM_PLAN_TYPE_MAX : _plan->type() ;
      }

      OSS_INLINE SQL_AST &ast()
      {
         return _ast ;
      }

      INT32 execute( _pmdEDUCB *cb ) ;

      INT32 fetch( BSONObj &obj ) ;


   private:
      SQL_AST _ast ;
      qgmParamTable _paramT ;
      qgmPtrTable _ptrT ;
      qgmPlan *_plan ;
   } ;

   typedef class _qgmPlanContainer qgmPlanContainer ;

   INT32 qgmDump ( _qgmPlanContainer *op, CHAR *pBuffer,
                   INT32 bufferSize ) ;
}

#endif

