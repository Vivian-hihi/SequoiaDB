#include "dpsTransLockDef.hpp"
#include "dms.hpp"
#include "dmsStorageUnit.hpp"

namespace engine
{
   dpsTransLockId::dpsTransLockId( UINT32 logicCSID,
                                 UINT16 collectionID,
                                 const _dmsRecordID *recordID )
   {
      _logicCSID = logicCSID ;
      _collectionID = collectionID ;
      if ( recordID )
      {
         _recordExtentID = recordID->_extent ;
         _recordOffset = recordID->_offset ;
      }
      else
      {
         _recordExtentID = DMS_INVALID_EXTENT ;
         _recordOffset = DMS_INVALID_OFFSET ;
      }
   }

   dpsTransLockId::dpsTransLockId()
   {
      _logicCSID = ~0 ;
      _collectionID = DMS_INVALID_MBID ;
      _recordExtentID = DMS_INVALID_EXTENT ;
      _recordOffset = DMS_INVALID_OFFSET ;
   }

   dpsTransLockId::~dpsTransLockId()
   {
   }

   BOOLEAN dpsTransLockId::operator<( const dpsTransLockId &rhs ) const
   {
      if ( _logicCSID < rhs._logicCSID )
      {
         return TRUE;
      }
      else if ( _logicCSID > rhs._logicCSID )
      {
         return FALSE;
      }
      if ( _collectionID < rhs._collectionID )
      {
         return TRUE;
      }
      else if ( _collectionID > rhs._collectionID )
      {
         return FALSE;
      }
      if ( _recordExtentID < rhs._recordExtentID )
      {
         return TRUE;
      }
      return FALSE;
   }

   std::string dpsTransLockId::toString() const
   {
      CHAR szBuffer[100] = {0};
      ossSnprintf( szBuffer, 50,
                  "CSID:%u, CLID:%u, recordID:%d, recordOffset:%d",
                  _logicCSID, _collectionID, _recordExtentID, _recordOffset );
      std::string strInfo( szBuffer );
      return strInfo;
   }

   dpsTransCBLockInfo::dpsTransCBLockInfo( DPS_TRANSLOCK_TYPE lockType )
   : _lockType( lockType )
   {
      _pNextWaitCB = NULL ;
      _pRef = SDB_OSS_NEW ossAtomicSigned64(0);
   }
   dpsTransCBLockInfo::~dpsTransCBLockInfo()
   {
      if ( _pRef )
      {
         SDB_OSS_DEL _pRef ;
      }
   }
   INT64 dpsTransCBLockInfo::incRef()
   {
      return _pRef->inc();
   }

   INT64 dpsTransCBLockInfo::decRef()
   {
      return _pRef->dec();
   }

   BOOLEAN dpsTransCBLockInfo::isLockMatch( DPS_TRANSLOCK_TYPE type )
   {
      if ( _lockType == type )
      {
         return TRUE ;
      }
      if ( DPS_TRANSLOCK_IS == type )
      {
         return TRUE ;
      }
      if ( DPS_TRANSLOCK_X == _lockType )
      {
         return TRUE ;
      }
      return FALSE ;
   }

   DPS_TRANSLOCK_TYPE dpsTransCBLockInfo::getType()
   {
      return _lockType ;
   }

   void dpsTransCBLockInfo::setType( DPS_TRANSLOCK_TYPE lockType )
   {
      _lockType = lockType ;
   }

   _pmdEDUCB *dpsTransCBLockInfo::getNextWaitCB()
   {
      return _pNextWaitCB ;
   }
   void dpsTransCBLockInfo::setNextWaitCB( _pmdEDUCB *pWaitCB )
   {
      _pNextWaitCB = pWaitCB ;
   }
}
