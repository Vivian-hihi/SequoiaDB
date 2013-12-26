/*******************************************************************************
   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = clsVSPrimary.hpp

   Descriptive Name =

   When/how to use:

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          11/28/2012  YW  Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef CLSVSPRIMARY_HPP_
#define CLSVSPRIMARY_HPP_

#include "clsVoteStatus.hpp"

namespace engine
{
   class _clsVSPrimary : public _clsVoteStatus
   {
   public:
      _clsVSPrimary( _clsGroupInfo *info,
                     _netRouteAgent *agent ) ;
      virtual ~_clsVSPrimary() ;

   public:
      virtual INT32 handleInput( const MsgHeader *header,
                                 INT32 &next ) ;

      virtual void handleTimeout( const UINT32 &millisec,
                                  INT32 &next ) ;

      virtual void active( INT32 &next ) ;
      virtual void deactive () ;

      virtual const CHAR *name() const { return "Primary" ;}


   } ;
}

#endif

