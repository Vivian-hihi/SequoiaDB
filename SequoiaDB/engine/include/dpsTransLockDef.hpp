#ifndef DPSTRANSLOCKDEF_HPP_
#define DPSTRANSLOCKDEF_HPP_

#include "ossTypes.h"
#include <map>
#include <string>
#include "dms.hpp"
#include "ossAtomic.hpp"
#include "msg.h"

namespace engine
{
#define DPS_TRANS_LOCK_WAIT_TIME          864000000ll    //
   class _pmdEDUCB;

   enum DPS_TRANSLOCK_TYPE
   {
      // note: don't modify the enum lightly,
      // check the fun( dpsTransLock::upgradeCheck
      // and dpsLockBucket::isLockCompatible ) before modify
      DPS_TRANSLOCK_IS = 0,
      DPS_TRANSLOCK_IX,
      DPS_TRANSLOCK_S,
      DPS_TRANSLOCK_X
   };

   /*enum DPS_TRANSLOCK_STATUS
   {
      DPS_TRANSLOCK_GOT = 0,
      DPS_TRANSLOCK_WAIT
   };*/

   typedef std::map<UINT32, MsgRouteID>      DpsTransNodeMap;

   class dpsTransLockId : public SDBObject
   {
   public:
      dpsTransLockId( UINT32 logicCSID,
                     UINT16 collectionID,
                     const _dmsRecordID *recordID );
      dpsTransLockId();
      ~dpsTransLockId();

      BOOLEAN operator<( const dpsTransLockId &rhs ) const;

      std::string toString() const;
   public:
      UINT32               _logicCSID;
      dmsExtentID          _recordExtentID;
      dmsOffset            _recordOffset;
      UINT16               _collectionID;
   };

   class dpsTransCBLockInfo : public SDBObject
   {
   public:
      dpsTransCBLockInfo( DPS_TRANSLOCK_TYPE lockType );
      ~dpsTransCBLockInfo();
      INT64 incRef();
      INT64 decRef();
      BOOLEAN isLockMatch( DPS_TRANSLOCK_TYPE type );
      DPS_TRANSLOCK_TYPE getType();
      void setType( DPS_TRANSLOCK_TYPE lockType );
      _pmdEDUCB *getNextWaitCB();
      void setNextWaitCB( _pmdEDUCB *pWaitCB );
   private:
      _pmdEDUCB                  *_pNextWaitCB ;
      DPS_TRANSLOCK_TYPE         _lockType ;
      ossAtomicSigned64          *_pRef;
   };

   typedef std::map< dpsTransLockId, dpsTransCBLockInfo * >    DpsTransCBLockList;

}

#endif
