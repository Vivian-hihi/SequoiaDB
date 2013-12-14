#ifndef CATEVENTPROCESSOR_HPP__
#define CATEVENTPROCESSOR_HPP__

#include "core.hpp"
#include "oss.hpp"
#include "pd.hpp"
#include "pmdEDU.hpp"

namespace engine
{
   class catEventProcessor : public SDBObject
   {
   public:
      virtual ~catEventProcessor()
      {
      }
      virtual INT32 processEvent( pmdEDUEvent &event )
      {
         INT32 rc = SDB_MAX_ERROR;
         switch ( event._eventType )
         {
         case PMD_EDU_EVENT_ACTIVE:
               //TODO:get status from replica
               rc = active();
               break;

         case  PMD_EDU_EVENT_DEACTIVE:
               rc = deactive();
               break;

         case PMD_EDU_EVENT_MSG:
               rc = processMsg(event._Data);
               break;

         default:
               rc = SDB_INVALIDARG;
               PD_LOG( PDEVENT,
                     "received unknow event(EventType = %d)",
                     event._eventType );
         }
         return rc;

      }

      virtual INT32 init() = 0;
      virtual INT32 active() = 0;
      virtual INT32 deactive() = 0;
   private:
      virtual INT32 processMsg(void *pMsg) = 0;
   };
}

#endif
