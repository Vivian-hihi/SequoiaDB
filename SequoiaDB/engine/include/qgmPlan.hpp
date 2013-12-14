/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = qgmPlan.hpp

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

#ifndef QGMPLAN_HPP_
#define QGMPLAN_HPP_

#include "qgmDef.hpp"
#include "pd.hpp"
#include "qgmParamTable.hpp"

namespace engine
{
   enum QGM_PLAN_TYPE
   {
      QGM_PLAN_TYPE_RETURN = 0,
      QGM_PLAN_TYPE_FILTER,
      QGM_PLAN_TYPE_SCAN,
      QGM_PLAN_TYPE_NLJOIN,
      QGM_PLAN_TYPE_INSERT,
      QGM_PLAN_TYPE_UPDATE,
      QGM_PLAN_TYPE_AGGR,
      QGM_PLAN_TYPE_SORT,
      QGM_PLAN_TYPE_DELETE,
      QGM_PLAN_TYPE_COMMAND,
      QGM_PLAN_TYPE_SPLIT,
      QGM_PLAN_TYPE_HASHJOIN,

      //
      QGM_PLAN_TYPE_MAX,
   } ;

   class _qgmPlan ;
   typedef vector<_qgmPlan *> QGM_PINPUT ;

   class _pmdEDUCB ;

   class _qgmPlan : public SDBObject
   {
   public:
      _qgmPlan( QGM_PLAN_TYPE type, const qgmField &alias ) ;
      virtual ~_qgmPlan() ;

   public:
      virtual void close() ;

      virtual string toString() const { return "" ;}

      inline QGM_PLAN_TYPE type() const { return _type ; }

      inline void merge( BOOLEAN ifMerge ){ _merge = ifMerge ; }

      inline const qgmField &alias() const { return _alias ; }

      inline UINT32 inputSize() const { return _input.size() ; }

      inline BOOLEAN ready(){ return _initialized ;}

      inline void setParamTable( _qgmParamTable *param )
      {
         SDB_ASSERT( NULL != param, "impossible" )
         _param = param ;
         return ;
      }

      inline _qgmPlan *input( UINT32 id ) const
      {
         return id <= _input.size() ?
                _input.at( id ) : NULL ;
      }

      inline void addVar( const varItem &item )
      {
         _varlist.push_back( item ) ;
         return ;
      }

      inline void setVar( const QGM_VARLIST &list )
      {
         _varlist = list ;
         return ;
      }

      INT32 addChild( _qgmPlan *child ) ;

      INT32 execute( _pmdEDUCB *eduCB ) ;

      INT32 fetchNext( qgmFetchOut &next ) ;

   protected:
      void _modifyAttrAlias(  ) ;

   private:
      virtual INT32 _execute( _pmdEDUCB *eduCB ) = 0 ;

      virtual INT32 _fetchNext( qgmFetchOut &next ) = 0 ;

   protected:
      QGM_VARLIST _varlist ;
      QGM_PINPUT _input ;
      QGM_PLAN_TYPE _type ;
      _pmdEDUCB *_eduCB ;
      qgmField _alias ;
      BOOLEAN _executed ;
      BOOLEAN _initialized ;
      BOOLEAN _merge ;
      _qgmParamTable *_param ;
   } ;

   typedef class _qgmPlan qgmPlan ;
}
#endif

