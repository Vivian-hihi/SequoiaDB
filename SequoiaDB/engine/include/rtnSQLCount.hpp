/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = rtnSQLCount.hpp

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

#ifndef RTNSQLCOUNT_HPP_
#define RTNSQLCOUNT_HPP_

#include "rtnSQLFunc.hpp"

namespace engine
{
   class _rtnSQLCount : public _rtnSQLFunc
   {
   public:
      _rtnSQLCount() ;
      virtual ~_rtnSQLCount() ;

   public:
      virtual INT32 result( BSONObjBuilder &builder ) ;

   private:
      virtual INT32 _push( const RTN_FUNC_PARAMS &param ) ;

   private:
      SINT64 _count ;
   } ;

   typedef class _rtnSQLCount rtnSQLCount ;
}

#endif

