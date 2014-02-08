/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = rtnSQLBuildObj.hpp

   Descriptive Name =

   When/how to use: this program may be used on binary and text-formatted
   versions of PMD component. This file contains declare for QGM operators

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          09/16/2013  JHL  Initial Draft

   Last Changed =

*******************************************************************************/
#ifndef RTNSQLBUILDOBJ_HPP__
#define RTNSQLBUILDOBJ_HPP__

#include "rtnSQLFunc.hpp"
#include "../bson/bsonobj.h"

namespace engine
{
   class rtnSQLBuildObj : public _rtnSQLFunc
   {
   public:
      rtnSQLBuildObj();

      virtual ~rtnSQLBuildObj();

      virtual INT32 result( bson::BSONObjBuilder &builder );

      virtual BOOLEAN isAggr()
      {
         return FALSE;
      }

   private:
      virtual INT32 _push( const RTN_FUNC_PARAMS &param );

   private:
      bson::BSONObj     _obj;
      BOOLEAN           _hasData;
   };
}

#endif

