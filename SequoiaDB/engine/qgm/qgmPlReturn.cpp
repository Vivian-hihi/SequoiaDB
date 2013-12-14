/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = qgmPlReturn.cpp

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

#include "qgmPlReturn.hpp"

namespace engine
{
   _qgmPlReturn::_qgmPlReturn()
   :_qgmPlan( QGM_PLAN_TYPE_RETURN, qgmField() )
   {
      _initialized = TRUE ;
   }

   _qgmPlReturn::~_qgmPlReturn()
   {

   }

   INT32 _qgmPlReturn::_execute( _pmdEDUCB *eduCB )
   {
      return input(0)->execute( eduCB ) ;
   }

   INT32 _qgmPlReturn::_fetchNext( qgmFetchOut &next )
   {
      return input(0)->fetchNext( next ) ;
   }
}

