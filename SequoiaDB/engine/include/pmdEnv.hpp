/*******************************************************************************
   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = pmdEnv.hpp

   Descriptive Name = Process MoDel Main

   When/how to use: this program may be used on binary and text-formatted
   versions of PMD component. This file contains main function for SequoiaDB,
   and all other process-initialization code.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          22/04/2014  XJH Initial Draft

   Last Changed =

*******************************************************************************/
#ifndef PMDENV_HPP_
#define PMDENV_HPP_

#include "pmdCommon.hpp"

using namespace bson ;

namespace engine
{

   /*
      pmd system info define
   */
   typedef struct _pmdSysInfo
   {
      SDB_ROLE                      _dbrole ;
      MsgRouteID                    _nodeID ;

      _pmdSysInfo()
      {
         _dbrole        = SDB_ROLE_STANDALONE ;
         _nodeID.value  = MSG_INVALID_ROUTEID ;
      }
   } pmdSysInfo ;

   SDB_ROLE       pmdGetDBRole() ;
   void           pmdSetDBRole( SDB_ROLE role ) ;
   MsgRouteID     pmdGetNodeID() ;
   void           pmdSetNodeID( MsgRouteID id ) ;

   pmdSysInfo*    pmdGetSysInfo () ;

   /*
      pmd trap functions
   */
   INT32    pmdEnableSignalEvent( const CHAR *filepath ) ;

   INT32&   pmdGetSigNum() ;

   /*
      Env define
   */
   #define  PMD_SIGNUM                 pmdGetSigNum()

}

#endif //PMDENV_HPP_

