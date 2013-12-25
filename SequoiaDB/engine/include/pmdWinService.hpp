/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = pmdDaemon.hpp

   Descriptive Name = pmdDaemon

   When/how to use: this program may be used on binary and text-formatted
   versions of PMD component. This file contains main function for sdbcm,
   which is used to do cluster managing.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          12/09/2013  JHL  Initial Draft

   Last Changed =

*******************************************************************************/
#ifndef PMDWINSERVICE_HPP__
#define PMDWINSERVICE_HPP__
#include "ossFeat.h"

#if defined (_WINDOWS)

#include "ossTypes.h"

typedef  INT32 (*PMD_WINSERVICE_FUNC)( INT32 argc, CHAR **argv ) ;

#define PMD_WINSVC_SVCNAME_MAX_LEN     255

INT32 WINAPI pmdWinstartService( const CHAR *pServiceName,
                        PMD_WINSERVICE_FUNC svcFun );

#endif //#if defined (_WINDOWS)

#endif //#define PMDWINSERVICE_HPP__