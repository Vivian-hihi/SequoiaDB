/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = rtnSQLAvg.hpp

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

#ifndef RTNSQLAVG_HPP_
#define RTNSQLAVG_HPP_

#include "rtnSQLFunc.hpp"

namespace engine
{
   class _rtnSQLAvg : public _rtnSQLFunc
   {
   public:
      _rtnSQLAvg() ;
      virtual ~_rtnSQLAvg() ;

   public:
      virtual INT32 result( BSONObjBuilder &builder ) ;

   private:
      virtual INT32 _push( const RTN_FUNC_PARAMS &param ) ;

   private:
      FLOAT64 _total ;
      UINT64 _count ;
   } ;

   typedef class _rtnSQLAvg rtnSQLAvg ;
}

#endif

