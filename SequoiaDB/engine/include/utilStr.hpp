/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = utilStr.hpp

   Descriptive Name =

   When/how to use: str util

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          09/14/2012  YW  Initial Draft

   Last Changed =

******************************************************************************/

#ifndef UTILSTR_HPP_
#define UTILSTR_HPP_

#include "core.hpp"

namespace engine
{
   /// skip spaces at begin.
   INT32 utilStrTrimBegin( const CHAR *src, const CHAR *&begin ) ;

   /// remove spaces at end.
   INT32 utilStrTrimEnd( CHAR *src ) ;

   /// trim spaces at begin or end .
   INT32 utilStrTrim( CHAR *src, const CHAR *&begin ) ;

   INT32 utilStrToUpper( const CHAR *src, CHAR *&upper ) ;

   INT32 utilStrJoin( const CHAR **src,
                      UINT32 cnt,
                      CHAR *join,
                      UINT32 &joinSize ) ;

   INT32 utilStr2TimeT( const CHAR *str,
                        time_t &tm,
                        UINT64 *usec = NULL ) ;
}

#endif

