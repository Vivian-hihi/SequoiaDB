/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = rtnSQLFuncFactory.hpp

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

#ifndef RTNSQLFUNCFACTORY_HPP_
#define RTNSQLFUNCFACTORY_HPP_

#include "rtnSQLFunc.hpp"

namespace engine
{
   class _rtnSQLFunc ;

   class _rtnSQLFuncFactory : public SDBObject
   {
   public:
      INT32 create( const CHAR *name,
                    UINT32 paramNum,
                    _rtnSQLFunc *&func ) ;
   } ;

   typedef class _rtnSQLFuncFactory rtnSQLFuncFactory ;
}

#endif

