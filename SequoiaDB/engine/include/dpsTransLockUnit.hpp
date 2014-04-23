#ifndef DPSTRANSLOCKUNIT_HPP_
#define DPSTRANSLOCKUNIT_HPP_

#include "pmdEDU.hpp"
#include <map>

namespace engine
{
   typedef std::map< UINT32, DPS_TRANSLOCK_TYPE >    dpsTransLockRunList;
   class dpsTransLockUnit : public SDBObject
   {
   public:
      dpsTransLockUnit()
      {
         _pWaitCB = NULL ;
      }
      ~dpsTransLockUnit(){};
   public:
      _pmdEDUCB   *_pWaitCB;
      dpsTransLockRunList  _runList;
   };
}

#endif
