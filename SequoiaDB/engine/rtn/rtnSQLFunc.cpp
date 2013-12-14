/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = rtnSQLFunc.cpp

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

#include "rtnSQLFunc.hpp"
#include <sstream>

namespace engine
{
   string _rtnSQLFunc::toString() const
   {
      stringstream ss ;

      ss << "{name:"
         << _name ;
      if ( !_alias.empty() )
      {
         ss << ",alias:"
            << _alias.toString() ;
      }
      if ( !_param.empty() )
      {
         ss << ",param:[" ;
         vector<qgmOpField>::const_iterator itr = _param.begin() ;
         for ( ; itr != _param.end(); itr++ )
         {
            ss <<"{" << itr->toString() << "}," ;
         }
         ss.seekp((INT32)ss.tellp()-1 ) ;
         ss << "]" ;
      }

      return ss.str() ;
   }
}
