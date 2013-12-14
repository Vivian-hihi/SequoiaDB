/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = qgmOptiAggregation.hpp

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

#ifndef QGMOPTIAGGREGATION_HPP_
#define QGMOPTIAGGREGATION_HPP_

#include "qgmOptiTree.hpp"

namespace engine
{
   struct _qgmAggrSelector
   {
      qgmOpField value ;
      vector<qgmOpField> param ;

      _qgmAggrSelector()
      {
      }

      _qgmAggrSelector( const qgmOpField & field )
      {
         value = field ;
      }

      string toString() const
      {
         stringstream ss ;
         if ( SQL_GRAMMAR::DBATTR == value.type )
         {
            ss << "{value:" << value.value.toString()
               << ", alias:" << value.alias.toString()
               << "}" ;
         }
         else
         {
            ss << "{value:" << value.value.toString() ;
            if ( !value.alias.empty() )
            {
               ss << ", alias:" << value.alias.toString() ;
            }
            if ( !param.empty() )
            {
               ss << ", params:[" ;
               for ( vector<qgmOpField>::const_iterator itr = param.begin() ;
                     itr != param.end();
                     itr++ )
               {
                  ss << itr->value.toString() << ",";
               }
               ss.seekp((INT32)ss.tellp()-1 ) ;
               ss << "]" ;
            }
            ss << "}" ;
         }
         return ss.str() ;
      }
   } ;
   typedef struct _qgmAggrSelector qgmAggrSelector ;
   typedef std::vector< qgmAggrSelector > qgmAggrSelectorVec ;

   class _qgmOptiAggregation : public _qgmOptiTreeNode
   {
   public:
      _qgmOptiAggregation( _qgmPtrTable *table,
                           _qgmParamTable *param ) ;
      virtual ~_qgmOptiAggregation() ;

      virtual INT32        init () ;
      virtual INT32        done () ;

   public:
      virtual INT32 outputSort( qgmOPFieldVec &sortFields ) ;
      virtual INT32 outputStream( qgmOpStream &stream ) ;

      virtual string toString() const ;

      INT32 parse( const qgmOpField &field,
                   BOOLEAN &isFunc,
                   BOOLEAN needRele ) ;

      qgmAggrSelectorVec &aggrSelector()
      {
         return _selector ;
      }

      BOOLEAN isInAggrFieldAlias( const qgmDbAttr &field ) const ;

   protected:
      virtual UINT32 _getFieldAlias( qgmOPFieldPtrVec &fieldAlias,
                                     BOOLEAN getAll ) ;
      virtual INT32 _pushOprUnit( qgmOprUnit *oprUnit, PUSH_FROM from ) ;
      virtual INT32 _removeOprUnit( qgmOprUnit *oprUnit ) ;
      virtual INT32 _updateChange( qgmOprUnit *oprUnit ) ;

      INT32   _addFields( qgmOprUnit *oprUnit ) ;

      enum AGGR_TYPE { AGGR_GROUPBY, AGGR_SELECTOR } ;
      void    _update2Unit( AGGR_TYPE type ) ;

   public:
      qgmAggrSelectorVec _selector ;      /// parsed
      qgmOPFieldVec      _groupby ;

   private:
      BOOLEAN            _hasAggrFunc;

   } ;
   typedef class _qgmOptiAggregation qgmOptiAggregation ;
}

#endif

