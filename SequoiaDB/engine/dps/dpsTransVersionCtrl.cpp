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

   Source File Name = dpsTransVersionCtrl.cpp

   Descriptive Name = dps transaction version control

   When/how to use: this program may be used on binary and text-formatted
   versions of Data Protection component. This file contains functions for
   transaction isolation control through version control implmenetation.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          11/05/2018  YC  Initial Draft

   Last Changed =

*******************************************************************************/

#include "pmdCB.hpp"
#include "ossErr.h"
#include "dpsTransVersionCtrl.hpp"
#include "dpsTransLockMgr.hpp"
#include "ossMem.hpp"
#include "pdTrace.hpp"
#include "dpsTrace.hpp"
#include "ixmExtent.hpp" // for _keyCmp

namespace engine
{
   // memBlockPool default constructor, 
   memBlockPool::memBlockPool()
   : _numDynamicAlloc16B( 0 ),
     _numDynamicAlloc32B( 0 ),
     _numDynamicAlloc128B( 0 ),
     _numDynamicAlloc1K( 0 ),
     _numDynamicAlloc4K( 0 )

   {
      _16BSeg = new _utilSegmentManager<element16B>();
      _32BSeg = new _utilSegmentManager<element32B>();
      _128BSeg = new _utilSegmentManager<element128B>();
      _1KSeg = new _utilSegmentManager<element1K>();
      _4KSeg = new _utilSegmentManager<element4K>();

      _16BSeg->init( DEFAULT_SEG_SIZE_FOR_SMALL_REC,
                     MAX_SEG_SIZE_FOR_SMALL_REC );
      _32BSeg->init( DEFAULT_SEG_SIZE_FOR_SMALL_REC, 
                     MAX_SEG_SIZE_FOR_SMALL_REC );
      _128BSeg->init( DEFAULT_SEG_SIZE_FOR_SMALL_REC,
                      MAX_SEG_SIZE_FOR_SMALL_REC );
      _1KSeg->init( DEFAULT_SEG_SIZE_FOR_LARGE_REC,
                    MAX_SEG_SIZE_FOR_LARGE_REC );
      _4KSeg->init( DEFAULT_SEG_SIZE_FOR_LARGE_REC,
                    MAX_SEG_SIZE_FOR_LARGE_REC );

   }

   memBlockPool::~memBlockPool()
   {
      delete _16BSeg;
      delete _32BSeg;
      delete _128BSeg;
      delete _1KSeg;
      delete _4KSeg;
   }

   // Description:
   //   acquire memory block in proper pool based on the asked size.
   //   if we runout of space, we will dynamically allocate space
   // Input:
   //   askSize: size to alloc
   // Output:
   //   memBlock: address of the pointer to the memory block
   //   type:     pointer to where the memory was allocated
   // Return code:
   //   rc:  return SDB_OK if succeeded, otherwise error code
   //
   // PD_TRACE_DECLARE_FUNCTION ( SDB_MEMBLOCKPOOL_ACQUIRE, "memBlockPool::acquire" )
   INT32 memBlockPool::acquire( UINT32 const askSize,
                                CHAR     *  &memBlock,
                                MEMBLOCKPOOL_TYPE * type )
   {
      PD_TRACE_ENTRY( SDB_MEMBLOCKPOOL_ACQUIRE );
      INT32  rc = SDB_OK;

      // input argument NOT NULL and NOT 0 check
      SDB_ASSERT( type, "Invalid arguments" );
      SDB_ASSERT( (askSize > 0), "Invalid arguments" );

      // do block aquirement based on size
      if( askSize <= 16 )
      {
         element16B * mem;
         rc = _16BSeg->acquire( mem );
         if ( SDB_OK == rc )
         {
            *type = MEMBLOCKPOOL_TYPE_16;
            memBlock = (CHAR *) mem;
            goto done;
         }
         else
         {
            _numDynamicAlloc16B.inc();
            goto dynamic;
         }
      }
      else if( askSize <= 32 )
      {
         element32B * mem;
         rc = _32BSeg->acquire( mem );
         if ( SDB_OK == rc )
         {
            *type = MEMBLOCKPOOL_TYPE_32;
             memBlock = (CHAR *) mem;
            goto done;
         }
         else
         {
            _numDynamicAlloc32B.inc();
            goto dynamic;
         }
      }
      else if( askSize <= 128 )
      {
         element128B * mem;
         rc = _128BSeg->acquire( mem );
         if ( SDB_OK == rc )
         {
            *type = MEMBLOCKPOOL_TYPE_128;
             memBlock = (CHAR *) mem;
            goto done;
         }
         else
         {
            _numDynamicAlloc128B.inc();
            goto dynamic;
         }
      }
      else if( askSize <= 1024 )
      {
         element1K * mem;
         rc = _1KSeg->acquire( mem );
         if ( SDB_OK == rc )
         {
            *type = MEMBLOCKPOOL_TYPE_1024;
             memBlock = (CHAR *) mem;
            goto done;
         }
         else
         {
            _numDynamicAlloc1K.inc();
            goto dynamic;
         }
      }
      else if( askSize <= 4096 )
      {
         element4K * mem;
         rc = _4KSeg->acquire( mem );
         if ( SDB_OK == rc )
         {
            *type = MEMBLOCKPOOL_TYPE_4096;
             memBlock = (CHAR *) mem;
            goto done;
         }
         else
         {
            _numDynamicAlloc4K.inc();
            goto dynamic;
         }
      }
      
      // Failed to allocate from existing memory pool, dynamically alloc
      dynamic:
       memBlock = (CHAR*) SDB_OSS_MALLOC(askSize);
      if( NULL != memBlock )
      {
         *type = MEMBLOCKPOOL_TYPE_DYN;
      }
      else
      {
         rc = SDB_OOM;
         goto err;
      }

      done:
      PD_TRACE2( SDB_MEMBLOCKPOOL_ACQUIRE, PD_PACK_INT(askSize), PD_PACK_INT(*type) ); 
      PD_TRACE_EXITRC( SDB_MEMBLOCKPOOL_ACQUIRE, rc );
      return rc; 

      err:
      PD_LOG( PDERROR, "Having error when acquiring memory block: rc = %d. ", rc );
      goto done;
   }


   // Description:
   //   release memory block to proper pool based on the type
   // Input:
   //   memBlock: pointer to the memory block address
   //   type:     how the memory was allocated
   //
   // PD_TRACE_DECLARE_FUNCTION ( SDB_MEMBLOCKPOOL_RELEASE, "memBlockPool::release" )
   void memBlockPool::release( CHAR * &memBlock, MEMBLOCKPOOL_TYPE type )
   {
      PD_TRACE_ENTRY( SDB_MEMBLOCKPOOL_RELEASE );
      PD_TRACE1( SDB_MEMBLOCKPOOL_RELEASE, PD_PACK_INT(type) );
      INT32 rc = SDB_OK;

      SDB_ASSERT( ( NULL != memBlock ), "Invalid arguments" );
      switch (type) 
      {
         case MEMBLOCKPOOL_TYPE_16:
         {
            rc = _16BSeg->release( (element16B *) memBlock);
            break;
         }
         case MEMBLOCKPOOL_TYPE_32:
         {
            rc = _32BSeg->release( (element32B *) memBlock);
            break;
         }
         case MEMBLOCKPOOL_TYPE_128:
         {
            rc = _128BSeg->release( (element128B *) memBlock);
            break;
         }
         case MEMBLOCKPOOL_TYPE_1024:
         {
            rc = _1KSeg->release( (element1K *) memBlock);
            break;
         }
         case MEMBLOCKPOOL_TYPE_4096:
         {
            rc = _4KSeg->release( (element4K *) memBlock);
            break;
         }
         case MEMBLOCKPOOL_TYPE_DYN:
         {
            SDB_OSS_FREE(memBlock);
            break;
         }
         default:
         {
            PD_LOG( PDERROR, "Invalid type (%d)", type );
            SDB_ASSERT( FALSE, "Invalid arguments" );
            break;
         }
         memBlock = NULL;
      }
      // FIXME: this is debug level assert, what should we do in production??
      SDB_ASSERT( ( SDB_OK == rc ), "Sever error during release" ); 

      PD_TRACE_EXIT( SDB_MEMBLOCKPOOL_RELEASE );
   }

   // assistant function to print the index key
   void preIdxTreeNodeKey::printKey() const
   {
      try
      {
         PD_LOG( PDDEBUG, "====>preIdxTreeNodeKey: rid=(%d, %d), key=%s",
                 _rid._extent, _rid._offset, 
                 this->toString().c_str() );
      }
      catch( std::exception &e )
      {
         PD_LOG ( PDERROR, "Failed to printKey: %s",
                  e.what() ) ;
#ifdef _DEBUG
         SDB_ASSERT( FALSE, " printKey failed" );
#endif
      }

      return;
   }

   // assistant function to print the index value 
   // we can only print the lbrHdrIdx now
   void preIdxTreeNodeValue::printValue() const
   {
      PD_LOG( PDDEBUG, "====>preIdxTreeNodeValue=%d", _lrbHdrIdx );
      return;
   }

   // Description:
   //   insert a node to map. Latch is held within the function
   // Input:
   //   keyNode: key to insert
   //   value:   value to insert
   //   lockHeld: if the tree lock is already held or not
   // Return:
   //   SDB_OK if success.  Error code on any failure
   // PD_TRACE_DECLARE_FUNCTION ( SDB_PREIDXTREE_INSERT, "preIdxTree::insert" )
   INT32 preIdxTree::insert ( const preIdxTreeNodeKey &keyNode,
                              const preIdxTreeNodeValue & value,
                              const BOOLEAN lockHeld )
   {
      PD_TRACE_ENTRY( SDB_PREIDXTREE_INSERT );
      
      INT32 rc = SDB_OK;

      //input check
      SDB_ASSERT( keyNode.isValid(), "key is invalid");
      SDB_ASSERT( value.isValid() , "value is invalid");
      SDB_ASSERT( ( NULL != keyNode.getOrdering() ), "ordering is NULL");

//      TREE_NODE_PAIR myPair(keyNode, value);
      std::pair< INDEX_BINARY_TREE::iterator, bool > ret;

      // insert the pair into the map(tree)
      if( !lockHeld )
      {
         lockX();
      }


      //ret = _tree->insert(myPair);
      ret = _tree->insert( TREE_NODE_PAIR(keyNode, value) );

      if( !lockHeld )
      {
         unlockX();
      }

      // Insert failed due to identical key(key+rid). This should not happen.
      // Instead of panic, let's return err and leave caller to handle
      if ( false == ret.second )
      {
         rc = SDB_IXM_IDENTICAL_KEY;
         goto error;
      }

      done:
      PD_TRACE_EXITRC( SDB_PREIDXTREE_INSERT, rc );
      return rc;

      error:
      PD_LOG( PDERROR, "Trying to insert identical keys into the memory tree" );
      keyNode.printKey();
      value.printValue();
      
      goto done;
   }

   // Create a node from key and rid, insert the node to map(tree)
   // Dependency: 
   //   Caller has to held the tree latch in X
   INT32 preIdxTree::insert ( const BSONObj * keyData, const dmsRecordID & rid ,
                              const preIdxTreeNodeValue & value ,
                              const BOOLEAN lockHeld )
   {
      preIdxTreeNodeKey keyNode( keyData, rid, getOrdering() );

      PD_LOG( PDDEBUG, "preIdxTree:Inserting keyData, rid=(%d, %d)",
              rid._extent, rid._offset );
      return insert( keyNode, value, lockHeld );
   }
   
   // Description:
   //   delete a node from map. Latch is held within the function
   // Input:
   //   keyNode: key to delete
   // Return:
   //   Number of node deleted from the map
   // PD_TRACE_DECLARE_FUNCTION ( SDB_PREIDXTREE_REMOVE, "preIdxTree::remove" )
   INT32 preIdxTree::remove( const preIdxTreeNodeKey & keyNode )
   {
      PD_TRACE_ENTRY( SDB_PREIDXTREE_REMOVE );

      INT32 numDeleted = 0;

      lockX();

      numDeleted = _tree->erase( keyNode );
      unlockX();

#ifdef _DEBUG
      if ( 1 != numDeleted )
      {
         PD_LOG( PDERROR, "preIdxTree:removed %d nodes", numDeleted );
         printTree();
      }

      SDB_ASSERT( ( 1 == numDeleted ), "deleted other than one keys" ); 
#endif
      PD_TRACE_EXITRC( SDB_PREIDXTREE_REMOVE, numDeleted );
      return numDeleted;
   }

   INT32 preIdxTree::remove( const BSONObj * keyData, const dmsRecordID & rid )
   {
      preIdxTreeNodeKey keyNode( keyData, rid, getOrdering() );
#ifdef  _DEBUG    
      // can be used for debug purpose
      PD_LOG( PDDEBUG, "preIdxTree:removing keyData(%s), rid=(%d, %d)",
              keyData->toString().c_str(),
              rid._extent, rid._offset );
#endif
      return remove(keyNode);
   }

   // Description:
   //   Locate the best matching location based on the passed in idxkey obj
   //   and search criteria
   // Input:
   //   keyObj+rid: indexkey objct (with rid) to look for
   //   direction: search direction
   // Output:
   //   it: The iterator pointing to the best location.
   //   found: find the exact match or not
   // Return:
   //   SDB_OK:  normal return
   //   otherwise any popped error code
   // Dependency:
   //   caller must hold the tree latch in S
   //   FIXME:
   //   Is it possible to limit this interface to only be called in the tight
   //   loop within one tree latch cycle, then we can fully trust and only 
   //   use the iterator
   // PD_TRACE_DECLARE_FUNCTION ( SDB_PREIDXTREE_LOCATE, "preIdxTree::locate" )
   INT32 preIdxTree::locate ( const BSONObj      &keyObj,
                              const dmsRecordID  &rid,
                              INDEX_BINARY_TREE::iterator &it,
                              INT32               direction )
   {
      PD_TRACE_ENTRY( SDB_PREIDXTREE_LOCATE );
      INT32  rc = SDB_OK;
      preIdxTreeNodeKey myKey( &keyObj, rid, getOrdering() );

      // quick check for exact match
      INDEX_BINARY_TREE::iterator tempIter = _tree->find( myKey );
      if( tempIter != _tree->end() )
      {
         it = tempIter;
         goto done;
      }
      
      // if we are here, the passed in it is definitly invalid, directly use
      // map function to find the lower_bound or upper_bound 
      if( direction >0 )
      {
         
         tempIter = _tree->lower_bound(myKey);
      }
      else
      {
         tempIter = _tree->upper_bound(myKey);
      }

      if( tempIter != _tree->end() )
      {
         it = tempIter;
         goto done;
      }
      
      done:
      PD_TRACE_EXITRC( SDB_PREIDXTREE_LOCATE, rc );
      return rc;
   }

   // Description:
   //   Locate the best matching key location based on the provided key value
   //   and search criteria. The key value is likely saved from the last cycle
   //   before the pause. Since the tree structure can be changed due to 
   //   insertion and deletion, we don't bother to try and verify the saved
   //   iterator. Directly use the key value to find the best match key and
   //   start the new round from there.
   // Input:
   //   prevKey: previous indexkey to start with the search
   //   keepFieldsNum & skipToNext:
   //   keepFieldsNum is the number of fields from prevKey that should match 
   //   the currentKey (for example if the prevKey is {c1:1, c2:1}, and
   //   keepFieldsNum = 1, that means we want to match c1:1 key for the 
   //   current location. Depends on if we have skipToNext set, if we do
   //   that it means we want to skip c1:1 and match whatever the next
   //   (for example c1:1.1); otherwise we want to continue match the 
   //   elements from matchEle
   //   matchEle matchInclusive: push down matching criteria from access plan
   //   o: key order information
   //   direction: search direction
   // Output:
   //   iter: The iterator pointing to the best location.
   // Return:
   //   SDB_IXM_EOC: end of index search
   //   Any error coming from the function
   // Dependency: 
   //   Caller must hold tree latch in S/X
   // PD_TRACE_DECLARE_FUNCTION ( SDB_PREIDXTREE_KEYLOCATE, "preIdxTree::keyLocate" )
   INT32 preIdxTree::keyLocate
   ( INDEX_BINARY_TREE::iterator &iter, const BSONObj &prevKey,
     INT32 keepFieldsNum, BOOLEAN skipToNext,
     const vector < const BSONElement *> &matchEle,
     const vector < BOOLEAN > &matchInclusive,
     INT32 direction,
     dpsTransCB* cb ) const
   {
      PD_TRACE_ENTRY( SDB_PREIDXTREE_KEYLOCATE );

      INT32               rc           = SDB_OK;
      const BSONObj     * data         = NULL;
      UTIL_OBJIDX         hdrIdx       = 0;
      dpsTransLRBHeader * lrbHeaderPtr = NULL;
      BOOLEAN             first        = TRUE;
      
      // create a dummyRid to form a map key search
      dmsRecordID dummyRid; 

      if (this->empty())
      {
         rc = SDB_IXM_EOC;
         goto done;
      }
      // we will first use the above key to find a rough lower/upper bound,
      // but we need to further use match criteria to accurately locate the
      // best qualified key
      if( direction > 0 )
      { 
         do
         {
            if( first )
            {
               bson::Ordering * ordering = this->getOrdering();
               // FIXME: we need to use a better starting point by taking the 
               // into consideration
               // use the min dummy rid
               dummyRid.resetMin();
               // FIXME: use more information to create myKey. Note that the
               // initial key is always dummy, which means we will always start
               // with first key
               preIdxTreeNodeKey myKey( &prevKey, dummyRid,
                                        ordering );
               // if do forward scan, the lower bound would be our start
               iter = _tree->lower_bound(myKey);
               first = FALSE;
            }
            else
            {
               // move forward
               iter++;
            }
            // return EOC if we hit the end
            if( iter == _tree->end() )
            {
               rc = SDB_IXM_EOC;
               goto done;
            }

            // FIXME: debug code to validate the entry in the tree
            // retrieve the LRB header pointer and it should not be NULL because
            // lock should not be released before the tree is cleaned up. Same
            // reason, the _oldIdx should have been set up as well.
            hdrIdx = iter->second.getLRBHdrIdx();
            lrbHeaderPtr = cb->getLockMgrHandle()->getLRBHdrPtrByIdx(hdrIdx);
            if( (NULL == lrbHeaderPtr) ||
                (data = lrbHeaderPtr->getOldIdxValue(_idxLID)) == NULL )
            {
               rc = SDB_SYS;
               PD_LOG( PDERROR, "In memory idx tree points to non-exist lrbHeader"
                                "(hdrIdx=%d, header=%d)", hdrIdx, lrbHeaderPtr );
               printTree();
               SDB_ASSERT(FALSE, "LRB Header not valid");
               goto err;
            }
            
         }
         // compare the key value one by one until we found the first one
         // satisfying the matching criteria
         while ( _ixmExtent::_keyCmp( *data, prevKey, keepFieldsNum,
                          skipToNext, matchEle,
                          matchInclusive, *(getOrdering()), direction ) < 0 );
      }
      else //  (direction < 0)
      {
         do
         {
            if( first )
            {
/*
               bson::Ordering * ordering = this->getOrdering();
                 
               // we can use the max dummy rid, but there is no way to get 
               // a max bson. Let's simply start with last element.
               dummyRid.resetMax();
               preIdxTreeNodeKey myKey( &prevKey, dummyRid, ordering );
               // if do backward scan, the upper bound would be our start
               iter = _tree->upper_bound(myKey);
               // return EOC if we hit the end
               if( iter == _tree->end() )
               {
                  rc = SDB_IXM_EOC;
                  goto done;
               }
*/      
               // we can use the max dummy rid, but there is no way to get 
               // a max bson. Let's simply start with last element. Trick is
               // use reverse iterator (rbegin)'s next and convert to 
               // forward_iterator   
               // FIXME: we need to use a better starting point by taking the 
               // matchEle into consideration
               iter = (++(_tree->rbegin())).base();
               first = FALSE;
            }
            else
            {
               // can't move pass the begin
               if( iter == _tree->begin() )
               {
                  rc = SDB_IXM_EOC;
                  goto done;
               }
               // safe to move backward now
               iter--;
            }

            // retrieve the LRB header pointer and it should not be NULL because
            // lock should not be released before the tree is cleaned up. Same
            // reason, the _oldIdx should have been set up as well.
            hdrIdx = iter->second.getLRBHdrIdx();
            lrbHeaderPtr = cb->getLockMgrHandle()->getLRBHdrPtrByIdx(hdrIdx);
            if( (NULL == lrbHeaderPtr) ||
                (data = lrbHeaderPtr->getOldIdxValue(_idxLID)) == NULL )
            {
               rc = SDB_SYS;
               PD_LOG( PDERROR, "In memory idx tree points to non-exist lrbHeader"
                                "(idx=%d)", hdrIdx );
               goto err;
            }
            
         }
         // compare the key value one by one until we found the first one
         // satisfying the matching criteria
         while ( _ixmExtent::_keyCmp( *data, prevKey, keepFieldsNum,
                          skipToNext, matchEle,
                          matchInclusive, *(this->getOrdering()), direction ) > 0 );
      }

      // Once we reached here, we have found a best matching key pointed by 
      // the iter

      done:
      PD_TRACE_EXITRC( SDB_PREIDXTREE_KEYLOCATE, rc );
      return rc;

      err:
      PD_LOG( PDERROR, "Failed to locate the key:%s",
              prevKey.toString().c_str() );
      goto done;
   }

   // Description:
   //   After the listIterators have changed, we will advance the key to the
   //   best matching key location based on the provided cur location, prevkey
   //   and search criteria. The key value is likely saved from the last cycle
   //   before the pause. Since the tree structure can be changed due to
   //   insertion and deletion, we don't bother to try and verify the saved
   //   iterator. Directly use the key value to find the best match key and
   //   start the new round from there.
   // Input:
   //   iter: The iterator pointing to the current searching location.
   //   prevKey: previous indexkey to start with the search
   //   keepFieldsNum & skipToNext:
   //   keepFieldsNum is the number of fields from prevKey that should match
   //   the currentKey (for example if the prevKey is {c1:1, c2:1}, and
   //   keepFieldsNum = 1, that means we want to match c1:1 key for the
   //   current location. Depends on if we have skipToNext set, if we do
   //   that it means we want to skip c1:1 and match whatever the next
   //   (for example c1:1.1); otherwise we want to continue match the
   //   elements from matchEle
   //   matchEle matchInclusive: push down matching criteria from access plan
   //   direction: search direction
   // Output:
   //   iter: The iterator pointing to the best location.
   //   prevKey: 
   // Return:
   //   SDB_IXM_EOC: end of index search
   //   Any error coming from the function
   // Dependency:
   //   Caller must hold tree latch, the iter must not be tree end
   // PD_TRACE_DECLARE_FUNCTION ( SDB_PREIDXTREE_KEYADVANCE, "preIdxTree::keyAdvance" )
   INT32 preIdxTree::keyAdvance( INDEX_BINARY_TREE::iterator &iter,
                                 const BSONObj &prevKey,
                                 INT32 keepFieldsNum, BOOLEAN skipToNext,
                                 const vector < const BSONElement *> &matchEle,
                                 const vector < BOOLEAN > &matchInclusive,
                                 INT32 direction, dpsTransCB *cb ) const
   {
      PD_TRACE_ENTRY( SDB_PREIDXTREE_KEYADVANCE );
      INT32               rc           = SDB_OK;
      const BSONObj     * data         = NULL;
      UTIL_OBJIDX         hdrIdx       = 0;
      dpsTransLRBHeader * lrbHeaderPtr = NULL;
      
      if( direction > 0 )
      { 
         do
         {
            // move forward
            iter++;

            // return EOC if we hit the end
            if( iter == _tree->end() )
            {
               rc = SDB_IXM_EOC;
               goto done;
            }

            // retrieve the LRB header pointer and it should not be NULL because
            // lock should not be released before the tree is cleaned up. Same
            // reason, the _oldIdx should have been set up as well.
            hdrIdx = iter->second.getLRBHdrIdx();
            lrbHeaderPtr = cb->getLockMgrHandle()->getLRBHdrPtrByIdx(hdrIdx);
            if( (NULL == lrbHeaderPtr) ||
                (data = lrbHeaderPtr->getOldIdxValue(_idxLID)) == NULL )
            {
               rc = SDB_SYS;
               PD_LOG( PDERROR, "In memory idx tree points to non-exist lrbHeader"
                                "(idx=%d)", hdrIdx );
               goto err;
            }
         }
         // compare the key value one by one until we found the first one
         // satisfying the matching criteria
         while ( ixmExtent::_keyCmp( *data, prevKey, keepFieldsNum,
                          skipToNext, matchEle,
                          matchInclusive, *(getOrdering()), direction ) < 0 );
      }
      else //  (direction < 0)
      {
         do
         {
            // can't move pass the begin
            if( iter == _tree->begin() )
            {
               rc = SDB_IXM_EOC;
               goto done;
            }
            // safe to move backward now
            iter--;

            // retrieve the LRB header pointer and it should not be NULL because
            // lock should not be released before the tree is cleaned up. Same
            // reason, the _oldIdx should have been set up as well.
            hdrIdx = iter->second.getLRBHdrIdx();
            lrbHeaderPtr = cb->getLockMgrHandle()->getLRBHdrPtrByIdx(hdrIdx);
            if( (NULL == lrbHeaderPtr) ||
                (data = lrbHeaderPtr->getOldIdxValue(_idxLID)) == NULL )
            {
               rc = SDB_SYS;
               PD_LOG( PDERROR, "In memory idx tree points to non-exist lrbHeader"
                                "(idx=%d)", hdrIdx );
               goto err;
            }
            
         }
         // compare the key value one by one until we found the first one
         // satisfying the matching criteria
         while ( ixmExtent::_keyCmp( *data, prevKey, keepFieldsNum,
                          skipToNext, matchEle,
                          matchInclusive, *(getOrdering()), direction ) > 0 );
      } // end of else
      done:
      PD_TRACE_EXITRC( SDB_PREIDXTREE_KEYADVANCE, rc );
      return rc;

      err:
      PD_LOG( PDERROR, "Failed to advance the key:%s",
              prevKey.toString().c_str() );
      goto done;
   }

   // Traverse the tree to see if the key exist, caller need to hold
   // the latch otherwise the iterator can change underneath
   // PD_TRACE_DECLARE_FUNCTION ( SDB_PREIDXTREE_IXOBJEXIST, "preIdxTree::ixObjExist" )
   BOOLEAN preIdxTree::ixObjExist( const BSONObj&  key,
                                   preIdxTreeNodeValue & value ) 
   {
      PD_TRACE_ENTRY( SDB_PREIDXTREE_IXOBJEXIST );
      BOOLEAN found = FALSE;
      dmsRecordID  rid;
 
      rid.resetMin();
      preIdxTreeNodeKey lowKey( &key, rid, getOrdering() );
      rid.resetMax();
      preIdxTreeNodeKey highKey( &key, rid, getOrdering() );

      INDEX_BINARY_TREE::iterator startIter;
      INDEX_BINARY_TREE::iterator endIter;

      // based on order, choose the proper range to search
      if ( getOrdering()->get(_idxLID) > 0 )
      {
         startIter = _tree->lower_bound( lowKey );
         endIter = _tree->upper_bound( highKey );
      }
      else
      {
         startIter = _tree->lower_bound( highKey );
         endIter = _tree->upper_bound( lowKey );
      }

      // loop till hit end or pass endIter or found
      while ( startIter != _tree->end() ) 
      {
         if( startIter->first.toBson().equal(key) )
         {
            found = TRUE;
            value.setValue( startIter->second.getLRBHdrIdx()) ;
            break;
         }
         if( startIter != endIter )
         {
            startIter++;
         }
         else
         {
            break;
         }
      }

      PD_TRACE1( SDB_PREIDXTREE_IXOBJEXIST,  PD_PACK_UINT(found) );
      PD_TRACE_EXIT( SDB_PREIDXTREE_IXOBJEXIST );
      return found;
   }

   void preIdxTree::printTree() const
   {
      PD_LOG( PDDEBUG, "==>: print nodes for tree(%d), size=%d",
              _idxLID, this->size() );
      INDEX_BINARY_TREE::iterator it = _tree->begin();

      while ( it != _tree->end() )
      {
         it->first.printKey();
         it->second.printValue();
         it++;
      }

   }

   // constructor for oldVersionCB, will create in memory index trees
   // and _memBlockPool. This will be invoked during dpsTransCB construction.
   oldVersionCB::oldVersionCB()
   {
      // Initialize oldVersionCB for runtime if db config transisolation
      // is set to 1 (RC)
      SDB_ASSERT( (TRUE == pmdGetOptionCB()->transactionOn() ), 
                  "transactionOn must be enabled");
      latchX();
      _idxTrees = new IDXID_TO_TREE_MAP();
      _memBlockPool = new memBlockPool();
      _initialized = TRUE;
      releaseX();
      PD_LOG( PDINFO, "Initialized oldVersionCB" );
   }

   // destructor for oldVersionCB, will free up in memory index trees
   // and _memBlockPool. Caller should hold _oldVersionCBLatch(X) in dpsTransCB
   oldVersionCB::~oldVersionCB()
   {
      latchX();
      if ( TRUE == _initialized ) 
      {
         // loop through each inner tree and clear them before clear
         // the outter tree
         for ( IDXID_TO_TREE_MAP::iterator it = _idxTrees->begin();
            it != _idxTrees->end(); it++ )
         {
            // only give a warning as this could be expected during force 
            // shutdown when there are outstanding transactions
            PD_LOG( PDWARNING, 
                 "Trying to free up in memory index trees while no empty" );
            (const_cast<preIdxTree *>(it->second))->clear();
         }
         _idxTrees->clear();

         // delete of the tree will remove all elements in the map, which will
         // in turn call the destructor of each element class
         delete _idxTrees;
         _idxTrees = NULL;

         // free all mem pools
         delete _memBlockPool;
         _memBlockPool = NULL;

         _initialized = FALSE;
      }
      releaseX();
   }

   // based on global logic index id, get the in memory idx tree
   // caller must hold latch of oldVersionCB
   preIdxTree * oldVersionCB::getIdxTree( const globIdxID & gIdxID )
   {
      preIdxTree * tree = NULL;

      IDXID_TO_TREE_MAP::iterator it = _idxTrees->find(gIdxID);
      if ( it != _idxTrees->end() )    
      {
         tree = const_cast<preIdxTree *> (it->second);
      }
      return tree;  
   }

   // Create an in memory index tree and add to the map
   // Caller must hold _oldVersionCBLatch in X
   // PD_TRACE_DECLARE_FUNCTION ( SDB_OLDVERSIONCB_ADDIDXTREE, "oldVersionCB::addIdxTree" )
   void oldVersionCB::addIdxTree( const globIdxID &gid,
                                  const ixmIndexCB * indexCB )
   {
      PD_TRACE_ENTRY( SDB_OLDVERSIONCB_ADDIDXTREE );
#ifdef _DEBUG
      PD_LOG( PDDEBUG, "Going to add in memory Index tree for (%d,%d,%d)",
              gid._csID,
              gid._clID,
              gid._idxLID );
#endif
      // FIXME: Optimization to be considered
      // It's ok to have local variable t as the map will do memory allocation
      // However, this has performance overhead. 
      preIdxTree *t = SDB_OSS_NEW preIdxTree(gid._idxLID, indexCB);
      pair<IDXID_TO_TREE_MAP::iterator, bool> rv = 
         _idxTrees->insert( IDXID_TO_TREE_MAP_PAIR(gid, t) );

      // setup the order in the preIdxTree
      (const_cast<preIdxTree *> (rv.first->second))->setOrder(indexCB);
      
      PD_TRACE_EXIT( SDB_OLDVERSIONCB_ADDIDXTREE );
   }

   // Delete an in memory index tree and remove it from the map
   // _oldVersionCBLatch is held X in the function
   // PD_TRACE_DECLARE_FUNCTION ( SDB_OLDVERSIONCB_DELIDXTREE, "oldVersionCB::delIdxTree" )
   void oldVersionCB::delIdxTree( const globIdxID &gid )
   {
      preIdxTree * memTree = NULL;
      PD_TRACE_ENTRY( SDB_OLDVERSIONCB_DELIDXTREE );
#ifdef _DEBUG 
      // can enable this for debug purpose
      PD_LOG( PDDEBUG, "Going to delete in memory Index tree for (%d,%d,%d)",
              gid._csID,
              gid._clID,
              gid._idxLID );
#endif

      // Latch oldVCB and get the index tree
      latchX();
      memTree = getIdxTree( gid );

      if ( NULL != memTree )
      {
         memTree->clear();
         _idxTrees->erase(gid);
         SDB_OSS_DEL memTree;
      }

      releaseX();
      PD_TRACE_EXIT ( SDB_OLDVERSIONCB_DELIDXTREE );
   }

   // helper function to print the gids of all trees in _idxTrees
   // Input:
   //    latched: whether caller held the latch
   void oldVersionCB::printTrees( BOOLEAN latched = FALSE ) 
   {
      const preIdxTree *idxTree = NULL;
      if ( !latched )
      {
         _oldVersionCBLatch.get() ;
      }
      if ( TRUE == _initialized ) 
      {
         UINT32 i = 0;
         // loop through each inner tree and clear them before clear
         // the outter tree
         PD_LOG( PDINFO, "There are %d idx trees in _idxTrees, they are:",
                 _idxTrees->size() );

         for ( IDXID_TO_TREE_MAP::iterator it = _idxTrees->begin();
            it != _idxTrees->end(); it++, i++ )
         {
            idxTree = it->second;
            PD_LOG( PDINFO, "==>Idx tree (%d): gid=(%d, %d, %d)",
                    i, it->first._csID, it->first._clID, it->first._idxLID );
            // print each idx in the tree
            idxTree->printTree();
         }
      }
      if ( !latched )
      {
         _oldVersionCBLatch.release() ;
      }
   }

   // Description:
   //   Given an index object, find the proper in-memory tree and insert 
   // into it if the index is not already there. If the index already
   // exist, it's a no-op. This function will take proper tree latches
   // under the cover
   // Input: 
   //   gid: global ID of the index (csid, clid, idxid)
   //   obj: BSON containing the index
   //   rid: record ID
   //   oldVer: pointer to a constiner having older version record/idx
   //   
   // Dependency:
   //   The index tree should already exist. (addIdxTree() should have been
   // called by the caller earlier). Tree latch MUST have already been held.
   // PD_TRACE_DECLARE_FUNCTION ( SDB_OLDVERSIONCB_INSERTIDXOBJ, "oldVersionCB::insertIdxObj" )
   INT32 oldVersionCB::insertIdxObj( const globIdxID &gid, const BSONObj &obj,
                                     const dmsRecordID &rid, 
                                     oldVersionContainer *oldVer,
                                     const BOOLEAN takeLock )
   {
      INT32       rc      = SDB_OK;
      preIdxTree *idxTree = NULL;
      BOOLEAN     locked  = FALSE;

      PD_TRACE_ENTRY( SDB_OLDVERSIONCB_INSERTIDXOBJ );

      // search for the tree without latch as tree latch might have already
      // been held and tree is gurantee to exist
      idxTree = getIdxTree(gid);

      SDB_ASSERT( ( NULL != idxTree ), "index tree not exist" ); 

      // lock the inner idx tree for both find and insert
      if ( takeLock )
      {
         idxTree->lockX();
         locked = TRUE;
      }

      // check if it exist. We might be able to save this check if all 
      // callers did the check.
      //if( idxTree->find( (CHAR *) &obj, rid ) == idxTree->end() )
      if( idxTree->find( &obj, rid ) == idxTree->end() )
      {
         //not found, do the insert
         // first get the LRBHdr index from lock manager
         preIdxTreeNodeValue value(oldVer->getLrbHdrIdx());
         //int objSize = obj.objsize();

         // a lock obj for set to insert with. Note that the set will do memory 
         // allocation by its own
         idxObj  myIdxObj;
         myIdxObj._idxLID = gid._idxLID;
         // proper ordering pointer will be setup by the insert
         myIdxObj._order = NULL;
         // get memory from segment, copy the key obj in and hang off lrbHdr,
         // Note that this segment is not released until release of the lock
         // FIXME: having trouble acquire memory from _memBlockPool
         // should we do this under LRB bucket latch. The latching protocal is:
         // get idxTreeLatch before the lrbBucketLatch.
         /*
         _memBlockPool->acquire( objSize, (CHAR **)&(myIdxObj._idxObj),
                                 &(myIdxObj._recordMemType) );
         */
         // currently use a dummy one which is dynamic allocation
         // FIXME: the tree insert should call its allocate function which 
         // should use segment
         //myIdxObj._idxObj = new BSONObj();
         //myIdxObj._recordMemType = MEMBLOCKPOOL_TYPE_DYN;
         myIdxObj._idxObj = obj.copy();
 
         // insert to both idxset(oldVer) and idxTree
         if( oldVer->insertIdx(myIdxObj) )
         {
            idxTree->insert( &obj, rid, value, TRUE );
#ifdef _DEBUG
            PD_LOG( PDDEBUG, "Inserted key(%s) into memIXTree: "
                    "(csid=%d, clid=%d, idxlid=%d, rid=(%d, %d)",
                    obj.toString().c_str(),
                    gid._csID,
                    gid._clID,
                    gid._idxLID,
                    rid._extent,
                    rid._offset );
            //idxTree->printTree();
#endif
         }
         else
         {
            rc = SDB_SYS;
            PD_LOG( PDERROR, "Index does not exist in tree but failed to "
                    "insert to set (csid=%d, clid=%d, idxlid=%d, rid=(%d, %d)",
                    gid._csID,
                    gid._clID,
                    gid._idxLID,
                    rid._extent,
                    rid._offset );
            goto error;

         }

         PD_TRACE5( SDB_OLDVERSIONCB_INSERTIDXOBJ, 
                    PD_PACK_UINT(gid._csID),
                    PD_PACK_UINT(gid._clID),
                    PD_PACK_UINT(gid._idxLID),
                    PD_PACK_INT(rid._extent),
                    PD_PACK_INT(rid._offset) );
         
      }

      done: 
      if ( locked )
      {
         idxTree->unlockX();
      }
      PD_TRACE_EXITRC( SDB_OLDVERSIONCB_INSERTIDXOBJ, rc );
      return rc;
 
      error:
      PD_LOG( PDERROR, "Failed to insert an index obj" );
      goto done;
   }

}  // end of namespace
