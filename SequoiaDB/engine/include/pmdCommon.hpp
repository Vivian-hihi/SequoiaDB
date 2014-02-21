/*******************************************************************************
   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = pmdMain.cpp

   Descriptive Name = Process MoDel Main

   When/how to use: this program may be used on binary and text-formatted
   versions of PMD component. This file contains main function for SequoiaDB,
   and all other process-initialization code.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          09/14/2012  TW  Initial Draft

   Last Changed =

*******************************************************************************/
#ifndef PMDCOMMON_HPP_
#define PMDCOMMON_HPP_

#include "core.hpp"
#include "pmd.hpp"

namespace engine
{
   INT32 pmdBuildFullPath( const CHAR *path, const CHAR *name,
                           UINT32 fullSize, CHAR *full );

   SDB_ROLE pmdGetRoleEnum( const CHAR *role ) ;

   INT32 pmdPrefReplStr2Enum( const CHAR *prefReplStr ) ;

   INT32 pmdPrefReplEnum2Str( INT32 enumPrefRepl,
                              CHAR *prefReplStr,
                              UINT32 len ) ;


}



#endif //PMDCOMMON_HPP_

