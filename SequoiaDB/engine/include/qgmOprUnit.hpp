/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = qgmOprUnit.hpp

   Descriptive Name =

   When/how to use: this program may be used on binary and text-formatted
   versions of PMD component. This file contains functions for agent processing.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          05/04/2012  XJH Initial Draft

   Last Changed =

******************************************************************************/

#ifndef QGMOPR_UNIT_HPP_
#define QGMOPR_UNIT_HPP_

#include "core.hpp"
#include "pd.hpp"
#include "ossUtil.hpp"
#include <vector>

#include "qgmOptiTree.hpp"
#include "qgmConditionNodeHelper.hpp"
#include "qgmOptiAggregation.hpp"

using namespace std ;

namespace engine
{

   #define QGM_OPR_FILTER_COPY_FLAG          ( FALSE )

   enum OPR_FILTER_TYPE
   {
      FILTER_SEC,       // SELECT
      FILTER_CON,       // CONDITION
      FILTER_BOTH       // SELECT+CONDITION
   };
   class _qgmFilterUnit : public _qgmOprUnit
   {
      public:
         _qgmFilterUnit ( QGM_OPTI_TYPE type ) ;
         _qgmFilterUnit ( const _qgmFilterUnit &right,
                          BOOLEAN delDup = FALSE ) ;
         virtual ~_qgmFilterUnit () ;

         virtual BOOLEAN isOptional() const ;
         BOOLEAN isFieldsOptional() const ;

         BOOLEAN hasCondition() const { return _conds.size() > 0 ; }

      public:
         OPR_FILTER_TYPE filterType () const ;

         INT32 setCondition( qgmConditionNode *cond ) ;
         INT32 setCondition( qgmConditionNodePtrVec &conds ) ;
         INT32 addCondition( qgmConditionNode *cond ) ;
         INT32 addCondition( qgmConditionNodePtrVec &conds ) ;
         void  emptyCondition() ;
         INT32 removeCondition( qgmConditionNode *cond ) ;
         INT32 removeCondition( qgmConditionNodePtrVec &conds ) ;

         qgmConditionNode* getMergeCondition() ;
         qgmConditionNodePtrVec getConditions() ;

         INT32 getCondFields( qgmDbAttrPtrVec &fields ) ;

      protected:
         virtual void   _toString( stringstream &ss ) const ;
         virtual INT32  _replaceRele( const qgmField &newRele ) ;
         virtual INT32  _replaceFieldAlias( const qgmOPFieldPtrVec &fieldAlias ) ;
         virtual INT32  _restoreFieldAlias( const qgmOPFieldPtrVec &fieldAlias ) ;

         INT32    _separateCond( qgmConditionNode *cond,
                                 qgmConditionNodePtrVec &conds ) ;

         BOOLEAN  _hasFieldAlias() const ;

      protected:
         qgmConditionNodePtrVec     _conds ;

   };
   typedef _qgmFilterUnit qgmFilterUnit ;

   class _qgmAggrUnit : public _qgmOprUnit
   {
      public:
         _qgmAggrUnit( QGM_OPTI_TYPE type ) ;
         _qgmAggrUnit( const _qgmAggrUnit &right,
                       BOOLEAN delDup = FALSE ) ;
         virtual ~_qgmAggrUnit() ;

      public:
         void     clearAggrSelector() { _aggrSelect.clear() ; }
         INT32    addAggrSelector( const qgmAggrSelector &selector ) ;
         INT32    addAggrSelector( const qgmAggrSelectorVec &selectors ) ;
         qgmAggrSelectorVec *getAggrSelector() { return &_aggrSelect ; }

      protected:
         virtual void   _toString( stringstream &ss ) const ;
         virtual INT32  _replaceRele( const qgmField &newRele ) ;
         virtual INT32  _replaceFieldAlias( const qgmOPFieldPtrVec &fieldAlias ) ;
         virtual INT32  _restoreFieldAlias( const qgmOPFieldPtrVec &fieldAlias ) ;

      protected:
         qgmAggrSelectorVec         _aggrSelect ;

   };
   typedef _qgmAggrUnit qgmAggrUnit ;

}

#endif //QGMOPR_UNIT_HPP_

