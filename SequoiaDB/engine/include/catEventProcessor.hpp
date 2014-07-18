/*******************************************************************************


   Copyright (C) 2011-2014 SequoiaDB Ltd.

   This program is free software: you can redistribute it and/or modify
   it under the term of the GNU Affero General Public License, version 3,
   as published by the Free Software Foundation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warrenty of
   MARCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU Affero General Public License for more details.

   You should have received a copy of the GNU Affero General Public License
   along with this program. If not, see <http://www.gnu.org/license/>.

   Source File Name = catEventProcessor.hpp

   Descriptive Name = Process MoDel Header

   When/how to use: this program may be used on binary and text-formatted
   versions of PMD component. This file contains structure kernel control block,
   which is the most critical data structure in the engine process.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          09/14/2012  TW  Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef CATEVENTPROCESSOR_HPP__
#define CATEVENTPROCESSOR_HPP__

#include "core.hpp"
#include "oss.hpp"
#include "pd.hpp"
#include "pmdEDU.hpp"
#include "msg.h"
#include "netDef.hpp"

namespace engine
{
   /*
      catEventProcessor define
   */
   class catEventProcessor : public SDBObject
   {
   public:
      virtual ~catEventProcessor()
      {
      }
      virtual INT32 processEvent( pmdEDUEvent &event )
      {
         INT32 rc = SDB_MAX_ERROR ;
         switch ( event._eventType )
         {
         case PMD_EDU_EVENT_ACTIVE:
               rc = active();
               break;

         case  PMD_EDU_EVENT_DEACTIVE:
               rc = deactive();
               break;

         case PMD_EDU_EVENT_MSG:
               rc = processMsg( (NET_HANDLE)event._userData,
                                (MsgHeader*)event._Data ) ;
               break;

         default:
               rc = SDB_INVALIDARG;
               PD_LOG( PDEVENT, "Received unknow event(EventType = %d)",
                       event._eventType );
         }
         return rc ;
      }

      virtual INT32 init() = 0 ;
      virtual INT32 active() = 0 ;
      virtual INT32 deactive() = 0 ;

   private:
      virtual INT32 processMsg( const NET_HANDLE &handle,
                                MsgHeader *pMsg ) = 0 ;

   } ;

}

#endif // CATEVENTPROCESSOR_HPP__
