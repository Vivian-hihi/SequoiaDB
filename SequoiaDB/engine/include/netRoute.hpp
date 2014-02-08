/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = netRoute.hpp

   Descriptive Name =

   When/how to use: this program may be used on binary and text-motionatted
   versions of PD component. This file contains declare of PD functions.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          09/14/2012  YW  Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef NETROUTE_HPP_
#define NETROUTE_HPP_

#include "core.hpp"
#include "oss.hpp"
#include "ossLatch.hpp"
#include "netDef.hpp"
#include <map>

using namespace std ;

namespace engine
{
   class _netRoute : public SDBObject
   {
      public:
         ~_netRoute() ;
         INT32 route( const _MsgRouteID &id,
                      CHAR *host,
                      CHAR *service ) ;

         INT32 route( const _MsgRouteID &id,
                      _netRouteNode &node ) ;

         /// return err when update an existing node.
         INT32 update( const _MsgRouteID &id,
                       const CHAR *host,
                       const CHAR *service,
                       BOOLEAN *newAdd = NULL ) ;
         INT32 update( const _MsgRouteID &id,
                       const _netRouteNode &node ) ;
         INT32 update( const _MsgRouteID &oldID,
                       const _MsgRouteID &newID ) ;

         void clear() ;

         inline void setLocal( const _MsgRouteID &id )
         {
            _local = id ;
         }

         inline const _MsgRouteID &local()
         {
            return _local ;
         }

      private:
         map<UINT64, _netRouteNode> _route ;
         _MsgRouteID _local ;
         _ossSpinSLatch _mtx ;
   };
}

#endif

