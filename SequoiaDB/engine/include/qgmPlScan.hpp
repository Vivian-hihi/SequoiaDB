/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = qgmPlScan.hpp

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

#ifndef QGMPLSCAN_HPP_
#define QGMPLSCAN_HPP_

#include "qgmPlan.hpp"
#include "rtn.hpp"
#include "rtnCoordQuery.hpp"
#include "msg.h"
#include "qgmSelector.hpp"

namespace engine
{
   struct _qgmConditionNode ;
   class netMultiRouteAgent ;

   class _qgmPlScan : public _qgmPlan
   {
   public:
      //  element in orderby  does not include the collection name.
      _qgmPlScan( const qgmDbAttr &collection,

                  // element should be:
                  // field:alias
                  const qgmOPFieldVec &selector,
                  const BSONObj &orderby,
                  const BSONObj &hint,
                  INT64 numSkip,
                  INT64 numReturn,
                  const qgmField &alias,
                  _qgmConditionNode *condition ) ;

      virtual ~_qgmPlScan() ;

   public:
      virtual void close() ;

      virtual string toString() const ;

      const qgmDbAttr &collection(){ return _collection ; }

   protected:
      INT32 _executeOnData( _pmdEDUCB *eduCB ) ;

      INT32 _executeOnCoord( _pmdEDUCB *eduCB ) ;

   private:
      virtual INT32 _execute( _pmdEDUCB *eduCB ) ;

      virtual INT32 _fetchNext ( qgmFetchOut &next ) ;

      void _killContext() ;
      INT32 _fetch( const CHAR *&result ) ;

   protected:
      BSONObj _condition ;
      BOOLEAN _invalidPredicate ;
      SINT64 _contextID ;
      SDB_ROLE _dbRole ;

   private:
      qgmDbAttr _collection ;
      qgmSelector _selector ;
      BSONObj _orderby ;
      BSONObj _hint ;
      INT64 _skip ;
      INT64 _return ;

      /// if it is a coord
      rtnCoordQuery _coordQuery ;

      /// if it is a data
      SDB_DMSCB *_dmsCB ;
      SDB_RTNCB *_rtnCB ;

      _qgmConditionNode *_conditionNode ;
   } ;
   typedef class _qgmPlScan qgmPlScan ;
}

#endif

