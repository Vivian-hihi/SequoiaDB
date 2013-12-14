/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = qgmPlAggregation.hpp

   Descriptive Name =

   When/how to use: this program may be used on binary and text-formatted
   versions of PMD component. This file contains declare for QGM operators

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          04/09/2013  YW  Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef QGMPLAGGREGATION_HPP_
#define QGMPLAGGREGATION_HPP_

#include "qgmPlan.hpp"
#include "qgmOptiAggregation.hpp"
#include "rtnSQLFunc.hpp"
#include "qgmSelector.hpp"

namespace engine
{
   class _qgmPlAggregation : public _qgmPlan
   {
   public:
      _qgmPlAggregation( const qgmAggrSelectorVec &selector,
                         const qgmOPFieldVec &groupby,
                         const qgmField &alias,
                         _qgmPtrTable *table ) ;

      virtual ~_qgmPlAggregation() ;

   public:
      virtual string toString() const ;

   private:
      virtual INT32 _execute( _pmdEDUCB *eduCB ) ;

      virtual INT32 _fetchNext( qgmFetchOut &next ) ;

   private:
      INT32 _push( const qgmFetchOut &next ) ;

      INT32 _result( qgmFetchOut &result ) ;

      INT32 _select( const qgmFetchOut &next,
                     const vector<qgmOpField> &fields,
                     RTN_FUNC_PARAMS &param ) ;

   private:
      std::vector<_rtnSQLFunc *> _func ;
      _qgmSelector _groupby ;
      BSONObj _groupbyKey ;
      BSONObj _preObj ;
      BOOLEAN _eoc ;
      UINT32  _pushCount ;
      BOOLEAN _isAggr;
   } ;

   typedef class _qgmPlAggregation qgmPlAggregation ;
}

#endif

