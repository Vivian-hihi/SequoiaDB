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

#include "ossTypes.h"
#include "ossLatch.hpp"
#include "ossAtomic.hpp"
#include "ossUtil.hpp"
#include "../bson/bson.hpp"
#include "pd.hpp"
#include <boost/intrusive/list.hpp>
#include <iterator>

using namespace bson ;

namespace engine
{

typedef enum
{
   MON_CLASS_DATABASE,   // MonClassDatabase
   MON_CLASS_EDU,        // MonClassEDU
   MON_CLASS_QUERY,      // MonClassQuery
   MON_CLASS_MAX
} MonitorClassType ;

// Status for MonClass
#define MON_CLASS_STATUS_NORMAL   0x00
#define MON_CLASS_STATUS_PEND_DEL 0x01
#define MON_CLASS_STATUS_PEND_ARC 0x02

typedef boost::intrusive::list_base_hook< > BaseHook ;

/**
 * MonClass - Parent class for all monitor class
 *
 * It is up to the user to define the synchronization strategy for each MonClass.
 * For example, if a metric can be concurrently updated by more than one thread,
 * then the user might want to define that metric as atomic
 */
class MonClass : public BaseHook
{
   // TODO
   const ossTick _createTS ;     /**! create time for this object */
   UINT16  _status ;             /**! object status */

protected:
   MonitorClassType _type ;      /**! object type */

public:

   MonClass() ;
   virtual ~MonClass() ;

   /**
    * getType - return the type of the object
    */
   MonitorClassType getType () const { return _type; } 

   /**
    * setPendingDelete - mark an object to be pending delete
    */
   void setPendingDelete() { _status |= MON_CLASS_STATUS_PEND_DEL ; }
   BOOLEAN isPendingDelete() const { return ( _status & MON_CLASS_STATUS_PEND_DEL ) ? TRUE : FALSE ; }

   /**
    * setPendingArchive - mark an object to be pending archive
    */
   void setPendingArchive() { _status |= MON_CLASS_STATUS_PEND_ARC ; }
   BOOLEAN isPendingArchive() const { return ( _status & MON_CLASS_STATUS_PEND_ARC ) ? TRUE : FALSE ; }

   /**
    * getStatus() - get the current status
    */
   UINT16 getStatus() const { return _status ; }

   /**
    * Get the create timestamp
    */
   ossTick getCreateTS() const { return _createTS ; }

   /**
    * Format object into a BSON
    */
   virtual void dump( BSONObj &obj ) = 0 ;

   /**
    * Reset metrics
    */
   virtual void reset() = 0 ;
} ;

/**
 * Template to ensure all subclasses implement the getType function
 */
template<class T>
class MonClassTemplate : public MonClass
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
BOOLEAN archiveQuery ( MonClass *obj ) ;

/**
 * Default function pointer to indicate no archive
 */
BOOLEAN noArchive ( MonClass *obj ) ;


typedef enum
{
   MON_CLASS_ACTIVE_LIST,
   MON_CLASS_ARCHIVED_LIST
} MonClassListType ;

typedef enum
{
   MON_SCAN_TYPE_READ,
   MON_SCAN_TYPE_WRITE
} MonClassScanType ;

//TODO
class LatchTable
{};

//TODO
class LockWaitInfo
{};

/**
 * Capture metrics about a database
 * TODO: to be implemented
 */
class MonClassDatabase : public MonClassTemplate<MonClassDatabase>
{
public:
   MonClassDatabase()
   {
      _type = MON_CLASS_DATABASE ;
   }

   static MonitorClassType getType () { return MON_CLASS_DATABASE ; }

   virtual void dump( BSONObj &obj ) {}

   virtual void reset() {}
} ;

/*
 * Capture metrics about an EDU
 * TODO: to be implemented
 */
class MonClassEDU : public MonClassTemplate<MonClassEDU>
{
public:
   EDUID  _eduID ;
   UINT32 _queryProcessed ;

   MonClassEDU ()
     : _eduID( 0 ),
       _queryProcessed( 0 )
   {
      _type = MON_CLASS_EDU ;
   }

   static MonitorClassType getType () { return MON_CLASS_EDU ; }

   virtual void dump( BSONObj &obj ) {}

   virtual void reset() {}
} ;

/**
 * Capture metrics about a query
 */
class MonClassQuery : public MonClassTemplate<MonClassQuery>
{
public:
   ossTickDelta _responseTime;
   UINT32 _accessPlanID;
   UINT32 _transactionID;
   UINT32 _latchWaitTime;
   UINT32 _lockWaitTime;
   UINT32 _dataRead;
   UINT32 _dataWrite;

   LatchTable *_latchTable; // a table to record the latch it owns and waiting for
   LockWaitInfo *_lockWait; // structure to record the lock it is waiting for

   MonClassQuery ()
     :  _accessPlanID( 0 ),
        _transactionID( 0 ),
        _latchWaitTime( 0 ),
        _lockWaitTime( 0 ),
        _dataRead( 0 ),
        _dataWrite( 0 ),
        _latchTable( NULL ),
        _lockWait( NULL )
   {
      _type = MON_CLASS_QUERY ;
   }

   static MonitorClassType getType () { return MON_CLASS_QUERY ; }

   //TODO: to be implemented
   virtual void dump( BSONObj &obj ) {}

   //TODO: to be implemented
   virtual void reset() {}
} ;

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

typedef BOOLEAN (*archiveFunc)(MonClass *) ;

/**
 * MonClassContainer defines
 *
 * It is a container for a MonClass type. Contains an active list and archived list.
 * See below for latch protocol for these two lists.
 */
class MonClassContainer
{
friend class MonitorManager ;

   typedef boost::intrusive::list< MonClass, boost::intrusive::base_hook<BaseHook> > MonClassList ;

public:
   typedef listFwdIterator<MonClassList> iterator ;
   typedef listFwdIterator<const MonClassList> const_iterator ;

private:

   // disable assignment/copy
   MonClassContainer& operator= (const MonClassContainer& other) ;
   MonClassContainer( const MonClassContainer& other ) ;

   // a list of all the active objects
   MonClassList activeList ;

   // latch protecting the activeList
   // Protocol is as follows:
   // - Scanner gets the headLatch and latch in S. It releases the headLatch after making a copy of the list head.
   // - Delete is async, and does not need any latch. It simply marks the object as pending delete.
   // - Add gets the headLatch in X, then add a node to the head of the list.
   // - Real delete will be done by a background agent. It gets the headLatch in S and latch in X.
   //   It releases the headLatch after processing the list head.
   // - Latches should be acquired in this order: listLatch->archiveLatch->headLatch
   ossSpinSLatch activeListLatch ;
   ossSpinSLatch activeListHeadLatch ;

   // a list of all the archived objects
   MonClassList archivedList ;

   ossSpinSLatch archiveListLatch ;

   ossAtomic32 _activeListLen ;       /**< number of elements in the active list */
   UINT32 _archivedListLen ;          /**< number of elements in the archived list, protected by archivedListLatch */
   UINT32 _archivedListMaxLen ;       /**< maximum number of elements allowed in the archived list */

   ossAtomic32 _numPendingArchive ;   /**< Number of pending archive objects in the active list */
   ossAtomic32 _numPendingDelete ;    /**< Number of pending delete objects in the active list */

   BOOLEAN _active ;                  /**< Whether monitoring is on for this class */

   /*
    * Process pending archive/delete objects from the active list
    */
   void _processPendingObj() ;

   MonClassContainer (archiveFunc fp)
      : _activeListLen( 0 ),
        _archivedListLen( 0 ),
        _archivedListMaxLen( 100 ),
        _numPendingArchive( 0 ),
        _numPendingDelete( 0 ),
        _active ( FALSE ) 
   {
      doArchive = fp ;
   }

public:

   UINT32 getActiveListLen() { return _activeListLen.fetch() ; }

   UINT32 getArchivedListLen() const { return _archivedListLen ; }

   UINT32 getNumPendingArchive() { return _numPendingArchive.fetch() ; }

   UINT32 getNumPendingDelete() { return _numPendingDelete.fetch() ; }

   void getListLatch ( OSS_LATCH_MODE latchMode )
   {
      latchMode == EXCLUSIVE ? activeListLatch.get() : activeListLatch.get_shared() ;
   }

   void releaseListLatch ( OSS_LATCH_MODE latchMode )
   {
      latchMode == EXCLUSIVE ? activeListLatch.release()
                             : activeListLatch.release_shared() ;
   }

   void getHeadLatch ( OSS_LATCH_MODE latchMode )
   {
      latchMode == EXCLUSIVE ? activeListHeadLatch.get() : activeListHeadLatch.get_shared() ;
   }

   void releaseHeadLatch ( OSS_LATCH_MODE latchMode )
   {
      latchMode == EXCLUSIVE ? activeListHeadLatch.release()
                             : activeListHeadLatch.release_shared() ;
   }

   void getArchiveLatch ( OSS_LATCH_MODE latchMode )
   {
      latchMode == EXCLUSIVE ? archiveListLatch.get() : archiveListLatch.get_shared() ;
   }

   void releaseArchiveLatch ( OSS_LATCH_MODE latchMode )
   {
      latchMode == EXCLUSIVE ? archiveListLatch.release()
                             : archiveListLatch.release_shared() ;
   }

   /**
    * Add an object to the active list
    * @param obj the object to be added
    */
   void add ( MonClass *obj ) ;

   /**
    * Remove an object from the active list. The object will get archived based on
    * the archive rule.
    * @param obj the object to be added
    */
   void remove ( MonClass *obj ) ;

   /**
    * function pointer to specify the archive rule
    * @param obj the object to be archived
    */
   archiveFunc doArchive ;

   BOOLEAN isMonitorOn() const { return _active ; }
   void setMonitorStatus( BOOLEAN active ) { _active = active ; }

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
            return iterator( archivedList.begin() ) ;
         case ( MON_CLASS_ACTIVE_LIST ):
         default:
            return iterator( activeList.begin() ) ;
      }
   }

   iterator end( MonClassListType listType )
   {
      SDB_ASSERT ( listType == MON_CLASS_ARCHIVED_LIST ||
                   listType == MON_CLASS_ACTIVE_LIST,
                   "Invalid monitor class list type" ) ;

      if ( listType == MON_CLASS_ARCHIVED_LIST )
      {
         return iterator( archivedList.end() ) ;
      }
      else
      {
         return iterator( activeList.end() ) ;
      }
   }
} ;

class MonClassReadScanner
{
private:
   typedef MonClassContainer::iterator iterator ;

   MonClassContainer *_container ; /**< container for the type of MonClass */
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
   MonClassReadScanner( MonClassContainer *container, MonClassListType listType )
     : _container( container ),
       _listType( listType ),
       _initCalled( FALSE ),
       _hasHeadLatch( FALSE ),
       _hasListLatch( FALSE ),
       _hasArchiveLatch( FALSE ),
       _endReached( FALSE )
   {
   }

   ~MonClassReadScanner()
   {
      doneScan() ;
   }

   /**
    * Return the next node
    */
   MonClass* getNext() ;
} ;

class MonClassWriteScanner
{
private:
   typedef MonClassContainer::iterator iterator ;

   MonClassContainer *_container ; /**< container for the type of MonClass */
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

   MonClassWriteScanner( MonClassContainer *container, MonClassListType listType )
     : _container( container ),
       _listType( listType ),
       _initCalled( FALSE ),
       _hasHeadLatch( FALSE ),
       _hasListLatch( FALSE ),
       _hasArchiveLatch( FALSE ),
       _endReached( FALSE )
   {
   }

   ~MonClassWriteScanner()
   {
      doneScan() ;
   }

   MonClass* getNext() ;
} ;
} // namespace engine

#endif //MONCLASS_HPP_
