/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = rtnSQLSum.cpp

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

#include "rtnSQLSum.hpp"

namespace engine
{
   _rtnSQLSum::_rtnSQLSum()
   :_sum(0),
    _effective( FALSE )
   {
      _name = "sum" ;
   }

   _rtnSQLSum::~_rtnSQLSum()
   {

   }

   INT32 _rtnSQLSum::result( BSONObjBuilder &builder )
   {
      INT32 rc = SDB_OK ;
      try
      {
         if ( !_effective )
         {
            //builder.appendNull( _alias.toString() ) ;
            builder.append( _alias.toString(), 0 ) ;
         }
         else
         {
            builder.append( _alias.toString(), _sum ) ;
            _effective = FALSE ;
            _sum = 0 ;
         }
      }
      catch ( std::exception &e )
      {
         PD_LOG( PDERROR, "unexcepted err happened%s",
                 e.what() ) ;
         rc = SDB_SYS ;
         goto error ;
      }
   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _rtnSQLSum::_push( const RTN_FUNC_PARAMS &param )
   {
      INT32 rc = SDB_OK ;
      const BSONElement &ele = *(param.begin()) ;
      if ( !ele.eoo() && ele.isNumber() )
      {
         _sum += ele.Number() ;
         _effective = TRUE ;
      }
   done:
      return rc ;
   }
}
