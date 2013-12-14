/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = rtnSQLAvg.cpp

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

#include "rtnSQLAvg.hpp"

namespace engine
{
   _rtnSQLAvg::_rtnSQLAvg()
   :_total(0),
    _count(0)
   {
      _name = "avg" ;
   }

   _rtnSQLAvg::~_rtnSQLAvg()
   {

   }

   INT32 _rtnSQLAvg::result( BSONObjBuilder &builder )
   {
      INT32 rc = SDB_OK ;
      try
      {
         if ( 0 == _count )
         {
            builder.appendNull( _alias.toString() ) ;
         }
         else
         {
            FLOAT64 avg = _total / _count ;
            builder.append( _alias.toString(), avg ) ;
            _total = 0 ;
            _count = 0 ;
         }
      }
      catch ( std::exception &e )
      {
         PD_LOG( PDERROR, "unexpected err happened:%s",
                 e.what() ) ;
         rc = SDB_OK ;
         goto error ;
      }
   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _rtnSQLAvg::_push( const RTN_FUNC_PARAMS &param )
   {
      INT32 rc = SDB_OK ;

      try
      {
         const BSONElement &ele = *( param.begin() ) ;
         if ( !ele.eoo() && ele.isNumber() )
         {
            _total += ele.Number() ;
            ++_count ;
         }
      }
      catch ( std::exception &e )
      {
         PD_LOG( PDERROR, "unexpected err happened:%s",
                 e.what() ) ;
         rc = SDB_OK ;
         goto error ;
      }
   done:
      return rc ;
   error:
      goto done ;
   }


}
