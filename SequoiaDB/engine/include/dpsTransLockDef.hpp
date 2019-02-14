/*******************************************************************************


   Copyright (C) 2011-2018 SequoiaDB Ltd.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU Affero General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Affero General Public License for more details.

   You should have received a copy of the GNU Affero General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.

   Source File Name = dpsTransLockDef.hpp

   Descriptive Name = Operating System Services Types Header

   When/how to use: this program may be used on binary and text-formatted
   versions of OSS component. This file contains declare for data types used in
   SequoiaDB.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          11/08/2018  XJH Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef DPS_TRANSLOCK_DEF_HPP__
#define DPS_TRANSLOCK_DEF_HPP__

#include "ossTypes.h"
#include "oss.hpp"
#include "dms.hpp"
#include "ossUtil.hpp"
#include "pd.hpp"     // SDB_ASSERT
#include <sstream>
#include "../bson/bson.h"

using namespace bson ;
using namespace std ;

namespace engine
{

   /*
      DEFINE LOCK TYPE
   */
   typedef UINT8 DPS_TRANSLOCK_TYPE ;

   //
   // WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING
   //
   // Do NOT modify the value of lock type defined below before
   // you verify the compatibility of the functions :
   //   isLockCompatible
   //   upgradeCheck
   //   lockCoverage
   //
   // WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING
   //
   #define   DPS_TRANSLOCK_IS  ((UINT8)0)
   #define   DPS_TRANSLOCK_IX  ((UINT8)1)
   #define   DPS_TRANSLOCK_S   ((UINT8)2)
   #define   DPS_TRANSLOCK_U   ((UINT8)3)
   #define   DPS_TRANSLOCK_X   ((UINT8)4)
   #define   DPS_TRANSLOCK_MAX ((UINT8)( DPS_TRANSLOCK_X + 1 ))

   //
   // Lock compatibilities
   //
   //                   State of Held
   // -------------------------------------------
   // State          IS    IX    S     U     X
   // Being
   // requested
   // -------------------------------------------
   // IS             Y     Y     Y     Y     N
   // IX             Y     Y     N     N     N
   // S              Y     N     Y     Y     N
   // U              Y     N     Y     N     N
   // X              N     N     N     N     N
   static const UINT8 _LockCompatibilityMatrix[DPS_TRANSLOCK_MAX]
                                              [DPS_TRANSLOCK_MAX] =
     {{ 1,  1,  1,  1,  0 },
      { 1,  1,  0,  0,  0 },
      { 1,  0,  1,  1,  0 },
      { 1,  0,  1,  0,  0 },
      { 0,  0,  0,  0,  0 }} ;
   OSS_INLINE BOOLEAN dpsIsLockCompatible
   (
      const DPS_TRANSLOCK_TYPE current,
      const DPS_TRANSLOCK_TYPE request
   )
   {
      SDB_ASSERT( ( ( current < DPS_TRANSLOCK_MAX ) &&
                    ( request < DPS_TRANSLOCK_MAX ) ),
                  "Invalid arguments" ) ;
      return ( _LockCompatibilityMatrix[request][current] ? TRUE : FALSE ) ;
   }

   //
   //  lock upgrade check
   //
   // request     current mode
   // mode      IS   IX   S    U    X
   // --------------------------------
   // IS        x    x    x    x    x     x: upgrade is not allowed or needed
   // IX        v    x    x    x    x     v: upgrade is needed
   // S         v    x    x    x    x
   // U         x    x    x    x    x
   // X         x    x    v    v    x
   //
   // Valid upgrade :
   //     IS --> IX
   //      S --> X
   //     IS --> S
   //      U --> X
   //
   // Why IS or IX can't upgrade to X ?
   //   when IUD, IX lock is applied on collection; if later drop collection is
   //   allowed( X lock on collection ), then no way to rollback.
   static const UINT8 _LockUpgradeCheckMatrix[DPS_TRANSLOCK_MAX]
                                             [DPS_TRANSLOCK_MAX]=
     {{ 0, 0, 0, 0, 0 },
      { 1, 0, 0, 0, 0 },
      { 1, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0 },
      { 0, 0, 1, 1, 0 }} ;
   OSS_INLINE INT32 dpsUpgradeCheck
   (
      const DPS_TRANSLOCK_TYPE current,
      const DPS_TRANSLOCK_TYPE request
   )
   {
      SDB_ASSERT( ( ( current < DPS_TRANSLOCK_MAX ) &&
                    ( request < DPS_TRANSLOCK_MAX ) ),
                  "Invalid arguments" ) ;
      if ( _LockUpgradeCheckMatrix[request][current] )
      {
         return SDB_OK ;
      }
      return SDB_DPS_INVALID_LOCK_UPGRADE_REQUEST ;
   }

   // Lock coverage
   //                   State of Held
   // -------------------------------------------
   // State          IS    IX    S     U     X
   // Being
   // requested
   // -------------------------------------------
   // IS             Y     Y     Y     Y     Y
   // IX             N     Y     N     N     Y
   // S              N     N     Y     Y     Y
   // U              N     N     N     Y     Y
   // X              N     N     N     N     Y
   static const UINT8 _LockCoverageMatrix[DPS_TRANSLOCK_MAX]
                                         [DPS_TRANSLOCK_MAX]=
     {{ 1, 1, 1, 1, 1 },
      { 0, 1, 0, 0, 1 },
      { 0, 0, 1, 1, 1 },
      { 0, 0, 0, 1, 1 },
      { 0, 0, 0, 0, 1 }} ;
   OSS_INLINE BOOLEAN dpsLockCoverage
   (
      const DPS_TRANSLOCK_TYPE current,
      const DPS_TRANSLOCK_TYPE request
   )
   {
      SDB_ASSERT( ( ( current < DPS_TRANSLOCK_MAX ) &&
                    ( request < DPS_TRANSLOCK_MAX ) ),
                  "Invalid arguments" ) ;
      return ( _LockCoverageMatrix[request][current] ? TRUE : FALSE ) ;
   }


   // get intent lock mode
   static DPS_TRANSLOCK_TYPE _intentLockMatrix[DPS_TRANSLOCK_MAX] =
   {
      DPS_TRANSLOCK_IS,  // <-- DPS_TRANSLOCK_IS
      DPS_TRANSLOCK_IS,  // <-- DPS_TRANSLOCK_IX
      DPS_TRANSLOCK_IS,  // <-- DPS_TRANSLOCK_S
      DPS_TRANSLOCK_IS,  // <-- DPS_TRANSLOCK_U
      DPS_TRANSLOCK_IX   // <-- DPS_TRANSLOCK_X
   };
   OSS_INLINE DPS_TRANSLOCK_TYPE dpsIntentLockMode
   (
      const DPS_TRANSLOCK_TYPE request
   )
   {
      SDB_ASSERT( ( request < DPS_TRANSLOCK_MAX ), "Invalid argument" ) ;
      return ( _intentLockMatrix[request] ) ;
   }


   // Convert lock mode to string
   static const CHAR* _lockModeString[DPS_TRANSLOCK_MAX] =
   { "IS",
     "IX",
     "S",
     "U",
     "X"
   } ;
   OSS_INLINE const CHAR* lockModeToString ( const DPS_TRANSLOCK_TYPE lockMode )
   {
      SDB_ASSERT( ( lockMode < DPS_TRANSLOCK_MAX ), "Invalid argument" ) ;
      return _lockModeString[lockMode] ;
   }

   /*
      Name define
   */
   #define DPS_LOCKID_CSID          "CSID"
   #define DPS_LOCKID_CLID          "CLID"
   #define DPS_LOCKID_EXTENTID      "ExtentID"
   #define DPS_LOCKID_OFFSET        "Offset"

   /*
      _dpsTransLockId define
   */
   class _dpsTransLockId : public SDBObject
   {
      public:
         _dpsTransLockId( UINT32 logicCSID,
                          UINT16 collectionID,
                          const dmsRecordID *recordID )
         {
            _logicCSID        = logicCSID ;
            _collectionID     = collectionID ;
            _recordExtentID   = DMS_INVALID_EXTENT ;
            _recordOffset     = DMS_INVALID_OFFSET ;

            if ( recordID )
            {
               _recordExtentID= recordID->_extent ;
               _recordOffset  = recordID->_offset ;
            }
         }

         _dpsTransLockId( const _dpsTransLockId &id )
         {
            operator=( id ) ;
         }

         _dpsTransLockId()
         {
            reset() ;
         }

         ~_dpsTransLockId()
         {
         }

      public:

         OSS_INLINE BOOLEAN   operator<  ( const _dpsTransLockId &rhs ) const ;
         OSS_INLINE BOOLEAN   operator== ( const _dpsTransLockId &rhs ) const ;
         OSS_INLINE _dpsTransLockId& operator= ( const _dpsTransLockId &rhs ) ;

         OSS_INLINE void            reset() ;

         OSS_INLINE BOOLEAN         isValid() const ;
         OSS_INLINE UINT32          lockIdHash() const ;
         OSS_INLINE BOOLEAN         isLeafLevel() const ;
         OSS_INLINE BOOLEAN         isRootLevel() const ;
         OSS_INLINE _dpsTransLockId upOneLevel() const ;

         /*
            Access functions
         */
         UINT32         csID() const { return _logicCSID ; }
         UINT16         clID() const { return _collectionID ; }
         dmsExtentID    extentID() const { return _recordExtentID ; }
         dmsOffset      offset() const { return _recordOffset ; }

         /*
            Format functions
         */
         OSS_INLINE string       toString() const ;
         OSS_INLINE BSONObj      toBson() const ;
         OSS_INLINE void         toBson( BSONObjBuilder &builder ) const ;

      private:
         UINT32               _logicCSID ;
         UINT16               _collectionID ;
         dmsExtentID          _recordExtentID ;
         dmsOffset            _recordOffset ;
   } ;
   typedef _dpsTransLockId dpsTransLockId ;

   /*
      INLINE FUCTION
   */
   OSS_INLINE BOOLEAN _dpsTransLockId::operator< ( const _dpsTransLockId &rhs ) const
   {
      if ( _logicCSID < rhs._logicCSID )
      {
         return TRUE ;
      }
      else if ( _logicCSID > rhs._logicCSID )
      {
         return FALSE ;
      }
      if ( _collectionID < rhs._collectionID )
      {
         return TRUE ;
      }
      else if ( _collectionID > rhs._collectionID )
      {
         return FALSE ;
      }
      if ( _recordExtentID < rhs._recordExtentID )
      {
         return TRUE ;
      }
      else if ( _recordExtentID > rhs._recordExtentID )
      {
         return FALSE ;
      }
      if ( _recordOffset < rhs._recordOffset )
      {
         return TRUE ;
      }
      return FALSE ;
   }

   OSS_INLINE BOOLEAN _dpsTransLockId::operator== ( const _dpsTransLockId &rhs ) const
   {
      if ( _logicCSID == rhs._logicCSID &&
           _collectionID == rhs._collectionID &&
           _recordExtentID == rhs._recordExtentID &&
           _recordOffset == rhs._recordOffset )
      {
         return TRUE ;
      }
      return FALSE ;
   }

   OSS_INLINE _dpsTransLockId& _dpsTransLockId::operator= ( const _dpsTransLockId &rhs )
   {
      _logicCSID        = rhs._logicCSID ;
      _collectionID     = rhs._collectionID ;
      _recordExtentID   = rhs._recordExtentID ;
      _recordOffset     = rhs._recordOffset ;

      return *this ;
   }

   OSS_INLINE void _dpsTransLockId::reset()
   {
      _logicCSID        = ~0 ;
      _collectionID     = DMS_INVALID_MBID ;
      _recordExtentID   = DMS_INVALID_EXTENT ;
      _recordOffset     = DMS_INVALID_OFFSET ;
   }

   OSS_INLINE _dpsTransLockId _dpsTransLockId::upOneLevel() const
   {
      _dpsTransLockId id( *this ) ;

      if ( id.isValid() )
      {
         if ( DMS_INVALID_MBID != id._collectionID )
         {
            if ( DMS_INVALID_OFFSET != id._recordOffset )
            {
               // record lock => collection lock
               id._recordOffset   = DMS_INVALID_OFFSET ;
               id._recordExtentID = DMS_INVALID_EXTENT ;
            }
            else
            {
               // collection lock => collection space lock
               id._collectionID   = DMS_INVALID_MBID ;
               id._recordExtentID = DMS_INVALID_EXTENT ;
            }
         }
         else
         {
            // collection space lock
            id._recordOffset   = DMS_INVALID_OFFSET ;
            id._recordExtentID = DMS_INVALID_EXTENT ;
         }
      }

      return id ;
   }

   OSS_INLINE BOOLEAN _dpsTransLockId::isValid() const
   {
      if ( (UINT32)~0 == _logicCSID )
      {
         return FALSE ;
      }
      return TRUE ;
   }

   OSS_INLINE UINT32 _dpsTransLockId::lockIdHash() const
   {
      UINT32 v = 0 ;

      v  = ossHash((CHAR*)&( _logicCSID ),     sizeof( _logicCSID ), 5 ) ;
      v ^= ossHash((CHAR*)&( _recordExtentID ),sizeof( _recordExtentID ), 5 );
      v ^= ossHash((CHAR*)&( _recordOffset ),  sizeof( _recordOffset ), 5 ) ;
      v ^= ossHash((CHAR*)&( _collectionID ),  sizeof( _collectionID ), 5 ) ;

      return v ;
   }

   OSS_INLINE BOOLEAN _dpsTransLockId::isLeafLevel() const
   {
      if (    isValid()
           && ( DMS_INVALID_MBID   != _collectionID )
           && ( DMS_INVALID_EXTENT != _recordExtentID )
           && ( DMS_INVALID_OFFSET != _recordOffset ) )
      {
         return TRUE ;
      }
      return FALSE ;
   }

   OSS_INLINE BOOLEAN _dpsTransLockId::isRootLevel() const
   {
      if (    isValid()
           && ( DMS_INVALID_MBID   == _collectionID )
           && ( DMS_INVALID_EXTENT == _recordExtentID )
           && ( DMS_INVALID_OFFSET == _recordOffset ) )
      {
         return TRUE ;
      }
      return FALSE ;
   }

   OSS_INLINE string _dpsTransLockId::toString() const
   {
      stringstream ss ;
      ss << DPS_LOCKID_CSID":" << csID()
         << ", "DPS_LOCKID_CLID":" << clID()
         << ", "DPS_LOCKID_EXTENTID":" << extentID()
         << ", "DPS_LOCKID_OFFSET":" << offset() ;
      return ss.str() ;
   }

   OSS_INLINE BSONObj _dpsTransLockId::toBson() const
   {
      BSONObjBuilder builder( 128 ) ;
      toBson( builder ) ;
      return builder.obj () ;
   }

   OSS_INLINE void _dpsTransLockId::toBson( BSONObjBuilder &builder ) const
   {
      builder.append( DPS_LOCKID_CSID, (INT32)csID() ) ;
      builder.append( DPS_LOCKID_CLID, (INT32)clID() ) ;
      builder.append( DPS_LOCKID_EXTENTID, (INT32)extentID() ) ;
      builder.append( DPS_LOCKID_OFFSET, (INT32)offset() ) ;
   }

   /*
      _dpsTransResult define
   */
   struct _dpsTransRetInfo
   {
      dpsTransLockId       _lockID ;
      DPS_TRANSLOCK_TYPE   _lockType ;
      EDUID                _eduID ;
      BOOLEAN              _useOldVersion ;  // if we use old version record 
      BOOLEAN              _newAcquire ;     // if the lock was newly granted
      BOOLEAN              _skipNewRecord ;  // if should skip the new record

      _dpsTransRetInfo()
      {
         _lockType      = DPS_TRANSLOCK_IS ;
         _eduID         = 0 ;
         _useOldVersion = FALSE ;
         _newAcquire    =FALSE ;
         _skipNewRecord = FALSE ;
      }
   } ;
   typedef _dpsTransRetInfo dpsTransRetInfo ;

}

#endif // DPS_TRANSLOCK_DEF_HPP__

