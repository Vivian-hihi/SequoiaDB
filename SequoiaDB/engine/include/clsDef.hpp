/*******************************************************************************
   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = clsDef.hpp

   Descriptive Name =

   When/how to use: this program may be used on binary and text-formatted
   versions of Replication component. This file contains structure for
   replication control block.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          09/14/2012  YW  Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef CLSDEF_HPP_
#define CLSDEF_HPP_

#include "core.hpp"
#include "oss.hpp"
#include "dpsLogDef.hpp"
#include "netDef.hpp"
#include "msgDef.h"
#include "ossMem.hpp"
#include "msg.h"
#include "pmdEDU.hpp"
#include "ossRWMutex.hpp"

#include <map>

using namespace std ;

namespace engine
{
   typedef UINT32 CLS_GROUP_VERSION ;

//   const UINT32 CLS_VOTE_SILENCE_TIME = 5000 ;
   const UINT32 CLS_VOTE_CS_TIME = 3000 ;
   const UINT32 CLS_SHARING_BETA_INTERVAL = 2000 ;
   const UINT32 CLS_IS_PRIMARY = 0 ;
   const UINT32 CLS_NOT_PRIMARY = 1 ;
   const UINT32 CLS_SYNC_MAX_LEN = 1024 * 1024 * 5 ;
   extern UINT32 CLS_SHARING_BRK_TIME ;
   extern INT32  g_startShiftTime ;


   #define CLS_TID(sessionid)          ((UINT32)(sessionid & 0xFFFFFFFF))
   #define CLS_NODEID(sessionid)       ((UINT32)(sessionid >> 32))


   //full sync node timeout
   #define CLS_FS_NORES_TIMEOUT 10000
   #define CLS_DST_SESSION_NO_MSG_TIME          (300000)
   #define CLS_SRC_SESSION_NO_MSG_TIME          (10000)

   enum CLS_SYNC_STATUS
   {
      CLS_SYNC_STATUS_NONE = 0,
      CLS_SYNC_STATUS_PEER = 1,
      CLS_SYNC_STATUS_RC = 2,
   } ;

   enum CLS_BEAT_STATUS
   {
      CLS_BEAT_STATUS_BREAK = 0,
      CLS_BEAT_STATUS_ALIVE = 1,
   } ;

   const UINT32 CLS_BEAT_ID_OVERTURN_WINDOW = 2 ^ 31 ;

   enum CLS_GROUP_ROLE
   {
      CLS_GROUP_ROLE_SECONDARY = 0,
      CLS_GROUP_ROLE_PRIMARY =1,
   } ;

   enum CLS_ELECTION_STATUS
   {
      CLS_ELECTION_STATUS_SILENCE = 0,
      CLS_ELECTION_STATUS_SEC = 1,
      CLS_ELECTION_STATUS_VOTE = 2,
      CLS_ELECTION_STATUS_ANNOUNCE = 3,
      CLS_ELECTION_STATUS_PRIMARY = 4,
   } ;

   enum CLS_BS_STATUS
   {
      CLS_BS_CLOSED           = 0,
      CLS_BS_NORMAL,
      CLS_BS_FULLSYNC,
      CLS_BS_BACKUPOFFLINE,
   } ;

   class _clsGroupBeat : public SDBObject
   {
   public :
//      DPS_LSN _beginLsn ;
      DPS_LSN endLsn ;
      _MsgRouteID identity ;
      UINT64 timeStamp ;
      CLS_GROUP_VERSION version ;
      CLS_GROUP_ROLE role ;
      CLS_SYNC_STATUS syncStatus ;
      UINT32 beatID ;
      _clsGroupBeat():timeStamp(0),
                       version(0),
                       role(CLS_GROUP_ROLE_SECONDARY),
                       syncStatus(CLS_SYNC_STATUS_NONE),
                       beatID(0)
      {

      }

      BOOLEAN isValidID( const UINT32 &id )
      {
         if ( beatID < id )
         {
            return TRUE ;
         }
         else if ( (beatID - id) >
                    CLS_BEAT_ID_OVERTURN_WINDOW )
         {
            return TRUE ;
         }
         else
         {
            return FALSE ;
         }
      }
   } ;

   class _clsSharingStatus : public SDBObject
   {
   public:
      _clsGroupBeat beat ;
      UINT32 timeout ;
      _clsSharingStatus():timeout(CLS_SHARING_BRK_TIME)
      {
         SDB_ASSERT( 0 != CLS_SHARING_BRK_TIME, "impossible" )
      }
   } ;

   class _clsGroupInfo : public SDBObject
   {
   public :
      map<UINT64, _clsSharingStatus> info ;
      map<UINT64, _clsSharingStatus *> alives ;
      ossRWMutex mtx ;
      _MsgRouteID primary ;
      _MsgRouteID local ;
      UINT32 localBeatID ;
      CLS_GROUP_VERSION version ;
      _clsGroupInfo():localBeatID( 0 ),
                      version( 0 )
      {
         local.value = 0 ;
         primary.value = 0 ;
      }
      ~_clsGroupInfo()
      {
         alives.clear() ;
         info.clear() ;
      }

      UINT32 groupSize ()
      {
         return info.size() + 1 ;
      }

      UINT32 aliveSize ()
      {
         return alives.size() + 1 ;
      }

      BOOLEAN isAllNodeBeat()
      {
         map<UINT64, _clsSharingStatus>::iterator it = info.begin() ;
         while ( it != info.end() )
         {
            _clsSharingStatus &status = it->second ;
            if ( 0 == status.beat.beatID )
            {
               return FALSE ;
            }
            ++it ;
         }
         return TRUE ;
      }
   } ;


   enum CLS_ELECTION_ROUND
   {
      CLS_ELECTION_ROUND_STAGE_ONE = 0,
      CLS_ELECTION_ROUND_STAGE_TWO = 1,
   } ;

   class _clsSyncSession : public SDBObject
   {
   public :
      DPS_LSN_OFFSET endLsn ;
      _pmdEDUCB *eduCB ;

      /// local write has been completed.
      /// synced starts from one.
      _clsSyncSession():endLsn(DPS_INVALID_LSN_OFFSET), eduCB( NULL)
      {}

      BOOLEAN operator<( const _clsSyncSession &session )
      {
         return endLsn < session.endLsn ;
      }

      BOOLEAN operator<=( const _clsSyncSession &session )
      {
         return endLsn <= session.endLsn ;
      }
   } ;


   #define CLS_SAME_SYNC_LSN_MAX_TIMES    (20)

   class _clsSyncStatus : public SDBObject
   {
   public :
      DPS_LSN_OFFSET    offset ;
      _MsgRouteID       id ;
      BOOLEAN           valid ;
      UINT32            sameReqTimes ;

      _clsSyncStatus():offset(0)
      {
         id.value       = 0 ;
         valid          = TRUE ;
         sameReqTimes   = 0 ;
      }

      _clsSyncStatus& operator=( const _clsSyncStatus &right )
      {
         offset         = right.offset ;
         id.value       = right.id.value ;
         valid          = right.valid ;
         sameReqTimes   = right.sameReqTimes ;

         return *this ;
      }

      BOOLEAN isValid() const
      {
         // 1. already full sync
         // 2. sharing-break
         // 3. same sync req more than 20 times
         if ( DPS_INVALID_LSN_OFFSET == offset ||
              !valid ||
              sameReqTimes > CLS_SAME_SYNC_LSN_MAX_TIMES )
         {
            return FALSE ;
         }
         return TRUE ;
      }
   } ;

}

#endif

