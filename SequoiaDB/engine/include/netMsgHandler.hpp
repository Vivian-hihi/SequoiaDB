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

#ifndef NETMSGHANDLER_HPP_
#define NETMSGHANDLER_HPP_

#include "core.hpp"
#include "oss.hpp"
#include "netDef.hpp"

namespace engine
{
   /*
      _netMsgHandler define
   */
   class _netMsgHandler : public SDBObject
   {
      public:
        _netMsgHandler(){}
        virtual ~_netMsgHandler(){}
      public:
        virtual INT32 handleMsg( const NET_HANDLE &handle,
                                 const _MsgHeader *header,
                                 const CHAR *msg ) = 0 ;

        virtual void handleClose( const NET_HANDLE &handle, _MsgRouteID id )
        {
        }
   } ;

   typedef _netMsgHandler INetMsgHandler ;

}

#endif // NETMSGHANDLER_HPP_

