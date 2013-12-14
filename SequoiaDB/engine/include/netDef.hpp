/******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = pd.hpp

   Descriptive Name = Problem Determination Header

   When/how to use: this program may be used on binary and text-motionatted
   versions of PD component. This file contains declare of PD functions.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          09/14/2012  TW  Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef NETDEF_HPP_
#define NETDEF_HPP_

#include "core.hpp"
#include "oss.hpp"
#include "ossUtil.hpp"
#include "ossSocket.hpp"
#include "msg.hpp"
#include <string>

namespace engine
{
   typedef UINT32 NET_HANDLE ;

   const NET_HANDLE NET_INVALID_HANDLE = 0 ;

   typedef UINT32 NET_GROUP_ID ;
   typedef UINT32 NET_NODE_ID ;
   typedef UINT16 NET_SERVICE_ID ;


   class _netRouteNode : public SDBObject
   {
   public :
      CHAR _host[OSS_MAX_HOSTNAME+1] ;
      std::string _service[MSG_ROUTE_SERVICE_TYPE_MAX] ;
      MsgRouteID _id ;
      _netRouteNode()
      {
         _id.value = MSG_INVALID_ROUTEID ;
         _host[0] = 0 ;
      }
      _netRouteNode( const _netRouteNode &node )
      {
         _id = node._id ;
         ossMemcpy( _host, node._host, OSS_MAX_HOSTNAME+1 ) ;
         for ( UINT32 i = 0; i < MSG_ROUTE_SERVICE_TYPE_MAX; i++ )
         {
            _service[i] = node._service[i];
         }
      }

      const _netRouteNode &operator=(const _netRouteNode &node )
      {
         _id = node._id ;
         ossMemcpy( _host, node._host, OSS_MAX_HOSTNAME+1 ) ;
         for ( UINT32 i = 0; i < MSG_ROUTE_SERVICE_TYPE_MAX; i++ )
         {
            _service[i] = node._service[i];
         }
         return *this ;
      }
   } ;

   class _netIOV : public SDBObject
   {
   public:
      _netIOV()
      :iovBase( NULL ),
       iovLen( 0 )
      {

      }

      _netIOV( void *base, UINT32 len )
      :iovBase(base),
       iovLen(len)
      {

      }

      virtual ~_netIOV()
      {
         iovBase = NULL ;
         iovLen = 0 ;
      }
   public:
      const void *iovBase ;
      UINT32 iovLen ;
   } ;
   typedef class _netIOV netIOV ;

   typedef std::vector<netIOV> netIOVec ;
}

#endif

