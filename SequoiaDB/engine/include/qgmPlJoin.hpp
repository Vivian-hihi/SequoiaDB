/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = qgmOptiJoin.hpp

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

#ifndef QGMPLJOIN_HPP_
#define QGMPLJOIN_HPP_

#include "qgmPlan.hpp"

namespace engine
{
   class _qgmPlJoin : public _qgmPlan
   {
   public:
      _qgmPlJoin( INT32 joinType )
      :_qgmPlan( QGM_PLAN_TYPE_NLJOIN, _qgmField() ),
       _joinType( joinType ),
       _outerAlias( NULL ),
       _innerAlias( NULL ),
       _outer( NULL ),
       _inner( NULL )
      {

      }

      virtual ~_qgmPlJoin()
      {
         _joinType = SQL_GRAMMAR::SQLMAX ;
         _outerAlias = NULL ;
         _innerAlias = NULL ;
         _outer = NULL ;
         _inner = NULL ;
      }

   public:
      virtual string toString() const
      {
         return SQL_GRAMMAR::INNERJOIN == _joinType ?
                "Type:INNERJOIN\n" : "Type: LEFT OUTER JOIN\n" ;
      }

   protected:
      INT32 _joinType ;
      const qgmField *_outerAlias ;
      const qgmField *_innerAlias ;
      _qgmPlan *_outer ;
      _qgmPlan *_inner ;
   } ;
   typedef class _qgmPlJoin qgmPlJoin ;
}

#endif

