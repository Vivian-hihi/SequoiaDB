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

   Source File Name = monClass.hpp

   Descriptive Name = Monitor Service Header

   When/how to use: this program may be used on binary and text-formatted
   versions of OSS component. This file contains functions for OSS operations.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          05/24/2019  CW  Initial Draft

   Last Changed =

*******************************************************************************/
#ifndef MONCLASS_HPP_
#define MONCLASS_HPP_

#include "monLatch.hpp"
#include "dpsTransLockDef.hpp"
#include "utilPooledObject.hpp"
#include "ossTypes.h"
#include "monLatch.hpp"
#include "ossAtomic.hpp"
#include "ossUtil.hpp"
#include "msg.h"
#include "pd.hpp"
#include "../bson/bson.hpp"
#include <boost/intrusive/list.hpp>
#include <iterator>

using namespace bson ;

namespace engine
{

class _monAppCB ;

#define MONQUERY_SET_NAME(edu, n)\
  if (edu->getMonQueryCB()) \
  { \
     edu->getMonQueryCB()->_name.assign(n); \
  }\

#define MONQUERY_SET_QUERY_TEXT(edu, n)\
  if (edu->getMonQueryCB() && edu->getMonQueryCB()->_dataLvl == MON_DATA_LVL_DETAIL )\
  {\
     edu->getMonQueryCB()->_queryText.assign(n); \
  }\

typedef enum
{
   MON_CLASS_QUERY = 0,  // _MonClassQuery
   MON_CLASS_LATCH,      // _MonClassLatch
   MON_CLASS_LOCK,       // _MonClassLock
   MON_CLASS_MAX
} MonitorClassType ;

// Status for _MonClass
#define MON_CLASS_STATUS_NORMAL   0x00
#define MON_CLASS_STATUS_PEND_DEL 0x01
#define MON_CLASS_STATUS_PEND_ARC 0x02

// Monitor data capture level
typedef enum
{
   MON_DATA_LVL_NONE = 0,
   MON_DATA_LVL_BASIC,
   MON_DATA_LVL_DETAIL
} MonDataLvl ;

struct _MonClassBaseData
{
} ;

// Data structure used when constructing a MonClassLatch
struct _MonClassLatchData : public _MonClassBaseData
{
   UINT32 waiterTID ;
   UINT32 ownerTID ;
   MON_LATCH_IDENTIFIER latchID ;
   void* latchAddr ;
   OSS_LATCH_MODE latchMode ;
   UINT32 lastSOwner ;
   UINT32 numOwner ;

   _MonClassLatchData()
      : waiterTID( 0 ), ownerTID( 0 ), latchID( MON_LATCH_ID_MAX ),
        latchAddr( NULL ), latchMode( SHARED ), lastSOwner( 0 ),
        numOwner( 0 )
   {}
} ;

typedef _MonClassLatchData MonClassLatchData ;

typedef boost::intrusive::list_base_hook< > BaseHook ;

/**
 * _MonClass - Parent class for all monitor class
 *
 * It is up to the user to define the synchronization strategy for each _MonClass.
 * For example, if a metric can be concurrently updated by more than one thread,
 * then the user might want to define that metric as atomic
 */
class _MonClass : public BaseHook, public utilPooledObject
{
   ossTimestamp _createTS ;      /**! create timestamp for this object */
   ossTimestamp    _endTS ;      /**! end timestamp for this object */
   ossTick  _createTSTick ;      /**! create tick for this object */
   UINT16         _status ;      /**! object status */

protected:
   MonitorClassType _type ;      /**! object type */

public:
   //TODO retrieve this directly from container
   MonDataLvl    _dataLvl ;      /**! Monitor data collection level */

   _MonClass() ;
   virtual ~_MonClass() ;

   /**
    * getType - return the type of the object
    */
   MonitorClassType getType () const { return _type; }

   /**
    * setPendingDelete - mark an object to be pending delete
    */
   void setPendingDelete() { _status |= MON_CLASS_STATUS_PEND_DEL ; }
   BOOLEAN isPendingDelete() const
   {
      return ( _status & MON_CLASS_STATUS_PEND_DEL ) ? TRUE : FALSE ;
   }

   /**
    * setPendingArchive - mark an object to be pending archive
    */
   void setPendingArchive() { _status |= MON_CLASS_STATUS_PEND_ARC ; }
   BOOLEAN isPendingArchive() const
   {
      return ( _status & MON_CLASS_STATUS_PEND_ARC ) ? TRUE : FALSE ;
   }

   /**
    * getStatus() - get the current status
    */
   UINT16 getStatus() const { return _status ; }

   /**
    * Get the create timestamp
    */
   ossTimestamp getCreateTS() const { return _createTS ; }

   /**
    * Get the end timestamp
    */
   ossTimestamp &getEndTS() { return _endTS ; }

   /**
    * Get the create timestamp tick
    */
   ossTick getCreateTSTick() const { return _createTSTick ; }

   /**
    * Format object into a BSON
    */
   virtual void dump( BSONObj &obj ) = 0 ;

   /**
    * Reset metrics
    */
   virtual void reset() = 0 ;
} ;

typedef _MonClass MonClass ;

/**
 * Template to ensure all subclasses implement the getType function
 */
template<class T>
class MonClassTemplate : public _MonClass
{
protected:
   MonClassTemplate() {}

public:
   ~MonClassTemplate()
   {
      T::getType();
   }
} ;

/**
 * archive query information based on response time
 */
BOOLEAN archiveQuery ( _MonClass *obj ) ;

/**
 * archive latch information based on wait time
 */
BOOLEAN archiveLatch ( _MonClass *obj ) ;

BOOLEAN archiveLock ( _MonClass *obj ) ;

/**
 * Default function pointer to indicate no archive
 */
BOOLEAN noArchive ( _MonClass *obj ) ;

typedef BOOLEAN (*archiveFunc)(_MonClass *) ;

typedef enum
{
   MON_CLASS_ACTIVE_LIST,
   MON_CLASS_ARCHIVED_LIST
} MonClassListType ;

/*
 * Structure to help compute the delta from monAppCB
 */
struct _MonClassQueryTmpData
{
   UINT32            _dataRead ;
   UINT32           _indexRead ;
   UINT32           _dataWrite ;
   UINT32          _indexWrite ;

   _MonClassQueryTmpData()
      : _dataRead(0), _indexRead(0), _dataWrite(0), _indexWrite(0)
   {}

   _MonClassQueryTmpData& operator=(const _monAppCB& cb) ;

   void diff(_monAppCB &cb) ;
} ;
typedef _MonClassQueryTmpData MonClassQueryTmpData ;

/**
 * Capture metrics about a query
 */
class _MonClassQuery : public MonClassTemplate<_MonClassQuery>
{
public:
   UINT32                 _tid ;  /**! TID of the EDU */
   std::string           _name ;  /**! The target object name of this query */
   SINT64        _accessPlanID ;  /**! Access plan ID used by the query */
   UINT32              _opCode ;  /**! Message opCode */
   UINT32           _sessionID ;  /**! EDU Session ID */
   ossTickDelta  _responseTime ;  /**! Response time of the query */
   ossTickDelta _latchWaitTime ;  /**! Time spent on latch wait */
   ossTickDelta  _lockWaitTime ;  /**! Time spent on lock wait */
   UINT32            _dataRead ;  /**! Total data read (record)*/
   UINT32           _indexRead ;  /**! Total index read (record)*/
   UINT32             _lobRead ;  /**! Total LOB read */
   UINT32           _dataWrite ;  /**! Total data write (record) */
   UINT32          _indexWrite ;  /**! Total index write (record) */
   UINT32            _lobWrite ;  /**! Total LOB write */
   UINT32        _rowsReturned ;  /**! Total number of rows returned */
   UINT32          _numMsgSent ;  /**! Total number of messages sent to remote nodes */
   std::set<UINT32>      nodes ;  /**! Node ID where messages were sent to */
   MsgRouteID      _relatedNID ;  /**! coordinator node node ID */
   UINT32          _relatedTID ;  /**! coordinator node edu TID */
   BOOLEAN    _anchorToContext ;  /**! Whether this object is anchored to a context */
   std::string      _queryText ;  /**! Full query text */
   ossTickDelta _remoteNodesResponseTime ; /**! Time spent waiting on remote nodes */
   ossTickDelta _msgSentTime ;    /**! Time spent sending messages to remote nodes */

   _MonClassQuery ()
     :  _accessPlanID( -1 ),
        _opCode( 0 ),
        _sessionID( 0 ),
        _dataRead( 0 ),
        _indexRead( 0 ),
        _lobRead( 0 ),
        _dataWrite( 0 ),
        _indexWrite( 0 ),
        _lobWrite( 0 ),
        _rowsReturned( 0 ),
        _numMsgSent( 0 ),
        _relatedTID( 0 ),
        _anchorToContext( FALSE )
   {
      _type = MON_CLASS_QUERY ;
      _relatedNID.value = 0 ;
   }
   static MonitorClassType getType () { return MON_CLASS_QUERY ; }

   //TODO: to be implemented
   virtual void dump( BSONObj &obj ) {}

   //TODO: to be implemented
   virtual void reset() {}

   void startLatchTimer() { _latchWaitTimer.sample() ; }

   void stopLatchTimer()
   {
      ossTick tick ;
      tick.sample() ;
      _latchWaitTime += (tick - _latchWaitTimer) ;
   }

   void incMetrics( MonClassQueryTmpData &tmpData )
   {
      _dataRead += tmpData._dataRead ;
      _dataWrite += tmpData._dataWrite ;
      _indexRead += tmpData._indexRead ;
      _indexWrite += tmpData._indexWrite ;
   }

private:
   ossTick _latchWaitTimer ;
} ;

typedef _MonClassQuery MonClassQuery ;

class _MonClassLatch : public MonClassTemplate<_MonClassLatch>
{
public:
   _MonClassLatch ()
   {
      _type = MON_CLASS_LATCH ;
   }

   _MonClassLatch (_MonClassBaseData *data)
   {
      if ( data )
      {
         _MonClassLatchData *latchData = (_MonClassLatchData *) data ;
         _xOwnerTID = latchData->ownerTID ;
         _waiterTID = latchData->waiterTID ;
         _latchID = latchData->latchID ;
         _latchAddr = latchData->latchAddr ;
         _latchMode = latchData->latchMode ;
         _numOwner = latchData->numOwner ;
         _lastSOwner = latchData->lastSOwner ;
      }
      _type = MON_CLASS_LATCH ;
   }
   static MonitorClassType getType () { return MON_CLASS_LATCH ; }

   virtual void dump( BSONObj &obj ) {}

   virtual void reset() {}

   ossTickDelta _waitTime ;
   UINT32 _xOwnerTID ;
   UINT32 _waiterTID ;
   MON_LATCH_IDENTIFIER _latchID ;
   void *_latchAddr ;
   OSS_LATCH_MODE _latchMode ;
   UINT32 _numOwner ;
   UINT32 _lastSOwner ;
} ;

typedef _MonClassLatch MonClassLatch ;

class _MonClassLock : public MonClassTemplate<_MonClassLock>
{
public:
   _MonClassLock ()
      : _xOwnerTID( 0 ), _waiterTID( 0 ), _numOwner( 0 ),
        _lockMode( DPS_TRANSLOCK_MAX )
   {
      _type = MON_CLASS_LOCK ;
   }

   static MonitorClassType getType () { return MON_CLASS_LOCK ; }

   virtual void dump( BSONObj &obj ) {}

   virtual void reset() {}

   ossTickDelta _waitTime ;
   UINT32 _xOwnerTID ;
   UINT32 _waiterTID ;
   UINT32 _numOwner ;
   dpsTransLockId _lockID ;
   DPS_TRANSLOCK_TYPE _lockMode ;
} ;

typedef _MonClassLock MonClassLock ;


/**
 * Forward iterator for class list
 */
template <typename T>
class listFwdIterator : public std::iterator<std::forward_iterator_tag,
                                               T,
                                               std::ptrdiff_t,
                                               T*,
                                               T& >
{
   typename T::iterator itr;
   typedef typename T::reference reference ;
   typedef typename T::pointer pointer ;

public:
   explicit listFwdIterator(typename T::iterator it)
   {
      itr = it ;
   }

   listFwdIterator() {}

   listFwdIterator& operator++ () // Pre-increment
   {
      itr++ ;
      return *this;
   }

   listFwdIterator operator++ (int) // Post-increment
   {
      listFwdIterator tmp(*this);
      itr++ ;
      return tmp;
   }

   template<class OtherType>
   bool operator == (const listFwdIterator<OtherType>& rhs) const
   {
      return itr == rhs.itr;
   }

   template<class OtherType>
   bool operator != (const listFwdIterator<OtherType>& rhs) const
   {
      return itr != rhs.itr;
   }

   reference operator* () const
   {
      return *itr;
   }

   pointer operator-> () const
   {
      return &(*itr);
   }

   operator listFwdIterator<const T>() const
   {
      return listFwdIterator<const T>(itr);
   }
};


/**
 * _MonClassContainer defines
 *
 * It is a container for a _MonClass type. Contains an active list and archived list.
 * See below for latch protocol for these two lists.
 */
class _MonClassContainer : public utilPooledObject
{
friend class _MonitorManager ;

   typedef boost::intrusive::list< _MonClass, boost::intrusive::base_hook<BaseHook> > MonClassList ;

public:
   typedef listFwdIterator<MonClassList> iterator ;
   typedef listFwdIterator<const MonClassList> const_iterator ;

private:
   _MonClassContainer (MonitorClassType type) ;

   // disable assignment/copy
   _MonClassContainer& operator= (const _MonClassContainer& other) ;
   _MonClassContainer( const _MonClassContainer& other ) ;


   // a list of all the active objects
   MonClassList _activeList ;

   // latch protecting the activeList
   // Protocol is as follows:
   // - Scanner gets the headLatch and latch in S. It releases the headLatch after making a copy of the list head.
   // - Delete is async, and does not need any latch. It simply marks the object as pending delete.
   // - Add gets the headLatch in X, then add a node to the head of the list.
   // - Real delete will be done by a background agent. It gets the headLatch in S and latch in X.
   //   It releases the headLatch after processing the list head.
   // - Latches should be acquired in this order: listLatch->archiveLatch->headLatch
   ossSpinSLatch _activeListLatch ;
   ossSpinSLatch _activeListHeadLatch ;

   // a list of all the archived objects
   MonClassList _archivedList ;

   ossSpinSLatch _archiveListLatch ;

   ossAtomic32 _activeListLen ;       /**< number of elements in the active list */
   UINT32 _archivedListLen ;          /**< number of elements in the archived list, protected by archivedListLatch */
   UINT32 _archivedListMaxLen ;       /**< maximum number of elements allowed in the archived list */

   ossAtomic32 _numPendingArchive ;   /**< Number of pending archive objects in the active list */
   ossAtomic32 _numPendingDelete ;    /**< Number of pending delete objects in the active list */

   MonDataLvl _curCollectionLvl ;     /**< The current data collection level for this class */
   MonDataLvl _minOperationalLvl ;    /**< The minimum data collection level when MonClass objects will get created */
   /*
    * Process pending archive/delete objects from the active list
    */
   void _processPendingObj() ;

   /*
    * Remove archived objects when the number of obj exceeds the capacity
    */
   void _removeArchivedObj() ;

public:
   UINT32 getActiveListLen() { return _activeListLen.fetch() ; }

   UINT32 getArchivedListLen() const { return _archivedList.size() ; }

   void setMaxArchivedListLen( UINT32 size ) { _archivedListMaxLen = size ; }

   UINT32 getMaxArchivedListLen() const { return _archivedListMaxLen ; }

   UINT32 getNumPendingArchive() { return _numPendingArchive.fetch() ; }

   UINT32 getNumPendingDelete() { return _numPendingDelete.fetch() ; }

   void getListLatch ( OSS_LATCH_MODE latchMode )
   {
      latchMode == EXCLUSIVE ? _activeListLatch.get()
                             : _activeListLatch.get_shared() ;
   }

   void releaseListLatch ( OSS_LATCH_MODE latchMode )
   {
      latchMode == EXCLUSIVE ? _activeListLatch.release()
                             : _activeListLatch.release_shared() ;
   }

   void getHeadLatch ( OSS_LATCH_MODE latchMode )
   {
      latchMode == EXCLUSIVE ? _activeListHeadLatch.get()
                             : _activeListHeadLatch.get_shared() ;
   }

   void releaseHeadLatch ( OSS_LATCH_MODE latchMode )
   {
      latchMode == EXCLUSIVE ? _activeListHeadLatch.release()
                             : _activeListHeadLatch.release_shared() ;
   }

   void getArchiveLatch ( OSS_LATCH_MODE latchMode )
   {
      latchMode == EXCLUSIVE ? _archiveListLatch.get()
                             : _archiveListLatch.get_shared() ;
   }

   void releaseArchiveLatch ( OSS_LATCH_MODE latchMode )
   {
      latchMode == EXCLUSIVE ? _archiveListLatch.release()
                             : _archiveListLatch.release_shared() ;
   }

   void setCollectionLvl( MonDataLvl mode )
   {
      _curCollectionLvl = mode ;
   }
   MonDataLvl getCollectionLvl() { return _curCollectionLvl ; }

   BOOLEAN isOperational()
   {
      return ( _curCollectionLvl >= _minOperationalLvl )? TRUE: FALSE ;
   }

   /**
    * Add an object to the active list
    * @return The object added to the container
    */
   template<class T>
   T* add ()
   {
      T* obj = NULL ;
      if ( isOperational() )
      {
         obj = SDB_OSS_NEW T() ;
         if ( obj )
         {
            obj->_dataLvl = getCollectionLvl() ;
            ossScopedLock l( &_activeListHeadLatch, EXCLUSIVE ) ;
            _activeList.push_front( *obj ) ;
            _activeListLen.inc() ;
         }
         else
         {
            //TODO dump error msg 
         }
      }
      return obj ;
   }

   /**
    * Add an object to the active list
    * @return The object added to the container
    */
   template<class T>
   T* add (_MonClassBaseData *data)
   {
      T* obj = NULL ;
      if ( isOperational() )
      {
         obj = SDB_OSS_NEW T(data) ;
         if ( obj )
         {
            obj->_dataLvl = getCollectionLvl() ;
            ossScopedLock l( &_activeListHeadLatch, EXCLUSIVE ) ;
            _activeList.push_front( *obj ) ;
            _activeListLen.inc() ;
         }
         else
         {
            //TODO dump error msg 
         }
      }
      return obj ;
   }

   /**
    * Remove an object from the active list. The object will get archived based on
    * the archive rule.
    * @param obj the object to be added
    */
   void remove ( _MonClass *obj ) ;

   /**
    * function pointer to specify the archive rule
    * @param obj the object to be archived
    */
   archiveFunc doArchive ;

   /**
    * Return iterator to the first node of the list

    * Dependency: appropriate latch has been taken
    * @param listType the type of list to read
    */
   iterator begin( MonClassListType listType )
   {
      SDB_ASSERT ( listType == MON_CLASS_ARCHIVED_LIST ||
                   listType == MON_CLASS_ACTIVE_LIST,
                   "Invalid monitor class list type" ) ;
      switch ( listType )
      {
         case ( MON_CLASS_ARCHIVED_LIST ):
            return iterator( _archivedList.begin() ) ;
         case ( MON_CLASS_ACTIVE_LIST ):
         default:
            return iterator( _activeList.begin() ) ;
      }
   }

   iterator end( MonClassListType listType )
   {
      SDB_ASSERT ( listType == MON_CLASS_ARCHIVED_LIST ||
                   listType == MON_CLASS_ACTIVE_LIST,
                   "Invalid monitor class list type" ) ;

      if ( listType == MON_CLASS_ARCHIVED_LIST )
      {
         return iterator( _archivedList.end() ) ;
      }
      else
      {
         return iterator( _activeList.end() ) ;
      }
   }
} ;

typedef _MonClassContainer MonClassContainer ;

class _MonClassReadScanner
{
private:
   typedef _MonClassContainer::iterator iterator ;

   _MonClassContainer *_container ; /**< container for the type of MonClass */
   iterator itr ;                  /**< iterator pointing to current node */
   MonClassListType _listType ;    /**< The types of nodes to scan (active/archive) */
   MonClassListType _currentList ; /**< The list currently being scanned */
   BOOLEAN _initCalled ;           /**< Whether scan initialization has been called */
   BOOLEAN _hasHeadLatch ;         /**< Whether headLatch has been obtained */
   BOOLEAN _hasListLatch ;         /**< Whether listLatch has been obtained */
   BOOLEAN _hasArchiveLatch ;      /**< Whether archiveLatch has been obtained */
   BOOLEAN _endReached ;           /**< Whether the scan has read all the nodes */

   void initScan() ;
   void doneScan() ;

public:
   _MonClassReadScanner( MonClassContainer *container, MonClassListType listType )
     : _container( container ),
       _listType( listType ),
       _initCalled( FALSE ),
       _hasHeadLatch( FALSE ),
       _hasListLatch( FALSE ),
       _hasArchiveLatch( FALSE ),
       _endReached( FALSE )
   {
   }

   ~_MonClassReadScanner()
   {
      doneScan() ;
   }

   /**
    * Return the next node
    */
   _MonClass* getNext() ;
} ;

typedef _MonClassReadScanner MonClassReadScanner ;
} // namespace engine

#endif //MONCLASS_HPP_
