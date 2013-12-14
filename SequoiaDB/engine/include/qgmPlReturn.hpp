/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = qgmPlReturn.hpp

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

#ifndef QGMPLRETURN_HPP_
#define QGMPLRETURN_HPP_

#include "qgmPlan.hpp"
#include "qgmUtil.hpp"
#include <sstream>

namespace engine
{
   class _qgmPlReturn : public _qgmPlan
   {
   public:
      _qgmPlReturn() ;
      virtual ~_qgmPlReturn() ;

   public:
     virtual string toString() const
     {
        std::stringstream ss ;
        ss << "Type:" << qgmPlanType( _type ) << '\n' ;
        return ss.str() ;
     }

   private:
      virtual INT32 _execute( _pmdEDUCB *eduCB ) ;

      virtual INT32 _fetchNext ( qgmFetchOut &next ) ;
   } ;
   typedef class _qgmPlReturn qgmPlReturn ;
}

#endif

