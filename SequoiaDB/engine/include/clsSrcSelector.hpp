/*******************************************************************************
  OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = clsSrcSelector.hpp

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

#ifndef CLSSRCSELECTOR_HPP_
#define CLSSRCSELECTOR_HPP_

#include "core.hpp"
#include "oss.hpp"
#include "msg.hpp"
#include "clsDef.hpp"
#include <set>

using namespace std ;

namespace engine
{
   class _clsSyncManager ;
   class _clsReplicateSet ;
   class _clsNodeMgrAgent ;

   class _clsSrcSelector : public SDBObject
   {
   public:
      _clsSrcSelector() ;
      ~_clsSrcSelector() ;

   public:
      const MsgRouteID &getFullSyncSrc() ;

      const MsgRouteID &getSyncSrc() ;

      const MsgRouteID &selected( BOOLEAN isFullSync = FALSE ) ;

      const MsgRouteID &selectPrimary ( UINT32 groupID,
                                        MSG_ROUTE_SERVICE_TYPE type =
                                        MSG_ROUTE_SHARD_SERVCIE ) ;

      inline void addToBlakList( const MsgRouteID &id )
      {
         _blacklist.insert( id.value ) ;
      }

      inline void clearBlacklist()
      {
         _blacklist.clear() ;
      }

      inline void clearSrc()
      {
         _src.value = MSG_INVALID_ROUTEID ;
         _noRes = 0 ;
      }

      inline void timeout( UINT32 timeout )
      {
         _noRes += timeout ;
      }

      inline void clearTime()
      {
         _noRes = 0 ;
      }

      inline const MsgRouteID &src()
      {
         return _src ;
      }
   private:
      set<UINT64>       _blacklist ;
      _clsSyncManager   *_syncmgr ;
      MsgRouteID        _src ;
      UINT32            _noRes ;
      _clsNodeMgrAgent  *_nodeMgrAgent ;

   } ;
   typedef class _clsSrcSelector clsSrcSelector ;
}

#endif

