/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = qgmPlUpdate.hpp

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

#ifndef QGMPLUPDATE_HPP_
#define QGMPLUPDATE_HPP_

#include "qgmPlan.hpp"
#include "qgmUtil.hpp"

namespace engine
{
   struct _qgmConditionNode;

   class _qgmPlUpdate : public _qgmPlan
   {
   public:
      _qgmPlUpdate( const _qgmDbAttr &collection,
                    const qgmDbAttrVec &columns,
                    const qgmOPFieldVec &values,
                    _qgmConditionNode *condition ) ;

      virtual ~_qgmPlUpdate() ;

   public:
      virtual string toString() const
      {
         stringstream ss ;
         ss << "Type:" << qgmPlanType( _type ) << '\n';
         ss << "Updator:" << _updater.toString() << '\n';
         ss << "Condition:" << _condition.toString() << '\n';
         return ss.str() ;
      }

   private:
      virtual INT32 _execute( _pmdEDUCB *eduCB ) ;

      virtual INT32 _fetchNext ( qgmFetchOut &next )
      {
         return SDB_SYS ;
      }

   private:
      _qgmDbAttr _collection ;
      BSONObj _updater ;
      BSONObj _condition ;
   } ;

   typedef class _qgmPlUpdate qgmPlUpdate ;
}

#endif

