#ifndef DPSTRANSLOCKBUCKET_HPP_
#define DPSTRANSLOCKBUCKET_HPP_

#include "dpsTransLockDef.hpp"
#include "ossLatch.hpp"
#include <map>

namespace engine
{
   class dpsTransLockUnit;
   typedef std::map< dpsTransLockId, dpsTransLockUnit * > dpsTransLockUnitList;
   class dpsLockBucket : public SDBObject
   {
   public:
      friend class dpsTransLock;
   protected:
      dpsLockBucket();
      ~dpsLockBucket();
      INT32 acquire( _pmdEDUCB *eduCB, const dpsTransLockId &lockId,
                     DPS_TRANSLOCK_TYPE lockType );
      INT32 upgrade( _pmdEDUCB *eduCB, const dpsTransLockId &lockId,
                     DPS_TRANSLOCK_TYPE lockType );
      void release( _pmdEDUCB *eduCB, const dpsTransLockId &lockId );

      INT32 test( _pmdEDUCB *eduCB, const dpsTransLockId &lockId,
                     DPS_TRANSLOCK_TYPE lockType );

      INT32 tryAcquire( _pmdEDUCB *eduCB, const dpsTransLockId &lockId,
                     DPS_TRANSLOCK_TYPE lockType );

      INT32 tryAcquireOrAppend( _pmdEDUCB *eduCB, const dpsTransLockId &lockId,
                     DPS_TRANSLOCK_TYPE lockType, BOOLEAN appendHead = FALSE );

      BOOLEAN hasWait( const dpsTransLockId &lockId );

      INT32 waitLockX( _pmdEDUCB *eduCB, const dpsTransLockId &lockId );


   private:
      INT32 appendToRun( _pmdEDUCB *eduCB, DPS_TRANSLOCK_TYPE lockType,
                        dpsTransLockUnit *pLockUnit );

      void appendToWait( _pmdEDUCB *eduCB, const dpsTransLockId &lockId,
                        dpsTransLockUnit *pLockUnit );

      void appendHeadToWait( _pmdEDUCB *eduCB, const dpsTransLockId &lockId,
                        dpsTransLockUnit *pLockUnit );

      void removeFromWait( _pmdEDUCB *eduCB,
                        dpsTransLockUnit *pLockUnit,
                        const dpsTransLockId &lockId );

      void removeFromRun( _pmdEDUCB *eduCB,
                        dpsTransLockUnit *pLockUnit );

      void wakeUp( _pmdEDUCB *eduCB );

      BOOLEAN isLockCompatible( DPS_TRANSLOCK_TYPE first,
                              DPS_TRANSLOCK_TYPE second );

      BOOLEAN checkCompatible( _pmdEDUCB *eduCB,
                              DPS_TRANSLOCK_TYPE lockType,
                              dpsTransLockUnit *pLockUnit );

      INT32 waitLock( _pmdEDUCB *eduCB );


   private:
      dpsTransLockUnitList       _lockLst;
      ossSpinXLatch              _lstMutex;
   };
}

#endif
