/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = clsBase.hpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          29/11/2012  Xu Jianhui  Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef CLS_BASE_HPP_
#define CLS_BASE_HPP_

#include "core.hpp"
#include "msg.hpp"

namespace engine
{

   enum CLS_MEMBER_TYPE
   {
      CLS_SHARD   = 1,           //shard
      CLS_REPL    = 2            //repl
   };

   enum CLS_SESSION_START_TYPE
   {
      CLS_SESSION_ACTIVE   = 1,  //active
      CLS_SESSION_PASSIVE  = 2   //passive
   };

   enum CLS_INNER_SESSION_TID
   {
      CLS_TID_REPL_SYC     = 1,  //repl active sync
      CLS_TID_REPL_FS_SYC  = 2,  //repl active full sync

      CLS_TID_END          = 10  //end
   };

   #define CLS_INVALID_TIMERID         (0)

   #define CLS_BASE_HANDLE_ID          (70000)


   enum CLS_SYNC_STRATEGY
   {
      CLS_SYNC_NONE        = 0,
      CLS_SYNC_KEEPNORMAL  = 1,
      CLS_SYNC_KEEPALL     = 2
   } ;
   #define CLS_SYNC_DTF_STRATEGY    CLS_SYNC_NONE

   typedef MsgRouteID   NodeID ;
   #define INVALID_NODE_ID       (MSG_INVALID_ROUTEID)

   #define SAFE_DELETE(p) \
      do { \
         if ( p ) \
         { \
            SDB_OSS_DEL p ; \
            p = NULL ; \
         } \
      } while (0)

   #define SAFE_NEW_GOTO_ERROR(p, className) \
      do { \
         p = SDB_OSS_NEW className() ; \
         if ( !p ) \
         { \
            PD_LOG ( PDERROR, "Failed to allocate memory to #className" ) ; \
            rc = SDB_OOM ; \
            goto error ; \
         } \
      } while (0)

   #define SAFE_NEW_GOTO_ERROR1(p, className, arg1) \
      do { \
         p = SDB_OSS_NEW className( arg1 ) ; \
         if ( !p ) \
         { \
            PD_LOG ( PDERROR, "Failed to allocate memory to #className" ) ; \
            rc = SDB_OOM ; \
            goto error ; \
         } \
      } while (0)

   #define SAFE_NEW_GOTO_ERROR2(p, className, arg1, arg2) \
      do { \
         p = SDB_OSS_NEW className( arg1, arg2 ) ; \
         if ( !p ) \
         { \
            PD_LOG ( PDERROR, "Failed to allocate memory to #className" ) ; \
            rc = SDB_OOM ; \
            goto error ; \
         } \
      } while (0)

   #define INIT_OBJ_GOTO_ERROR(p) \
      do { \
         if ( SDB_OK != ( rc = p->initialize () ) ) \
         { \
            PD_LOG ( PDERROR, "init failed" ) ; \
            goto error ; \
         } \
      } while (0)

   #define DO_GOTO_ERROR(exp) \
      if ( !(exp) ) \
      { \
         goto error ; \
      }

}

#endif //CLS_BASE_HPP_



