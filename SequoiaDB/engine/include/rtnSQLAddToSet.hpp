/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = rtnSQLAddToSet.hpp

   Descriptive Name =

   When/how to use: this program may be used on binary and text-formatted
   versions of PMD component. This file contains declare for QGM operators

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          09/05/2013  JHL  Initial Draft

   Last Changed =

*******************************************************************************/
#ifndef RTNSQLADDTOSET_HPP__
#define RTNSQLADDTOSET_HPP__

#include <set>
#include <string>
#include "../bson/bsonobj.h"
#include "rtnSQLFunc.hpp"

namespace engine
{

   class rtnSQLAddToSet : public _rtnSQLFunc
   {
   typedef std::set< bson::BSONElement >     FIELD_SET;
   typedef std::vector< bson::BSONObj >      OBJ_VEC;
   public:
      rtnSQLAddToSet();

      virtual ~rtnSQLAddToSet();

      virtual INT32 result( bson::BSONObjBuilder &builder );

   private:
      virtual INT32 _push( const RTN_FUNC_PARAMS &param );

   private:
      bson::BSONArrayBuilder     *_pArrBuilder;
      FIELD_SET                  _fieldSet;
      OBJ_VEC                    _objVec;
   };
}

#endif

