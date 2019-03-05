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

   Source File Name = dpsTransVersionCtrl.hpp

   Descriptive Name = Data Protection Services Types Header

   When/how to use: this program may be used on binary and text-formatted
   versions of dps component. This file contains declare for data types used in
   SequoiaDB.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          11/05/2018  YXC Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef DPSTRANSVERSIONCTRL_HPP_
#define DPSTRANSVERSIONCTRL_HPP_

#include <map>
#include "dms.hpp"
#include "ixm.hpp"
#include "ossAtomic.hpp"
#include "ossLatch.hpp"
#include "ixmKey.hpp"
#include "dpsTransCB.hpp"
#include "dmsRecord.hpp"
#include "utilSegment.hpp"
#include "clsCatalogAgent.hpp"
#include "../bson/ordering.h"

namespace engine
{
   // class forward declare
   class preIdxTree;
   class dpsTransCB;
   class dpsTransLRBHeader;
   class oldVersionContainer;

   typedef  char    element16B[16];
   typedef  char    element32B[32];
   typedef  char    element128B[128];
   typedef  char    element1K[1024];
   typedef  char    element4K[4096];

   // FIXME: provide nub to update through config
   // All these numbers should be power of 2
   // default seg size for large record (1K/4K)
   #define  DEFAULT_SEG_SIZE_FOR_LARGE_REC   8192
   #define  MAX_SEG_SIZE_FOR_LARGE_REC   (DEFAULT_SEG_SIZE_FOR_LARGE_REC * 16)
   // default seg size for small record (16B/32B/128B)
   #define  DEFAULT_SEG_SIZE_FOR_SMALL_REC   (DEFAULT_SEG_SIZE_FOR_LARGE_REC*8)
   #define  MAX_SEG_SIZE_FOR_SMALL_REC   (MAX_SEG_SIZE_FOR_LARGE_REC*8)

   enum MEMBLOCKPOOL_TYPE
   {
      MEMBLOCKPOOL_TYPE_DYN = 0,  // dynamically allocate space
      MEMBLOCKPOOL_TYPE_16,       // allocate from 16B pool
      MEMBLOCKPOOL_TYPE_32,       // allocate from 32B pool
      MEMBLOCKPOOL_TYPE_128,
      MEMBLOCKPOOL_TYPE_1024,
      MEMBLOCKPOOL_TYPE_4096,
      MEMBLOCKPOOL_TYPE_MAX
   };

   // globIdxID uniquely define an index globally
   struct globIdxID
   {
      UINT32  _csID;   // collectionspace id
      UINT16  _clID;   // collection id
      SINT32  _idxLID; // index logic id
   } ;


   /** definition of memBlockPool
    *  memBlockPool is a place holder for a set of memory pools based on 
    *  the fixed size of element in the pool
    **/
   class memBlockPool : public SDBObject
   {
      // public interfaces:
      public: 
      // constructor 
      // default one, might use db config
      memBlockPool();

      //memBlockPool( UINT32 size1, UINT32 size2, Uint32 size3,
      //              UINT32 size4, UINT32 size5);

      // destructor
      ~memBlockPool();

      INT32 acquire( UINT32 const askSize, CHAR * &memBlock,
                     MEMBLOCKPOOL_TYPE * type );

      void  release( CHAR * &memBlock, MEMBLOCKPOOL_TYPE type );

      // FIXME: provide interface to access monitor counters

      // private attributes:
      private:
      _utilSegmentManager<element16B>  *_16BSeg; // mem segs with 16B element
      _utilSegmentManager<element32B>  *_32BSeg; // mem segs with 32B element 
      _utilSegmentManager<element128B> *_128BSeg;// mem segs with 128B element 
      _utilSegmentManager<element1K>   *_1KSeg;  // mem segs with 1 KB element
      _utilSegmentManager<element4K>   *_4KSeg;  // mem segs with 4 KB element

      // counters for monitor

      // how many times we dynamic alloc because we failed in each segment
      ossAtomic32  _numDynamicAlloc16B;  
      ossAtomic32  _numDynamicAlloc32B;
      ossAtomic32  _numDynamicAlloc128B;
      ossAtomic32  _numDynamicAlloc1K;
      ossAtomic32  _numDynamicAlloc4K;

   };

   /** definition of preIdxTreeNodeKey
    *  preIdxTreeNodeKey is the key for node in preIdxTree, it's child class
    *  ixmKey. Note that the raw key is stored in _keyData from super class,
    *  it's also a pointer pointing to a memory block owned by record lock
    **/
   //class preIdxTreeNodeKey : public ixmKey
   class preIdxTreeNodeKey : public ixmKeyOwned
   {
      // public interfaces:
      public: 
      // constructors:
      // use super class to construct key portion
      preIdxTreeNodeKey( const BSONObj*  key, const dmsRecordID & rid,
                         bson::Ordering * order )
      : ixmKeyOwned( *key )
      {
         _rid._extent = rid._extent;
         _rid._offset = rid._offset;
         _order = order;
      }
      
      // copy constructor
      preIdxTreeNodeKey( const preIdxTreeNodeKey & key )
      : ixmKeyOwned( key.toBson() )
      {
         _rid._extent = key._rid._extent;
         _rid._offset = key._rid._offset;
         _order = key._order;
      }

      ~preIdxTreeNodeKey ()
      {
         // We do not want to free the keyData in super class as we don't 
         // own it, simply rid to invalid incase some one continue using it.
         // delete of the lock LRB does the clean up of the key space.
         _rid.reset();
      }

      
      const bson::Ordering * getOrdering() const
      {
         return _order;
      }

      const dmsRecordID & getRID() const
      {
         return _rid;
      }

      BOOLEAN isValid() const
      {
         return _rid.isValid();
      }
      
      // assistant function to print out the rid ( and key )
      void printKey() const;

      // private attributes:
      private:
      // Original rid. This is used to differ duplicated keys when index is 
      // not unique
      dmsRecordID       _rid;
      // it's shared from the tree. Check clsCataOrder()
      bson::Ordering   *_order;
   };

   // the index tree node value is the index into the lrbHdr which contain
   // both old version index and record
   class preIdxTreeNodeValue : SDBObject
   {
      public:
      // constructor
      preIdxTreeNodeValue( dpsTransLRBHeader* lrbHdr )
      {
         _lrbHdr = lrbHdr;
      }

      ~preIdxTreeNodeValue()
      {
         _lrbHdr = NULL;
      }

      BOOLEAN isValid() const
      {
         return  ( NULL != _lrbHdr ) ;
      }

      void setValue( dpsTransLRBHeader* lrbHdr )
      {
         _lrbHdr = lrbHdr ;
      }

      dpsTransLRBHeader* &getLRBHdr()
      {
         return _lrbHdr;
      }

      // assistant function to print out the lrb hdr idx
      void printValue() const;
         
      // private member
      private:
      // index to lock LRB header, which contain old version record
      dpsTransLRBHeader*   _lrbHdr;

   };

   typedef std::pair<preIdxTreeNodeKey, preIdxTreeNodeValue> TREE_NODE_PAIR;

   // implement our own key comparison (less) function. 
   // Note that this does take the rid into consideration.
   class nodeKeyCompare
   {
      public:
      bool operator () ( const preIdxTreeNodeKey & lhk, 
                         const preIdxTreeNodeKey & rhk ) const
      {
         bool rv = false;
         const bson::Ordering * pOrdering = 
            ( NULL != lhk.getOrdering() ) ? lhk.getOrdering()
                                          : rhk.getOrdering() ;
         
         // compare key value using ixmKey compare
         INT32 rt = lhk.woCompare( rhk, *( pOrdering ) ) ; 
         if ( rt < 0 )
         {
            rv = true;
         }
         else if ( rt == 0 )// compare RID when key equals
         {
            // compare return -1 when lhk is before rhk
            if (lhk.getRID()._extent < rhk.getRID()._extent)
            {
               rv = true;
            }
            else if( lhk.getRID()._extent == rhk.getRID()._extent)
            {
               rv = (lhk.getRID()._offset < rhk.getRID()._offset);
            }
         }
         return rv;
      }
   };

   /*
   // FIXME: as an optimization, we should implement our own allocation
   // logic, instead of doing dynamic mem alloc, we should acquire/release
   // memory block from a mem pool
   // define my own allocator for the map(tree)
   class myNodeAlloc
   {
      // FIXME: To be implemented
      myNodeAlloc();

      // FIXME: allocate(), deallocate(), address(), To be implemented
      
      
   }
   */

   // use std::map which is implemented using red-black tree to hold 
   // old version index value
   typedef  std::map<preIdxTreeNodeKey, 
                     preIdxTreeNodeValue,
                     nodeKeyCompare
                     //myNodeAlloc
                     > INDEX_BINARY_TREE;

   //typedef  std::map<ixmKey, preIdxTreeNode> INDEX_BINARY_TREE;

   /** definition of preIdxTree
    *  preIdxTree is a red-black tree which holds all old key values of a 
    *  specific index during runtime. Index scanner will merge this in-memory
    *  tree with the index on disk during runtime based on its isolation level.
    *  The rule of thumb is: on-disk index contain latest value with could be 
    *  uncommitted. in-memory preIdxTree holds the last committed value.
    **/
   class preIdxTree : public SDBObject
   {
      // public interfaces:
      public: 
      // constructors & destructors
      preIdxTree(const UINT32 idxID, const ixmIndexCB * indexCB)
      {
         _idxLID = idxID;
         _tree = new INDEX_BINARY_TREE();
         _order = SDB_OSS_NEW clsCataOrder(
                     bson::Ordering::make (indexCB->keyPattern()) );
      }

      // copy constructor
      preIdxTree( const preIdxTree & intree )
      {
         _idxLID = intree.getLID();
         _tree =  intree.getTree() ;
         //_tree = new INDEX_BINARY_TREE( *(intree.getTree()) );
         _order = NULL;  // user has to setup the order themselves
         
      }

      // destructor
      ~preIdxTree() 
      {
         if ( NULL != _order )
         {
            SDB_OSS_DEL _order;
         }
         // this will remove all elements in the tree. 
         // Note that this will free up all the keyData as it's not owned 
         // by the tree
         if( _tree )
         {
            delete _tree;
            _tree = NULL;
         }
      }

      INDEX_BINARY_TREE * getTree() const
      {
         return _tree;
      }

      void setOrder ( const ixmIndexCB * indexCB )
      {
         _order = SDB_OSS_NEW clsCataOrder(
                     bson::Ordering::make (indexCB->keyPattern()) );
      }

      // get index logic ID
      UINT32 getLID() const 
      {
         return _idxLID;
      }

      bson::Ordering * getOrdering() const 
      {
         return _order->getOrdering();
      }

      // find a node based on the key, caller need to hold the latch
      // otherwise the iterator can change underneath
      INDEX_BINARY_TREE::iterator find ( const preIdxTreeNodeKey & key )
      {
         return _tree->find( key );
      }

      // find a node based on the key, caller need to hold the latch
      // otherwise the iterator can change underneath
      INDEX_BINARY_TREE::iterator find ( const BSONObj*  key,
                                         const dmsRecordID & rid )
      {
         preIdxTreeNodeKey myKey( key, rid, getOrdering() );
         return _tree->find( myKey );
      }

      // Traverse the tree to see if the key exist, caller need to hold 
      // the latch otherwise the iterator can change underneath
      BOOLEAN ixObjExist( const BSONObj&  key, preIdxTreeNodeValue & value );

      // locate the best match key in the tree based on the order and direction
      INT32 locate ( const BSONObj      &keyObj,
                     const dmsRecordID  &rid,
                     INDEX_BINARY_TREE::iterator &it,
                     INT32               direction );

      // move the iterator to the proper key location
      INT32 keyLocate( INDEX_BINARY_TREE::iterator &iter, const BSONObj &prevKey,
                       INT32 keepFieldsNum, BOOLEAN skipToNext,
                       const vector < const BSONElement *> &matchEle,
                       const vector < BOOLEAN > &matchInclusive,
                       INT32 direction, dpsTransCB* cb ) const;

      // Advance the pushed down verb and locate the key
      INT32 keyAdvance( INDEX_BINARY_TREE::iterator &iter,
                        const BSONObj &prevKey,
                        INT32 keepFieldsNum, BOOLEAN skipToNext,
                        const vector < const BSONElement *> &matchEle,
                        const vector < BOOLEAN > &matchInclusive,
                        INT32 direction, dpsTransCB *cb ) const;

      // iterator operation, caller need to hold the latch
      // otherwise the iterator can change underneath
      INDEX_BINARY_TREE::iterator begin ()
      {
         // start from the beginning of map
         return _tree->begin();
      }
      
      // iterator operation, caller need to hold the latch,
      // otherwise the iterator can change underneath
      INDEX_BINARY_TREE::iterator end ( )  
      {
         // start from the end of map
         return _tree->end();
      }

      void clear()
      {
         lockX();
         if (_tree)
         {
            _tree->clear();
         }
         unlockX();
      }
      // insert a node to map
      INT32 insert ( const preIdxTreeNodeKey &keyNode,
                     const preIdxTreeNodeValue & value,
                     const BOOLEAN lockHeld = FALSE );

      INT32 insert ( const BSONObj * keyData, const dmsRecordID & rid,
                     const preIdxTreeNodeValue & value,
                     const BOOLEAN lockHeld = FALSE );

      // delete a node
      INT32 remove( const preIdxTreeNodeKey & keyNode );
      INT32 remove( const BSONObj * keyData, const dmsRecordID & rid );
            
      void lockX()
      {
         _latch.get();
      }

      void lockS()
      {
         _latch.get_shared();
      }

      void unlockX()
      {
         _latch.release() ;
      }

      void unlockS()
      {
         _latch.release_shared();
      }
 
      BOOLEAN empty() const
      {
         return _tree->empty();
      }

      INT32 size() const
      {
         return _tree->size();
      }

      // assistant function to print out the whole tree.
      void printTree() const ;

      // private functions:
      private:

      // private attributes:
      private: 
      UINT32              _idxLID; // index logic id
      // Latching protocal
      // 1. preIdxTree latch must be held in X to insert/delete node in the tree
      //    oldVersionCB(_oldVersionCBLatch) need to be held in S before
      //    taking individual preIdxTree latch.
      // 2. Should never request LRB hash bkt latch while holding preIdxTree 
      //    latch, reverse order is OK. Keep in mind we store the _lrbHdrIdx
      //    in the tree so that we have direct access to lrbHdr without need
      //    to go through lrbhash bkt.
      ossSpinSLatch       _latch;  // latch for concurrency control, 
                                   // adding/removing node need latch in X
                                   // find/travers need latch in S
      INDEX_BINARY_TREE * _tree;   // tree to hold all old index key value
      clsCataOrder      * _order;  // wrap class to hold the shared ordering
   };

   // implement our own globalID comparison (less) function. 
   // it's ordered by  _csID, _clID and _idxLID
   struct globIDCompare
   {
      bool operator () ( const globIdxID &lid, 
                         const globIdxID &rid ) const
      {
         bool rv = false;
         // compare key value using ixmKey compare
         if ( lid._csID < rid._csID )
         {
            rv = true;
         }
         else if ( lid._csID == rid._csID )
         {
            if ( lid._clID < rid._clID )
            {
               rv = true;
            }
            else if (lid._clID == rid._clID)
            {
               if ( lid._idxLID < rid._idxLID )
               {
                  rv = true; 
               }
            }
         }
         return rv;
      }
   };

   // global map from an index to its own tree
   typedef  std::map< const globIdxID, 
                      const preIdxTree* ,
                      globIDCompare > IDXID_TO_TREE_MAP;

   typedef  std::pair< const globIdxID, 
                       const preIdxTree* > IDXID_TO_TREE_MAP_PAIR;

   /** definition of oldVersionCB
    *  Control block holding all resources and structures for old version 
    *  records and index keys. It's globally hanging of dpsTransCB
    **/
   class oldVersionCB : public SDBObject
   {
      // public interfaces
      public:
      // constructor
      oldVersionCB();

      // destructor
      ~oldVersionCB();

      const BOOLEAN isInitialized() const
      {
         return _initialized;
      }

      void latchS()
      {
         _oldVersionCBLatch.get_shared();
      }

      void latchX()
      {
         _oldVersionCBLatch.get();
      }

      void releaseX()
      {
         _oldVersionCBLatch.release();
      }

      void releaseS()
      {
         _oldVersionCBLatch.release_shared();
      }

      void addIdxTree( const globIdxID &gid, const ixmIndexCB * indexCB );
      void delIdxTree( const globIdxID &gid );

      INT32 insertIdxObj( const globIdxID     &gid, 
                          const BSONObj       &obj,
                          const dmsRecordID   &rid,
                          oldVersionContainer *oldVer,
                          const BOOLEAN       takeLock = TRUE );

      preIdxTree * getAndRemoveIdxTree( const globIdxID & gIdxID );

      preIdxTree * getIdxTree( const globIdxID & gIdxID );
      
      memBlockPool * getMemBlockPool() { return  _memBlockPool; }      

      // assistant function to print all tree's gid to diaglog
      void printTrees( BOOLEAN latched ) ;

      // counter to get number of indexes
      INT32 getNumTrees()
      {
         return _idxTrees->size();
      }

      // counter to get number of indexes
      INT32 getTotalTreeSize()
      {
         UINT32  totalNum = 0;
         for ( IDXID_TO_TREE_MAP::iterator it = _idxTrees->begin();
               it != _idxTrees->end(); it++ )
         {
            totalNum += it->second->size();
         }
         return totalNum;
      }

      // private functions
      private:

      // private attributes
      private:
      // latch to protect the fields. Should hold it in X to initialize and 
      // destroy _memBlockPool, otherwise hold in S
      ossSpinSLatch       _oldVersionCBLatch ;
      IDXID_TO_TREE_MAP * _idxTrees;  // trees holding older version of indexes
      memBlockPool      * _memBlockPool; // pool of memory blocks holding old 
                                         // version of records
      BOOLEAN             _initialized;  // flag indicating if it's initialized
   };

   class idxObj
   {
      public:
      idxObj()
      {
         _idxLID = -1;
         _order = NULL;
      }

      idxObj( const  idxObj & obj )
      {
         _idxLID = obj._idxLID;
         _order = obj._order;
         _idxObj = obj._idxObj.copy();
      }
      public:
      SINT32             _idxLID; // index unique logical ID
      //MEMBLOCKPOOL_TYPE  _recordMemType;  // which seg was _idxObj allocated
      bson::Ordering    *_order;  // pointer to ordering for comparison 
      BSONObj            _idxObj; // BSON Object
   };

   // implement our own idxObj comparison (less) function.
   // it's ordered by the unique _idxLID and bson obj
   class idxObjCompare
   {
      public: 

      bool operator () ( const idxObj &lobj,
                         const idxObj &robj ) const
      {
         bool rv = false;
         if ( lobj._idxLID < robj._idxLID )
         {
            rv = true;
         }
         else if( lobj._idxLID == robj._idxLID )
         {
            // same LID, do bson compare based on order
            rv = (lobj._idxObj.woCompare(robj._idxObj,
                                          *(lobj._order)) < 0);
         }
         return rv;
      }
   };

   // Use set of idx lid to figure out if the first version of an index value 
   // was already stored or not. 
   typedef std::set< SINT32 > idxLidSet;
   // use set of idxObj to store all index key values
   typedef std::set< idxObj, idxObjCompare > idxObjSet;

   // Class to store all information for old version record/indexes. This 
   // container is currently hanging off LRBHdr
   class oldVersionContainer : public SDBObject
   {
      public:
      oldVersionContainer(dpsTransLRBHeader* lrb )
      {
         _oldRecord = NULL;
         _order     = NULL;
         _lrbHdr  = lrb;
         _isNewRecord = FALSE;
      }

      // check if the index lid already exists in the set
      BOOLEAN idxLidExist(SINT32 id)
      {
         return ( _oldIdxLid.find(id) != _oldIdxLid.end() );
      }

      // based on index LID passed in, retrieve the index value
      BSONObj* getOldIdxValue(const  SINT32 lid )
      {
         BSONObj * obj = NULL;
         for ( idxObjSet::iterator i = _oldIdx.begin();
               i != _oldIdx.end(); ++i )
         {
            if ( i->_idxLID == lid )
            {
               obj = const_cast<BSONObj *> (&(i->_idxObj));
               break;
            }
         }
         return obj;
      }

      void setRecordNew() { _isNewRecord = TRUE; }
      void unsetRecordNew() { _isNewRecord = FALSE; }
      BOOLEAN isRecordNew() { return _isNewRecord; }

      void setOrder( clsCataOrder* o ) { _order = o ; }
      void freeOrder( )
      { 
         SDB_OSS_DEL _order ;
         _order = NULL ;
      }

      clsCataOrder* getOrder () { return _order ; }

      MEMBLOCKPOOL_TYPE & getRecordMemType() { return _recordMemType; }

      dmsRecord * &getOldRecord() {return _oldRecord; }
      void setOldRecord(dmsRecord * r) { _oldRecord = r; }

      BOOLEAN insertLID( const  SINT32 lid )
      {
         return _oldIdxLid.insert(lid).second;
      }

      void clearIdxLid() { _oldIdxLid.clear(); }

      BOOLEAN  insertOldIdxLid( const  SINT32 lid, ixmIndexCB* indexCB )
      {
         // only store order once
         if (!_order )
         {
            setOrder( SDB_OSS_NEW clsCataOrder(
                               Ordering::make(indexCB->keyPattern()) ) );
         }
         return insertLID(lid);
      }

      // given an index object, insert into the idxObjSet. Return false
      // if the same index for the record already exist. In this case,
      // the object was not inserted
      BOOLEAN insertIdx( idxObj & i )
      {
         // set the ordering pointer first
         i._order = _order->getOrdering();
         return _oldIdx.insert(i).second;
      }

      void deleteIdx( idxObjSet::iterator it )
      {
         _oldIdx.erase(it);
      }

      idxObjSet & getIdxSet ()  { return _oldIdx; }
      dpsTransLRBHeader*  const getLrbHdr()  { return _lrbHdr; }

      private:
      dmsRecord * _oldRecord;   // pointer to copy of old record
      dpsTransLRBHeader* _lrbHdr;   // LRB header index
      BOOLEAN     _isNewRecord; // is this a newly created record or not
      // A set of index Lids (up to 64) associated to this record.
      // We use this to figure out if the idx was already stored in the tree
      // Keep in mind that RC require us read the last committed version which
      // will be the version before first update in the transaction
      idxLidSet _oldIdxLid; 
      // pointer to the order for comparison
      clsCataOrder *_order;
      // point to a set containing all key sets.
      idxObjSet  _oldIdx;
      // tells which old record segment _oldRecord was allocated from
      MEMBLOCKPOOL_TYPE _recordMemType; 
   };
}

#endif //DPSTRANSVERSIONCTRL_HPP_

