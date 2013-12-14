/*******************************************************************************
   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = clsCatalogCaller.hpp

   Descriptive Name =

   When/how to use: this program may be used on binary and text-formatted
   versions of Replication component. This file contains structure for
   replication control block.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          09/14/2012  TW  Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef CLSCATALOGCALLER_HPP_
#define CLSCATALOGCALLER_HPP_

#include "core.hpp"
#include "oss.hpp"
#include "msg.hpp"

#include <map>

using namespace std ;
namespace engine
{
   class _netRouteAgent ;
   class _clsMgr ;

   struct _clsCataCallerMeta
   {
      MsgHeader *header ;
      UINT32 bufLen ;
      INT32  timeout ;

      _clsCataCallerMeta()
      :header(NULL),
       bufLen(0),
       timeout(-1)
      {

      }

      ~_clsCataCallerMeta()
      {
         if ( NULL != header )
         {
            SDB_OSS_FREE( header) ;
            header = NULL ;
         }
         bufLen = 0 ;
         timeout = -1 ;
      }
   } ;
   typedef std::map<UINT32, _clsCataCallerMeta> callerMeta ;

   class _clsCatalogCaller : public SDBObject
   {
   public:
      _clsCatalogCaller() ;
      ~_clsCatalogCaller() ;

   public:
      INT32 call( MsgHeader *header ) ;

      void remove( _MsgInternalReplyHeader *header ) ;

      void handleTimeout( const UINT32 &millisec ) ;

   private:
      callerMeta _meta ;
      _clsMgr *_cMgr ;
   } ;

   typedef class _clsCatalogCaller clsCatalogCaller ;
}

#endif

