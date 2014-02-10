/*    Copyright 2012 SequoiaDB Inc.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

/** \file client.hpp
    \brief C++ Client Driver
*/

#ifndef CLIENT_HPP__
#define CLIENT_HPP__
#include "core.hpp"
#if defined (SDB_ENGINE) || defined (SDB_CLIENT)
#include "../bson/bson.h"
#include "../util/fromjson.hpp"
#else
#include "bson/bson.hpp"
#include "fromjson.hpp"
#endif
//#include "bson/bson.h"
//#include "jstobs.h"
#include "spd.h"
#include <map>
#include <string>
#include <vector>
#if defined (_WINDOWS)
   #if defined (SDB_DLL_BUILD)
      #define DLLEXPORT __declspec(dllexport)
   #else
      #define DLLEXPORT __declspec(dllimport)
   #endif
#else
   #define DLLEXPORT
#endif

#define SDB_PAGESIZE_4K           4096
#define SDB_PAGESIZE_8K           8192
#define SDB_PAGESIZE_16K          16384
#define SDB_PAGESIZE_32K          32768
#define SDB_PAGESIZE_64K          65536
#define SDB_PAGESIZE_DEFAULT      SDB_PAGESIZE_4K

#define SDB_SNAP_CONTEXTS         0
#define SDB_SNAP_CONTEXTS_CURRENT 1
#define SDB_SNAP_SESSIONS         2
#define SDB_SNAP_SESSIONS_CURRENT 3
#define SDB_SNAP_COLLECTIONS      4
#define SDB_SNAP_COLLECTIONSPACES 5
#define SDB_SNAP_DATABASE         6
#define SDB_SNAP_SYSTEM           7
#define SDB_SNAP_CATALOG          8

#define SDB_LIST_CONTEXTS         0
#define SDB_LIST_CONTEXTS_CURRENT 1
#define SDB_LIST_SESSIONS         2
#define SDB_LIST_SESSIONS_CURRENT 3
#define SDB_LIST_COLLECTIONS      4
#define SDB_LIST_COLLECTIONSPACES 5
#define SDB_LIST_STORAGEUNITS     6
#define SDB_LIST_SHARDS           7
#define SDB_LIST_STOREPROCEDURES  8

#define FLG_INSERT_CONTONDUP  0x00000001

/** SDB_LIST_GROUPS will be deprecated in version 2.x, use SDB_LIST_SHARDS instead of it. */
#define SDB_LIST_GROUPS        SDB_LIST_SHARDS
/** class name 'sdbReplicaGroup' will be deprecated in version 2.x, use 'sdbShard' instead of it. */
#define sdbReplicaGroup        sdbShard
/** class name 'sdbReplicaNode' will be deprecated in version 2.x, use 'sdbNode' instead of it. */
#define sdbReplicaNode         sdbNode
/** sdb::listReplicaGroups will be deprecated in version 2.x, use sdb::listShards instead of it. */
#define listReplicaGroups      listShards
/** sdb::getReplicaGroup will be deprecated in version 2.x, use sdb::getShard instead of it. */
#define getReplicaGroup        getShard
/** sdb::createReplicaGroup will be deprecated in version 2.x, use sdb::createShard instead of it. */
#define createReplicaGroup     createShard
/** sdb::removeReplicaGroup will be deprecated in version 2.x, use sdb::removeShard instead of it. */
#define removeReplicaGroup     removeShard
/** sdb::createReplicaCataGroup will be deprecated in version 2.x, use sdb::createCataShard instead of it. */
#define createReplicaCataGroup createCataShard
/** sdb::activateReplicaGroup will be deprecated in version 2.x, use sdb::activateShard instead of it. */
#define activateReplicaGroup   activateShard

/** \namespace sdbclient
    \brief SequoiaDB Driver for C++
*/
namespace sdbclient
{
   const static bson::BSONObj _sdbStaticObject ;
   class _sdbCursor ;
   class _sdbCollection ;
   class sdb ;
   class _sdb ;
   class _ossSocket ;

   class DLLEXPORT _sdbCursor
   {
   private :
      _sdbCursor ( const _sdbCursor& other ) ;
      _sdbCursor& operator=( const _sdbCursor& ) ;
   public :
      _sdbCursor () {}
      virtual ~_sdbCursor () {}
      virtual INT32 next          ( bson::BSONObj &obj ) = 0 ;
      virtual INT32 current       ( bson::BSONObj &obj ) = 0 ;
      //virtual INT32 updateCurrent ( bson &rule ) = 0 ;
      //virtual INT32 delCurrent    () = 0 ;
   } ;

/** \class  sdbCursor
      \brief Database operation interfaces of cursor.
*/
   class DLLEXPORT sdbCursor
   {
   private :
      sdbCursor ( const sdbCursor& other ) ;
      sdbCursor& operator=( const sdbCursor& ) ;
   public :
/** \var pCursor
      \breif A pointer of virtual base class _sdbCursor

      Class sdbCursor is a shell for _sdbCursor.We use pCursor to 
      call the methods in class _sdbCursor.
*/
      _sdbCursor *pCursor ;

/** \fn sdbCursor ()
      \brief default constructor
*/
      sdbCursor ()
      {
         pCursor = NULL ;
      }

/** \fn ~sdbCursor ()
      \brief destructor
*/
      ~sdbCursor ()
      {
         if ( pCursor )
            delete pCursor ;
      }

/** \fn  INT32 next ( bson::BSONObj &obj )
      \brief Return the next document of current cursor, and move forward
      \param [out] obj The return bson object
      \retval SDB_OK Operation Success
      \retval Others Operation Fail
*/
      INT32 next ( bson::BSONObj &obj )
      {
         if ( !pCursor )
            return SDB_NOT_CONNECTED ;
         return pCursor->next ( obj ) ;
      }

/** \fn INT32 current ( bson::BSONObj &obj )
      \brief Return the current document of cursor, and don't move
      \param [out] obj The return bson object
      \retval SDB_OK Operation Success
      \retval Others Operation Fail
*/
      INT32 current ( bson::BSONObj &obj )
      {
         if ( !pCursor )
            return SDB_NOT_CONNECTED ;
         return pCursor->current ( obj ) ;
      }
/*
* \fn INT32 updateCurrent ( bson::BSONObj &rule )
    \brief Update the current document of cursor
    \param [in] rule The updating rule, cannot be null
    \retval SDB_OK Operation Success
    \retval Others Operation Fail
      INT32 updateCurrent ( bson::BSONObj &rule )
      {
         if ( !pCursor )
            return SDB_NOT_CONNECTED ;
         return pCursor->updateCurrent ( rule ) ;
      }

* \fn INT32 delCurrent ()
      \brief Delete the current document of cursor
      \retval SDB_OK Operation Success
      \retval Others Operation Fail

      INT32 delCurrent ()
      {
         if ( !pCursor )
            return SDB_NOT_CONNECTED ;
         return pCursor->delCurrent () ;
      }*/
   } ;

   class DLLEXPORT _sdbCollection
   {
   private :
      _sdbCollection ( const _sdbCollection& other ) ;
      _sdbCollection& operator=( const _sdbCollection& ) ;
   public :
      _sdbCollection () {}
      virtual ~_sdbCollection () {}
      // get the total number of records for a given condition, if the condition
      // is NULL then match all records in the collection
      virtual INT32 getCount ( SINT64 &count,
                               const bson::BSONObj &condition = _sdbStaticObject ) = 0 ;

      // insert a bson object into current collection
      // given:
      // object ( required )
      // returns id as the pointer pointing to _id bson element
      virtual INT32 insert ( const bson::BSONObj &obj, bson::BSONElement *id = NULL ) = 0 ;

      virtual INT32 bulkInsert ( SINT32 flags,
                                 std::vector<bson::BSONObj> &obj
                               ) = 0 ;
      // update bson object from current collection
      // given:
      // update rule ( required )
      // update condition ( optional )
      // hint ( optional )
      virtual INT32 update ( const bson::BSONObj &rule,
                             const bson::BSONObj &condition = _sdbStaticObject,
                             const bson::BSONObj &hint      = _sdbStaticObject
                           ) = 0 ;

      // update bson object from current collection, if there's nothing match
      // then insert an record that modified from empty BSON object
      // given:
      // update rule ( required )
      // update condition ( optional )
      // hint ( optional )
      virtual INT32 upsert ( const bson::BSONObj &rule,
                             const bson::BSONObj &condition = _sdbStaticObject,
                             const bson::BSONObj &hint      = _sdbStaticObject
                           ) = 0 ;

      // delete bson objects from current collection
      // given:
      // delete condition ( optional )
      // hint ( optional )
      virtual INT32 del ( const bson::BSONObj &condition = _sdbStaticObject,
                          const bson::BSONObj &hint      = _sdbStaticObject
                        ) = 0 ;

      // query objects from current collection
      // given:
      // query condition ( optional )
      // query selected def ( optional )
      // query orderby ( optional )
      // hint ( optional )
      // output: _sdbCursor ( required )
      virtual INT32 query  ( _sdbCursor **cursor,
                             const bson::BSONObj &condition = _sdbStaticObject,
                             const bson::BSONObj &selected  = _sdbStaticObject,
                             const bson::BSONObj &orderBy   = _sdbStaticObject,
                             const bson::BSONObj &hint      = _sdbStaticObject,
                             INT64 numToSkip    = 0,
                             INT64 numToReturn  = -1
                           ) = 0 ;

      virtual INT32 query  ( sdbCursor &cursor,
                             const bson::BSONObj &condition = _sdbStaticObject,
                             const bson::BSONObj &selected  = _sdbStaticObject,
                             const bson::BSONObj &orderBy   = _sdbStaticObject,
                             const bson::BSONObj &hint      = _sdbStaticObject,
                             INT64 numToSkip    = 0,
                             INT64 numToReturn  = -1
                           ) = 0 ;
      //virtual INT32 rename ( const CHAR *pNewName ) = 0 ;
      // create an index for the current collection
      // given:
      // index definition ( required )
      // index name ( required )
      // uniqueness ( required )
      // enforceness ( required )
      virtual INT32 createIndex ( const bson::BSONObj &indexDef,
                                  const CHAR *pName,
                                  BOOLEAN isUnique,
                                  BOOLEAN isEnforced
                                ) = 0 ;
      virtual INT32 getIndexes ( _sdbCursor **cursor,
                                 const CHAR *pName ) = 0 ;
      virtual INT32 getIndexes ( sdbCursor &cursor,
                                 const CHAR *pName ) = 0 ;
      virtual INT32 dropIndex ( const CHAR *pName ) = 0 ;
      virtual INT32 create () = 0 ;
      virtual INT32 drop () = 0 ;
      virtual const CHAR *getCollectionName () = 0 ;
      virtual const CHAR *getCSName () = 0 ;
      virtual const CHAR *getFullName () = 0 ;
      virtual INT32 split ( const CHAR *sourceShardName,
                            const CHAR *destShardName,
                            const bson::BSONObj &splitConditon,
                            const bson::BSONObj &splitEndCondition = _sdbStaticObject) = 0 ;
      virtual INT32 split ( const CHAR *sourceShardName,
                            const CHAR *destShardName,
                            FLOAT64 percent ) = 0 ;
      virtual INT32 splitAsync ( SINT64 &taskID,
                            const CHAR *sourceShardName,
                            const CHAR *destShardName,
                            const bson::BSONObj &splitCondition,
                            const bson::BSONObj &splitEndCondition = _sdbStaticObject) = 0 ;
      virtual INT32 splitAsync ( const CHAR *sourceShardName,
                            const CHAR *destShardName,
                            FLOAT64 percent,
                            SINT64 &taskID ) = 0 ;
      virtual  INT32 aggregate ( _sdbCursor **cursor,
                                std::vector<bson::BSONObj> &obj
                               )  = 0 ;
      virtual  INT32 aggregate ( sdbCursor &cursor,
                                std::vector<bson::BSONObj> &obj
                               )  = 0 ;
      virtual INT32 getQueryMeta  ( _sdbCursor **cursor,
                             const bson::BSONObj &condition = _sdbStaticObject,
                             const bson::BSONObj &orderBy   = _sdbStaticObject,
                             const bson::BSONObj &hint  = _sdbStaticObject,
                             INT64 numToSkip    = 0,
                             INT64 numToReturn  = -1
                           ) = 0 ;
      virtual INT32 getQueryMeta  ( sdbCursor &cursor,
                             const bson::BSONObj &condition = _sdbStaticObject,
                             const bson::BSONObj &orderBy   = _sdbStaticObject,
                             const bson::BSONObj &hint  = _sdbStaticObject,
                             INT64 numToSkip    = 0,
                             INT64 numToReturn  = -1
                           ) = 0 ;
      virtual INT32 attachCollection ( const CHAR *subClFullName,
                                      const bson::BSONObj &options) = 0 ;
      virtual INT32 detachCollection ( const CHAR *subClFullName) = 0 ;

      //virtual INT32 alter ( const bson *options ) = 0 ;
   } ;

/** \class sdbCollection
      \brief Database operation interfaces of cursor.
*/
   class DLLEXPORT sdbCollection
   {
   private :
/** \fn sdbCollection ( const sdbCollection& other ) ;
      \brief Copy constructor
      \param[in] A const object reference of class sdbCollection.
*/
      sdbCollection ( const sdbCollection& other ) ;

/** \fn sdbCollection& operator=( const sdbCollection& )
      \brief Assignment constructor
      \param[in] a const reference of class sdbCollection.
      \retval A const object reference of class sdbCollection.
*/
      sdbCollection& operator=( const sdbCollection& ) ;
   public :
/** \var pCollection
      \breif A pointer of virtual base class _sdbCollection

      Class sdbCollection is a shell for _sdbCollection.We use pCollection to
      call the methods in class _sdbCollection.
*/
      _sdbCollection *pCollection ;

/** \fn sdbCollection ()
    \brief Default constructor
*/
      sdbCollection ()
      {
         pCollection = NULL ;
      }

/** \fn ~sdbCollection ()
    \brief Destructor.
*/
      ~sdbCollection ()
      {
         if ( pCollection )
            delete pCollection ;
      }

/** \fn INT32 getCount ( SINT64 &count,
                         const bson::BSONObj &condition )
    \brief Get the count of matching documents in current collection.
    \param [in] condition The matching rule, return the count of all documents if this parameter is empty
    \param [out] count The count of matching documents, matches all records if not provided.
    \retval SDB_OK Operation Success
    \retval Others Operation Fail
*/
      INT32 getCount ( SINT64 &count,
                       const bson::BSONObj &condition = _sdbStaticObject )
      {
         if ( !pCollection )
            return SDB_NOT_CONNECTED ;
         return pCollection->getCount ( count, condition ) ;
      }

/** \fn INT32 split ( const CHAR *sourceShardName,
                      const CHAR *destShardName,
                      const bson::BSONObj &splitCondition,
                      const bson::BSONObj &splitEndCondition)
    \brief Split the specified collection from source shard to target shard by range.
    \param [in] sourceShardName The source shard name
    \param [in] destShardName The target shard name
    \param [in] splitCondition The split condition
    \param [in] splitEndCondition The split end condition or null
              eg:If we create a collection with the option {ShardingKey:{"age":1},ShardingType:"Hash",Partition:2^10},
             we can fill {age:30} as the splitCondition, and fill {age:60} as the splitEndCondition. when split,
             the target shard will get the records whose age's hash value are in [30,60). If splitEndCondition is null,
             they are in [30,max).
    \retval SDB_OK Operation Success
    \retval Others Operation Fail
*/
      INT32 split ( const CHAR *sourceShardName,
                    const CHAR *destShardName,
                    const bson::BSONObj &splitCondition,
                    const bson::BSONObj &splitEndCondition = _sdbStaticObject)
      {
         if ( !pCollection )
            return SDB_NOT_CONNECTED ;
         return pCollection->split ( sourceShardName,
                                     destShardName,
                                     splitCondition,
                                     splitEndCondition) ;
      }

/** \fn INT32 split ( const CHAR *sourceShardName,
                      const CHAR *destShardName,
                      FLOAT64 percent )
    \brief Split the specified collection from source shard to target by percent.
    \param [in] sourceShardName The source shard name
    \param [in] destShardName The target shard name
    \param [in] percent The split percent, Range:(0,100]
    \retval SDB_OK Operation Success
    \retval Others Operation Fail
*/
      INT32 split ( const CHAR *sourceShardName,
                    const CHAR *destShardName,
                    FLOAT64 percent )
      {
         if ( !pCollection )
            return SDB_NOT_CONNECTED ;
         return pCollection->split ( sourceShardName,
                                     destShardName,
                                     percent ) ;
      }

/** \fn INT32 splitAsync ( SINT64 &taskID,
                                   const CHAR *sourceShardName,
                                   const CHAR *destShardName,
                                   const bson::BSONObj &splitCondition,
                                   const bson::BSONObj &splitEndCondition )
    \brief Split the specified collection from source shard to target by range
    \param [out] taskID The id of current split task
    \param [in] sourceShardName The source shard name
    \param [in] destShardName The target shard name
    \param [in] splitCondition The split condition
    \param [in] splitEndCondition The split end condition or null
              eg:If we create a collection with the option {ShardingKey:{"age":1},ShardingType:"Hash",Partition:2^10},
              we can fill {age:30} as the splitCondition, and fill {age:60} as the splitEndCondition. when split,
              the target shard will get the records whose age's hash value are in [30,60). If splitEndCondition is null,
              they are in [30,max).
    \retval SDB_OK Operation Success
    \retval Others Operation Fail
*/
      INT32 splitAsync ( SINT64 &taskID,
                                   const CHAR *sourceShardName,
                                   const CHAR *destShardName,
                                   const bson::BSONObj &splitCondition,
                                   const bson::BSONObj &splitEndCondition = _sdbStaticObject )
      {
         if ( !pCollection )
            return SDB_NOT_CONNECTED ;
         return pCollection->splitAsync ( taskID,
                                     sourceShardName,
                                     destShardName,
                                     splitCondition,
                                     splitEndCondition ) ;
      }


/** \fn INT32 INT32 splitAsync ( const CHAR *pSourceShard,
                                 const CHAR *pTargetShard,
                                 FLOAT64 percent,
                                 SINT64 &taskID )
    \brief Split the specified collection from source shard to target by percent
    \param [in] sourceShardName The source shard name
    \param [in] destShardName The target shard name
    \param [in] percent The split percent, Range:(0.0, 100.0]
    \param [out] taskID The id of current split task
    \retval SDB_OK Operation Success
    \retval Others Operation Fail
*/
      INT32 splitAsync ( const CHAR *sourceShardName,
                                    const CHAR *destShardName,
                                    FLOAT64 percent,
                                    SINT64 &taskID )
      {
         if ( !pCollection )
            return SDB_NOT_CONNECTED ;
         return pCollection->splitAsync ( sourceShardName,
                                     destShardName,
                                     percent,
                                     taskID ) ;
      }


/* \fn INT32 alter ( const bson *options )
    \brief Alter the specified collection
    \param [in] options The modified options as following:
                        ReplSize Number of replnodes for sync write
    \retval SDB_OK Operation Success
    \retval Others Operation Fail

      INT32 alter ( const bson *options )
      {
         if ( !pCollection )
            return SDB_NOT_CONNECTED ;
         return pCollection->alter ( options ) ;
      }*/

/** \fn  INT32 bulkInsert ( SINT32 flags,
                         std::vector<bson::BSONObj> &obj
                       )
    \brief Insert a bulk of bson objects into current collection
    \param [in] flags FLG_INSERT_CONTONDUP or 0
    \param [in] obj The array of inserted bson objects
    \retval SDB_OK Operation Success
    \retval Others Operation Fail
*/
      INT32 bulkInsert ( SINT32 flags,
                         std::vector<bson::BSONObj> &obj
                       )
      {
         if ( !pCollection )
            return SDB_NOT_CONNECTED ;
         return pCollection->bulkInsert ( flags, obj ) ;
      }

/** \fn INT32 insert ( bson::BSONObj &obj, BSONElement *id = NULL )
    \brief Insert a bson object into current collection
    \param [in] obj The inserted bson object
    \param [out] id The object id of inserted bson object in current collection, the memory of id will be invalidated when next insert/bulkInsert is performed or the obj is destroyed
    \retval SDB_OK Operation Success
    \retval Others Operation Fail
*/
      INT32 insert ( const bson::BSONObj &obj, bson::BSONElement *id = NULL )
      {
         if ( !pCollection )
            return SDB_NOT_CONNECTED ;
         return pCollection->insert ( obj, id ) ;
      }

/** \fn  INT32 update ( const bson::BSONObj &rule,
                     const bson::BSONObj &condition,
                     const bson::BSONObj &hint
                   )
    \brief Update the matching documents in current collection
    \param [in] rule The updating rule
    \param [in] condition The matching rule, update all the documents if not provided
    \param [in] hint The hint, automatically match the optimal hint if not provided
    \retval SDB_OK Operation Success
    \retval Others Operation Fail
    \note It won't work to update the "ShardingKey" field, but the other fields take effect
*/
      INT32 update ( const bson::BSONObj &rule,
                     const bson::BSONObj &condition = _sdbStaticObject,
                     const bson::BSONObj &hint      = _sdbStaticObject
                   )
      {
         if ( !pCollection )
            return SDB_NOT_CONNECTED ;
         return pCollection->update ( rule, condition, hint ) ;
      }

/** \fn INT32 upsert ( const bson::BSONObj &rule,
                     const bson::BSONObj &condition = _sdbStaticObject,
                     const bson::BSONObj &hint      = _sdbStaticObject
                   )
    \brief Update the matching documents in current collection, insert if no matching
    \param [in] rule The updating rule
    \param [in] condition The matching rule, update all the documents if not provided
    \param [in] hint The hint, automatically match the optimal hint if not provided
    \retval SDB_OK Operation Success
    \retval Others Operation Fail
    \note It won't work to upsert the "ShardingKey" field, but the other fields take effect
*/
      INT32 upsert ( const bson::BSONObj &rule,
                     const bson::BSONObj &condition = _sdbStaticObject,
                     const bson::BSONObj &hint      = _sdbStaticObject
                   )
      {
         if ( !pCollection )
            return SDB_NOT_CONNECTED ;
         return pCollection->upsert ( rule, condition, hint ) ;
      }

/** \fn   INT32 del ( const bson::BSONObj &condition,
                  const bson::BSONObj &hint
                )
    \brief Delete the matching documents in current collection
    \param [in] condition The matching rule, delete all the documents if not provided
    \param [in] hint The hint, automatically match the optimal hint if not provided
    \retval SDB_OK Operation Success
    \retval Others Operation Fail
*/
      INT32 del ( const bson::BSONObj &condition = _sdbStaticObject,
                  const bson::BSONObj &hint      = _sdbStaticObject
                )
      {
         if ( !pCollection )
            return SDB_NOT_CONNECTED ;
         return pCollection->del ( condition, hint ) ;
      }

/* \fn INT32 query  ( _sdbCursor **cursor,
                     const bson::BSONObj &condition,
                     const bson::BSONObj &selected,
                     const bson::BSONObj &orderBy,
                     const bson::BSONObj &hint,
                     INT64 numToSkip,
                     INT64 numToReturn
                   )
    \brief Get the matching documents in current collection
    \param [in] condition The matching rule, return all the documents if not provided
    \param [in] selected The selective rule, return the whole document if not provided
    \param [in] orderBy The ordered rule, result set is unordered if not provided
    \param [in] hint The hint, automatically match the optimal hint if not provided
    \param [in] numToSkip Skip the first numToSkip documents, default is 0
    \param [in] numToReturn Only return numToReturn documents, default is -1 for returning all results
    \param [out] cursor The cursor of current query
    \retval SDB_OK Operation Success
    \retval Others Operation Fail
*/
      INT32 query  ( _sdbCursor **cursor,
                     const bson::BSONObj &condition = _sdbStaticObject,
                     const bson::BSONObj &selected  = _sdbStaticObject,
                     const bson::BSONObj &orderBy   = _sdbStaticObject,
                     const bson::BSONObj &hint      = _sdbStaticObject,
                     INT64 numToSkip          = 0,
                     INT64 numToReturn        = -1
                   )
      {
         if ( !pCollection )
            return SDB_NOT_CONNECTED ;
         return pCollection->query ( cursor, condition, selected, orderBy,
                                     hint, numToSkip, numToReturn ) ;
      }

/** \fn INT32 query  ( sdbCursor &cursor,
                     const bson::BSONObj &condition,
                     const bson::BSONObj &selected,
                     const bson::BSONObj &orderBy,
                     const bson::BSONObj &hint,
                     INT64 numToSkip,
                     INT64 numToReturn
                   )
    \brief Get the matching documents in current collection
    \param [in] condition The matching rule, return all the documents if not provided
    \param [in] selected The selective rule, return the whole document if not provided
    \param [in] orderBy The ordered rule, result set is unordered if not provided
    \param [in] hint The hint, automatically match the optimal hint if not provided
    \param [in] numToSkip Skip the first numToSkip documents, default is 0
    \param [in] numToReturn Only return numToReturn documents, default is -1 for returning all results
    \param [out] cursor The cursor of current query
    \retval SDB_OK Operation Success
    \retval Others Operation Fail
*/
      INT32 query  ( sdbCursor &cursor,
                     const bson::BSONObj &condition = _sdbStaticObject,
                     const bson::BSONObj &selected  = _sdbStaticObject,
                     const bson::BSONObj &orderBy   = _sdbStaticObject,
                     const bson::BSONObj &hint      = _sdbStaticObject,
                     INT64 numToSkip          = 0,
                     INT64 numToReturn        = -1
                   )
      {
         if ( !pCollection )
            return SDB_NOT_CONNECTED ;
         return pCollection->query ( cursor, condition, selected, orderBy,
                                     hint, numToSkip, numToReturn ) ;
      }

/* \fn INT32 rename ( const CHAR *pNewName )
    \brief Rename the specified collection
    \param [in] pNewName The new collection name
    \retval SDB_OK Operation Success
    \retval Others Operation Fail

      INT32 rename ( const CHAR *pNewName )
      {
         if ( !pCollection )
            return SDB_NOT_CONNECTED ;
         return pCollection->rename ( pNewName ) ;
      }*/

/** \fn INT32 createIndex ( bson::BSONObj &indexDef,
                          const CHAR *pName,
                          BOOLEAN isUnique
                        )
    \brief Create the index in current collection
    \param [in] indexDef The bson structure of index element, e.g. {name:1, age:-1}
    \param [in] pIndexName The index name
    \param [in] isUnique Whether the index elements are unique or not
    \param [in] isEnforced Whether the index is enforced unique
                           This element is meaningful when isUnique is set to true
    \retval SDB_OK Operation Success
    \retval Others Operation Fail
*/
      INT32 createIndex ( bson::BSONObj &indexDef,
                          const CHAR *pName,
                          BOOLEAN isUnique,
                          BOOLEAN isEnforced
                        )
      {
         if ( !pCollection )
            return SDB_NOT_CONNECTED ;
         return pCollection->createIndex ( indexDef, pName, isUnique,
                                           isEnforced ) ;
      }

/* \fn INT32 getIndexes ( _sdbCursor **cursor,
                         const CHAR *pName )
    \brief Get all of or one of the indexes in current collection
    \param [in] pName  The index name, returns all of the indexes if this parameter is null
    \param [out] cursor The cursor of all the result for current query
    \retval SDB_OK Operation Success
    \retval Others Operation Fail
*/
      INT32 getIndexes ( _sdbCursor **cursor,
                         const CHAR *pName )
      {
         if ( !pCollection )
            return SDB_NOT_CONNECTED ;
         return pCollection->getIndexes ( cursor, pName ) ;
      }

/** \fn INT32 getIndexes ( sdbCursor &cursor,
                         const CHAR *pName )
    \brief Get all of or one of the indexes in current collection
    \param [in] pName  The index name, returns all of the indexes if this parameter is null
    \param [out] cursor The cursor of all the result for current query
    \retval SDB_OK Operation Success
    \retval Others Operation Fail
*/
      INT32 getIndexes ( sdbCursor &cursor,
                         const CHAR *pName )
      {
         if ( !pCollection )
            return SDB_NOT_CONNECTED ;
         return pCollection->getIndexes ( cursor, pName ) ;
      }

/** \fn INT32 dropIndex ( const CHAR *pName )
    \brief Drop the index in current collection
    \param [in] pName The index name
    \retval SDB_OK Operation Success
    \retval Others Operation Fail
*/
      INT32 dropIndex ( const CHAR *pName )
      {
         if ( !pCollection )
            return SDB_NOT_CONNECTED ;
         return pCollection->dropIndex ( pName ) ;
      }

/** \fn INT32 create ()
    \brief create the specified collection of current collection space
    \deprecated This function will be deprecated in SequoiaDB1.6, use sdbCollectionSpace::createCollection instead of it.
    \retval SDB_OK Operation Success
    \retval Others Operation Fail
*/
      INT32 create ()
      {
         if ( !pCollection )
            return SDB_NOT_CONNECTED ;
         return pCollection->create () ;
      }

/** \fn INT32 drop ()
    \brief Drop the specified collection of current collection space
    \deprecated This function will be deprecated in SequoiaDB1.6, use sdbCollectionSpace::dropCollection instead of it.
    \retval SDB_OK Operation Success
    \retval Others Operation Fail
*/
      INT32 drop ()
      {
         if ( !pCollection )
            return SDB_NOT_CONNECTED ;
         return pCollection->drop () ;
      }


/** \fn const CHAR *getCollectionName ()
    \brief Get the name of specified collection in current collection space
    \return The name of specified collection.
*/
      const CHAR *getCollectionName ()
      {
         if ( !pCollection )
            return NULL ;
         return pCollection->getCollectionName () ;
      }

/** \fn const CHAR *getCSName ()
    \brief Get the name of current collection space
    \return The name of current collection space.
*/
      const CHAR *getCSName ()
      {
         if ( !pCollection )
            return NULL ;
         return pCollection->getCSName () ;
      }

/** \fn const CHAR *getFullName ()
    \brief Get the full name of specified collection in current collection space
    \return The full name of specified collection.
*/
      const CHAR *getFullName ()
      {
         if ( !pCollection )
            return NULL ;
         return pCollection->getFullName () ;
      }

/* \fn INT32 aggregate ( _sdbCursor **cursor,
                         std::vector<bson::BSONObj> &obj
                       )
    \brief Execute aggregate operation in specified collection
    \param [in] obj The array of bson objects
    \param [out] cursor The cursor handle of result
    \retval SDB_OK Operation Success
    \retval Others Operation Fail
*/
 INT32 aggregate ( _sdbCursor **cursor,
                   std::vector<bson::BSONObj> &obj
                 )
{
   if ( !pCollection )
      return SDB_NOT_CONNECTED ;
   return pCollection->aggregate ( cursor, obj ) ;
}

/** \fn INT32 aggregate ( sdbCursor &cursor,
                          std::vector<bson::BSONObj> &obj
                        )
    \brief Execute aggregate operation in specified collection
    \param [in] obj The array of bson objects
    \param [out] cursor The cursor object of result
    \retval SDB_OK Operation Success
    \retval Others Operation Fail
*/
 INT32 aggregate ( sdbCursor &cursor,
                   std::vector<bson::BSONObj> &obj
                 )
{
   if ( !pCollection )
      return SDB_NOT_CONNECTED ;
   return pCollection->aggregate ( cursor, obj ) ;
}

/* \fn  INT32 getQueryMeta ( _sdbCursor **cursor,
                             const bson::BSONObj &condition,
                             const bson::BSONObj &selected,
                             const bson::BSONObj &orderBy,
                             INT64 numToSkip,
                             INT64 numToReturn ) ;
    \brief Get the index blocks' or data blocks' infomation for concurrent query
    \param [in] condition The matching rule, return all the documents if not provided
    \param [in] orderBy The ordered rule, result set is unordered if not provided
    \param [in] hint One of the indexs of current collection, using default index to query if not provided
                    eg:{"":"ageIndex"}
    \param [in] numToSkip Skip the first numToSkip documents, default is 0
    \param [in] numToReturn Only return numToReturn documents, default is -1 for returning all results
    \param [out] cursor The cursor of current query
    \retval SDB_OK Operation Success
    \retval Others Operation Fail
*/
   INT32 getQueryMeta ( _sdbCursor **cursor,
                             const bson::BSONObj &condition,
                             const bson::BSONObj &orderBy,
                             const bson::BSONObj &hint,
                             INT64 numToSkip,
                             INT64 numToReturn ) ;

/** \fn  INT32 getQueryMeta ( sdbCursor &cursor,
                         const bson::BSONObj &condition,
                         const bson::BSONObj &selected,
                         const bson::BSONObj &orderBy,
                         INT64 numToSkip,
                         INT64 numToReturn )
    \brief Get the index blocks' or data blocks' infomations for concurrent query
    \param [in] condition The matching rule, return the whole range of index blocks if not provided
                    eg:{"age":{"$gt":25},"age":{"$lt":75}}
    \param [in] orderBy The ordered rule, result set is unordered if not provided
    \param [in] hint One of the indexs in current collection, using default index to query if not provided
                    eg:{"":"ageIndex"}
    \param [in] numToSkip Skip the first numToSkip documents, default is 0
    \param [in] numToReturn Only return numToReturn documents, default is -1 for returning all results
    \param [out] cursor The result of query
    \retval SDB_OK Operation Success
    \retval Others Operation Fail
*/
    INT32 getQueryMeta ( sdbCursor &cursor,
                         const bson::BSONObj &condition,
                         const bson::BSONObj &orderBy,
                         const bson::BSONObj &hint,
                         INT64 numToSkip,
                         INT64 numToReturn )
    {
       if ( !pCollection )
          return SDB_NOT_CONNECTED ;
       return pCollection->getQueryMeta ( cursor, condition, orderBy,
                                     hint, numToSkip, numToReturn ) ;
    }

/** \fn INT32 attachCollection ( const CHAR *subClFullName,
                                      const bson::BSONObj &options)
    \brief Attach the specified collection.
    \param [in] subClFullName The name of the subcollection
    \param [in] options The low boudary and up boudary
                eg: {"LowBound":{a:1},"UpBound":{a:100}}
    \retval SDB_OK Operation Success
    \retval Others Operation Fail
*/
    INT32 attachCollection ( const CHAR *subClFullName,
                                      const bson::BSONObj &options)
    {
       if ( !pCollection )
          return SDB_NOT_CONNECTED ;
       return pCollection->attachCollection ( subClFullName, options ) ;
    }

/** \fn INT32 detachCollection ( const CHAR *subClFullName)
    \brief Dettach the specified collection.
    \param [in] subClFullName The name of the subcollection
    \retval SDB_OK Operation Success
    \retval Others Operation Fail
*/
    INT32 detachCollection ( const CHAR *subClFullName)
    {
       if ( !pCollection )
          return SDB_NOT_CONNECTED ;
       return pCollection->detachCollection ( subClFullName ) ;
    }

   } ;

/** \enum sdbNodeStatus
    \breif The status of the node.
*/
   enum sdbNodeStatus
   {
      SDB_NODE_ALL = 0,
      SDB_NODE_ACTIVE,
      SDB_NODE_INACTIVE,
      SDB_NODE_UNKNOWN
   } ;

/** \typedef enum sdbNodeStatus sdbNodeStatus
    \breif The status of the node.
*/
   typedef enum sdbNodeStatus sdbNodeStatus ;

   class DLLEXPORT _sdbNode
   {
   private :
      _sdbNode ( const _sdbNode& other ) ;
      _sdbNode& operator=( const _sdbNode& ) ;
   public :
      _sdbNode () {}
      virtual ~_sdbNode () {}
      // connect to the current node
      virtual INT32 connect ( _sdb **dbConn ) = 0 ;
      virtual INT32 connect ( sdb &dbConn ) = 0 ;

      // get status of the current node
      virtual sdbNodeStatus getStatus () = 0 ;

      // get host name of the current node
      virtual const CHAR *getHostName () = 0 ;

      // get service name of the current node
      virtual const CHAR *getServiceName () = 0 ;

      // get node name of the current node
      virtual const CHAR *getNodeName () = 0 ;

      // stop the node
      virtual INT32 stop () = 0 ;

      // start the node
      virtual INT32 start () = 0 ;

      // modify config for the current node
/*      virtual INT32 modifyConfig ( std::map<std::string,std::string>
                                   &config ) = 0 ; */
   } ;

/** \class sdbNode
    \brief Database operation interfaces of node.This class takes the place of class "replicaNode".
    \note We use concept "node" instead of "replica node",
            and change the class name "ReplicaNode" to "Node".
            class "ReplicaNode" will be deprecated in version 2.x.
*/
   class DLLEXPORT sdbNode
   {
   private :
/** \fn sdbNode ( const sdbNode& other )
    \brief Copy Constructor
    \param[in] A const object reference  of class sdbNode.
*/
      sdbNode ( const sdbNode& other ) ;

/** \fn sdbNode& operator=( const sdbNode& )
    \brief Assignment constructor
    \param[in] A const reference  of class sdbNode.
    \retval A object const reference  of class sdbNode.
*/
      sdbNode& operator=( const sdbNode& ) ;
   public :
/** \var pNode
    \breif A pointer of virtual base class _sdbNode

    Class sdbNode is a shell for _sdbNode.We use pNode to
    call the methods in class _sdbNode.
*/
      _sdbNode *pNode ;

/** \fn sdbNode ()
    \brief Default constructor.
*/
      sdbNode ()
      {
         pNode = NULL ;
      }

/** \fn ~sdbNode ()
    \brief Destructor.
*/
      ~sdbNode ()
      {
         if ( pNode )
            delete pNode ;
      }
/* \fn connect ( _sdb **dbConn )
    \brief Connect to the current node.
    \param [out] dbConn The database obj of current connection
    \retval SDB_OK Operation Success
    \retval Others Operation Fail
*/
      INT32 connect ( _sdb **dbConn )
      {
         if ( !pNode )
            return SDB_NOT_CONNECTED ;
         return pNode->connect ( dbConn ) ;
      }

/** \fn connect ( sdb &dbConn )
    \brief Connect to the current node.
    \param [out] dbConn The database obj of current connection
    \retval SDB_OK Operation Success
    \retval Others Operation Fail
*/
      INT32 connect ( sdb &dbConn )
      {
         if ( !pNode )
            return SDB_NOT_CONNECTED ;
         return pNode->connect ( dbConn ) ;
      }

/** \fn sdbNodeStatus getStatus ()
    \brief Get status of the current node.
    \return  The status of current node.
*/
      sdbNodeStatus getStatus ()
      {
         if ( !pNode )
            return SDB_NODE_UNKNOWN ;
         return pNode->getStatus () ;
      }

/** \fn const CHAR *getHostName ()
    \brief Get host name of the current node.
    \return The host name.
*/
      const CHAR *getHostName ()
      {
         if ( !pNode )
            return NULL ;
         return pNode->getHostName () ;
      }

/** \fn CHAR *getServiceName ()
    \brief Get service name of the current node.
    \return The service name.
*/
      const CHAR *getServiceName ()
      {
         if ( !pNode )
            return NULL ;
         return pNode->getServiceName () ;
      }

/** \fn const CHAR *getNodeName ()
    \brief Get node name of the current node.
    \return The node name.
*/
      const CHAR *getNodeName ()
      {
         if ( !pNode )
            return NULL ;
         return pNode->getNodeName () ;
      }

/** \fn INT32  stop ()
    \brief Stop the node.
    \retval SDB_OK Operation Success
    \retval Others Operation Fail
*/
      INT32  stop ()
      {
         if ( !pNode )
            return SDB_NOT_CONNECTED ;
         return pNode->stop () ;
      }

/** \fn INT32 start ()
    \brief Start the node.
    \retval SDB_OK Operation Success
    \retval Others Operation Fail
*/
      INT32 start ()
      {
         if ( !pNode )
            return SDB_NOT_CONNECTED ;
         return pNode->start () ;
      }
/*      INT32 modifyConfig ( std::map<std::string,std::string> &config )
      {
         if ( !pNode )
            return NULL ;
         return pNode->modifyConfig ( config ) ;
      }*/
   } ;

   class DLLEXPORT _sdbShard
   {
   private :
      _sdbShard ( const _sdbShard& other ) ;
      _sdbShard& operator=( const _sdbShard& ) ;
   public :
      _sdbShard () {}
      virtual ~_sdbShard () {}
      // get number of logical nodes
      virtual INT32 getNodeNum ( sdbNodeStatus status, INT32 *num ) = 0 ;

      // get detailed information for the set
      virtual INT32 getDetail ( bson::BSONObj &result ) = 0 ;

      // get the master node
      virtual INT32 getMaster ( _sdbNode **node ) = 0 ;
      virtual INT32 getMaster ( sdbNode &node ) = 0 ;

      // get one of the slave node
      virtual INT32 getSlave ( _sdbNode **node ) = 0 ;
      virtual INT32 getSlave ( sdbNode &node ) = 0 ;

      // get a given node by name
      virtual INT32 getNode ( const CHAR *pNodeName,
                              _sdbNode **node ) = 0 ;
      virtual INT32 getNode ( const CHAR *pNodeName,
                              sdbNode &node ) = 0 ;

      // get a given node by host/service name
      virtual INT32 getNode ( const CHAR *pHostName,
                              const CHAR *pServiceName,
                              _sdbNode **node ) = 0 ;
      virtual INT32 getNode ( const CHAR *pHostName,
                              const CHAR *pServiceName,
                              sdbNode &node ) = 0 ;

      // create a new node in current shard
      virtual INT32 createNode ( const CHAR *pHostName,
                                 const CHAR *pServiceName,
                                 const CHAR *pDatabasePath,
                                 std::map<std::string,std::string> &config )= 0;
      // remove the specified node in current shard
      virtual INT32 removeNode ( const CHAR *pHostName,
                                 const CHAR *pServiceName,
                                 const bson::BSONObj &configure = _sdbStaticObject ) = 0 ;
      // stop the shard
      virtual INT32 stop () = 0 ;

      // start the shard
      virtual INT32 start () = 0 ;

      // get the shard name
      virtual const CHAR *getName () = 0 ;

      // whether the current shard is catalog shard or not
      virtual BOOLEAN isCatalog () = 0 ;
   } ;

/** \class sdbShard
    \brief Database operation interfaces of shard.This class takes the place of class "replicaGroup".
    \note We use concept "shard" instead of "replica group",
            and change the class name "ReplicaGroup" to "Shard".
            class "ReplicaGroup" will be deprecated in version 2.x.
*/
   class DLLEXPORT sdbShard
   {
   private :
/** \fn sdbShard ( const sdbShard& other )
    \brief Copy constructor
    \param[in] A const object reference of class sdbShard.
*/
      sdbShard ( const sdbShard& other ) ;

/** \fn sdbShard& operator=( const sdbShard& ) ;
   \brief Assignment constructor
   \param[in] A const reference object of class sdbShard.
   \retval A const reference object of class sdbShard.
*/
      sdbShard& operator=( const sdbShard& ) ;
   public :
/** \var pShard
      \breif A pointer of virtual base class _sdbShard

      Class sdbShard is a shell for _sdbShard.We use pCursor to
      call the methods in class _sdbShard.
*/
      _sdbShard *pShard ;

/** \fn sdbShard ()
    \brief Default constructor
*/
      sdbShard ()
      {
         pShard = NULL ;
      }

/** \fn ~sdbShard ()
    \brief Destructor
*/
      ~sdbShard ()
      {
         if ( pShard )
            delete pShard ;
      }

/** \fn INT32 getNodeNum ( sdbNodeStatus status, INT32 *num )
    \brief Get the count of node with given status in current shard.
    \param [in] status The specified status as below

        SDB_NODE_ALL
        SDB_NODE_ACTIVE
        SDB_NODE_INACTIVE
        SDB_NODE_UNKNOWN
    \param [out] num The count of node.
    \retval SDB_OK Operation Success
    \retval Others Operation Fail
*/
      INT32 getNodeNum ( sdbNodeStatus status, INT32 *num )
      {
         if ( !pShard )
            return SDB_NOT_CONNECTED ;
         return pShard->getNodeNum ( status, num ) ;
      }

/** \fn INT32 getDetail ( bson::BSONObj &result )
    \brief Get the detail of the shard.
    \param [out] result Return the all the info of current shard.
    \retval SDB_OK Operation Success
    \retval Others Operation Fail
*/
      INT32 getDetail ( bson::BSONObj &result )
      {
         if ( !pShard )
            return SDB_NOT_CONNECTED ;
         return pShard->getDetail ( result ) ;
      }

/* \fn INT32 getMaster ( _sdbNode **node )
    \brief Get the master node of the current shard.
    \param [out] node The master node.If not exit,return null.
    \retval SDB_OK Operation Success
    \retval Others Operation Fail
*/
      INT32 getMaster ( _sdbNode **node )
      {
         if ( !pShard )
            return SDB_NOT_CONNECTED ;
         return pShard->getMaster ( node ) ;
      }

/** \fn INT32 getMaster ( sdbNode &node )
    \brief Get the master node of the current shard.
    \param [out] node The master node.If not exit,return null.
    \retval SDB_OK Operation Success
    \retval Others Operation Fail
*/
      INT32 getMaster ( sdbNode &node )
      {
         if ( !pShard )
            return SDB_NOT_CONNECTED ;
         return pShard->getMaster ( node ) ;
      }

/* \fn INT32 getSlave ( _sdbNode **node )
    \brief Get one of slave node of the current shard,
           if no slave exists then get master.
    \param [out] node The slave node.
    \retval SDB_OK Operation Success
    \retval Others Operation Fail
*/
      INT32 getSlave ( _sdbNode **node )
      {
         if ( !pShard )
            return SDB_NOT_CONNECTED ;
         return pShard->getSlave ( node ) ;
      }

/** \fn  INT32 getSlave ( sdbNode &node )
    \brief Get one of slave node of the current shard,
           if no slave exists then get master
    \param [out] node The slave node.
    \retval SDB_OK Operation Success
    \retval Others Operation Fail
*/
      INT32 getSlave ( sdbNode &node )
      {
         if ( !pShard )
            return SDB_NOT_CONNECTED ;
         return pShard->getSlave ( node ) ;
      }

/* \fn INT32 getNode ( const CHAR *pNodeName,
                      _sdbNode **node )
    \brief Get specified node from current shard.
    \param [in] pHostName The host name of the node.
    \param [out] node  The specified node .
    \retval SDB_OK Operation Success
    \retval Others Operation Fail
*/
      INT32 getNode ( const CHAR *pNodeName,
                      _sdbNode **node )
      {
         if ( !pShard )
            return SDB_NOT_CONNECTED ;
         return pShard->getNode ( pNodeName, node ) ;
      }

/** \fn INT32 getNode ( const CHAR *pNodeName,
                      sdbNode &node )
    \brief Get specified node from current shard.
    \param [in] pHostName The host name of the node.
    \param [out] node  The specified node.
    \retval SDB_OK Operation Success
    \retval Others Operation Fail
*/
      INT32 getNode ( const CHAR *pNodeName,
                      sdbNode &node )
      {
         if ( !pShard )
            return SDB_NOT_CONNECTED ;
         return pShard->getNode ( pNodeName, node ) ;
      }

/* \fn INT32 getNode ( const CHAR *pHostName,
                      const CHAR *pServiceName,
                      _sdbNode **node )
    \brief Get specified node from current shard.
    \param [in] pHostName The host name of the node.
    \param [in] pServiceName The service name of the node.
    \param [out] node The specified node.
    \retval SDB_OK Operation Success
    \retval Others Operation Fail
*/
      INT32 getNode ( const CHAR *pHostName,
                      const CHAR *pServiceName,
                      _sdbNode **node )
      {
         if ( !pShard )
            return SDB_NOT_CONNECTED ;
         return pShard->getNode ( pHostName, pServiceName, node ) ;
      }

/** \fn INT32 getNode ( const CHAR *pHostName,
                      const CHAR *pServiceName,
                      sdbNode &node )
    \brief Get specified node from current shard.
    \param [in] pHostName The host name of the node.
    \param [in] pServiceName The service name of the node.
    \param [out] node The specified node.
    \retval SDB_OK Operation Success
    \retval Others Operation Fail
*/
      INT32 getNode ( const CHAR *pHostName,
                      const CHAR *pServiceName,
                      sdbNode &node )
      {
         if ( !pShard )
            return SDB_NOT_CONNECTED ;
         return pShard->getNode ( pHostName, pServiceName, node ) ;
      }

/** \fn INT32 createNode ( const CHAR *pHostName,
                         const CHAR *pServiceName,
                         const CHAR *pDatabasePath,
                         std::map<std::string,std::string> &config )
    \brief Create node in a given shard
    \param [in] pHostName The hostname for the node
    \param [in] pServiceName The servicename for the node
    \param [in] pDatabasePath The database path for the node
    \param [in] configure The configurations for the node
    \retval SDB_OK Operation Success
    \retval Others Operation Fail
*/
      INT32 createNode ( const CHAR *pHostName,
                         const CHAR *pServiceName,
                         const CHAR *pDatabasePath,
                         std::map<std::string,std::string> &config )
      {
         if ( !pShard )
            return SDB_NOT_CONNECTED ;
         return pShard->createNode ( pHostName, pServiceName,
                                            pDatabasePath, config ) ;
      }
/** \fn INT32 removeNode ( const CHAR *pHostName,
                                        const CHAR *pServiceName,
                                        const BSONObj &configure = _sdbStaticObject  )
    \brief remove node in a given shard
    \param [in] pHostName The hostname for the node
    \param [in] pServiceName The servicename for the node
    \param [in] configure The configurations for the node
    \retval SDB_OK Operation Success
    \retval Others Operation Fail
*/
      INT32 removeNode ( const CHAR *pHostName,
                                          const CHAR *pServiceName,
                                          const bson::BSONObj &configure = _sdbStaticObject )
         {
         if ( !pShard )
            return SDB_NOT_CONNECTED ;
         return pShard->removeNode ( pHostName, pServiceName,
                                           configure ) ;
   }
/** \fn INT32 stop ()
    \brief Stop current shard
    \retval SDB_OK Operation Success
    \retval Others Operation Fail
*/
      INT32 stop ()
      {
         if ( !pShard )
            return SDB_NOT_CONNECTED ;
         return pShard->stop () ;
      }

/** \fn INT32 INT32 start ()
    \brief Start up current shard.
    \retval SDB_OK Operation Success
    \retval Others Operation Fail
*/
      INT32 start ()
      {
         if ( !pShard )
            return SDB_NOT_CONNECTED ;
         return pShard->start () ;
      }
/** \fn BOOLEAN isCatalog ()
    \brief Test whether current shard is catalog.
    \retval TRUE The shard is catalog
    \retval FALSE The shard is not catalog
*/
      BOOLEAN isCatalog ()
      {
         if ( !pShard )
            return FALSE ;
         return pShard->isCatalog() ;
      }
   } ;

   class DLLEXPORT _sdbCollectionSpace
   {
   private :
      _sdbCollectionSpace ( const _sdbCollectionSpace& other ) ;
      _sdbCollectionSpace& operator=( const _sdbCollectionSpace& ) ;
   public :
      _sdbCollectionSpace () {}
      virtual ~_sdbCollectionSpace () {}
      // get a collection object
      virtual INT32 getCollection ( const CHAR *pCollectionName,
                                    _sdbCollection **collection ) = 0 ;

      virtual INT32 getCollection ( const CHAR *pCollectionName,
                                    sdbCollection &collection ) = 0 ;

      // create a new collection object with options
      virtual INT32 createCollection ( const CHAR *pCollection,
                                       const bson::BSONObj &options,
                                       _sdbCollection **collection ) = 0 ;

      virtual INT32 createCollection ( const CHAR *pCollection,
                                       const bson::BSONObj &options,
                                       sdbCollection &collection ) = 0 ;

      // create a new collection object
      virtual INT32 createCollection ( const CHAR *pCollection,
                                       _sdbCollection **collection ) = 0 ;

      virtual INT32 createCollection ( const CHAR *pCollection,
                                       sdbCollection &collection ) = 0 ;

      // drop an existing collection
      virtual INT32 dropCollection ( const CHAR *pCollection ) = 0 ;
      // create a collection space with current collection space name
      virtual INT32 create () = 0 ;
      // drop a collection space with current collection space name
      virtual INT32 drop () = 0 ;
      virtual const CHAR *getCSName () = 0 ;
   } ;
/** \class sdbCollectionSpace
    \brief Database operation interfaces of collection space
*/
   class DLLEXPORT sdbCollectionSpace
   {
   private :
/** \fn sdbCollectionSpace ( const sdbCollectionSpace& other )
    \brief Copy constructor.
    \param[in] A const object reference of class sdbCollectionSpace .
*/
      sdbCollectionSpace ( const sdbCollectionSpace& other ) ;

/** \fn sdbCollectionSpace& operator=( const sdbCollectionSpace& )
    \brief Assignment constructor.
    \param[in] A const object reference of class sdb.
    \retval A const object reference  of class sdb.
*/
      sdbCollectionSpace& operator=( const sdbCollectionSpace& ) ;
   public :
/** \var pCollectionSpace
      \breif A pointer of virtual base class _sdbCollectionSpace

      Class sdbCollectionSpace is a shell for _sdbCollectionSpace.We use pCursor to
      call the methods in class _sdbCollectionSpace.
*/
      _sdbCollectionSpace *pCollectionSpace ;

/** \fn sdbCollectionSpace( )
   \brief Default constructor.
*/
      sdbCollectionSpace ()
      {
         pCollectionSpace = NULL ;
      }

/** \fn ~sdbCollectionSpace ()
   \brief Destructor.
*/
      ~sdbCollectionSpace ()
      {
         if ( pCollectionSpace )
            delete pCollectionSpace ;
      }
/* \fn INT32 getCollection ( const CHAR *pCollectionName,
                            _sdbCollection **collection )
    \brief Get the named collection.
    \param [in] pCollectionName The full name of the collection.
    \param [out] collection The return collection handle.
    \retval SDB_OK Operation Success
    \retval Others Operation Fail
*/
      INT32 getCollection ( const CHAR *pCollectionName,
                            _sdbCollection **collection )
      {
         if ( !pCollectionSpace )
            return SDB_NOT_CONNECTED ;
         return pCollectionSpace->getCollection ( pCollectionName,
                                                  collection ) ;
      }

/** \fn INT32 getCollection ( const CHAR *pCollectionName,
                            sdbCollection &collection )
    \brief Get the named collection.
    \param [in] pCollectionName The full name of the collection.
    \param [out] collection The return collection object.
    \retval SDB_OK Operation Success
    \retval Others Operation Fail
*/
      INT32 getCollection ( const CHAR *pCollectionName,
                            sdbCollection &collection )
      {
         if ( !pCollectionSpace )
            return SDB_NOT_CONNECTED ;
         return pCollectionSpace->getCollection ( pCollectionName,
                                                  collection ) ;
      }

/* \fn INT32 createCollection ( const CHAR *pCollection,
 *                             const bson::BSONObj &options,
                               _sdbCollection **collection )
    \brief Create the specified collection in current collection space with options
    \param [in] pCollection The collection name
    \param [in] options The options for creating collection,
                including "ShardingKey", "ReplSize", "IsMainCL" and "Compressed" informations,
                no options, if null
    \param [out] collection The return collection handle.
    \retval SDB_OK Operation Success
    \retval Others Operation Fail
*/
      INT32 createCollection ( const CHAR *pCollection,
                               const bson::BSONObj &options,
                               _sdbCollection **collection )
      {
         if ( !pCollectionSpace )
            return SDB_NOT_CONNECTED ;
         return pCollectionSpace->createCollection ( pCollection,
                                                     options,
                                                     collection ) ;
      }

/** \fn INT32 createCollection ( const CHAR *pCollection,
 *                             const bson::BSONObj &options,
                               sdbCollection &collection )
    \brief Create the specified collection in current collection space with options
    \param [in] pCollection The collection name
    \param [in] options The options for creating collection,
                including "ShardingKey", "ReplSize", "IsMainCL" and "Compressed" informations,
                no options, if null
    \param [out] collection The return collection object .
    \retval SDB_OK Operation Success
    \retval Others Operation Fail
*/
      INT32 createCollection ( const CHAR *pCollection,
                               const bson::BSONObj &options,
                               sdbCollection &collection )
      {
         if ( !pCollectionSpace )
            return SDB_NOT_CONNECTED ;
         return pCollectionSpace->createCollection ( pCollection,
                                                     options,
                                                     collection ) ;
      }

/* \fn INT32 createCollection ( const CHAR *pCollection,
                               _sdbCollection **collection )
    \brief Create the specified collection in current collection space without
           sharding key and default ReplSize
    \param [in] pCollection The collection name
    \param [out] collection The return collection handle.
    \retval SDB_OK Operation Success
    \retval Others Operation Fail
*/
      INT32 createCollection ( const CHAR *pCollection,
                               _sdbCollection **collection )
      {
         if ( !pCollectionSpace )
            return SDB_NOT_CONNECTED ;
         return pCollectionSpace->createCollection ( pCollection,
                                                     collection ) ;
      }

/** \fn INT32 createCollection ( const CHAR *pCollection,
                               sdbCollection &collection )
    \brief Create the specified collection in current collection space without
           sharding key and default ReplSize.
    \param [in] pCollection The collection name.
    \param [out] collection The return collection object.
    \retval SDB_OK Operation Success
    \retval Others Operation Fail
*/
      INT32 createCollection ( const CHAR *pCollection,
                               sdbCollection &collection )
      {
         if ( !pCollectionSpace )
            return SDB_NOT_CONNECTED ;
         return pCollectionSpace->createCollection ( pCollection,
                                                     collection ) ;
      }

/** \fn INT32 dropCollection ( const CHAR *pCollection )
    \brief Drop the specified collection in current collection space.
    \param [in] pCollection  The collection name.
    \retval SDB_OK Operation Success
    \retval Others Operation Fail
*/
      INT32 dropCollection ( const CHAR *pCollection )
      {
         if ( !pCollectionSpace )
            return SDB_NOT_CONNECTED ;
         return pCollectionSpace->dropCollection ( pCollection ) ;
      }

/** \fn INT32 create ()
    \brief Create a new collection space.
    \deprecated This function will be deprecated in SequoiaDB1.6, use sdb::createCollectionSpace instead of it.
    \retval SDB_OK Operation Success.
    \retval Others Operation Fail
*/
      INT32 create ()
      {
         if ( !pCollectionSpace )
            return SDB_NOT_CONNECTED ;
         return pCollectionSpace->create () ;
      }

/** \fn INT32 drop ()
    \brief Drop current collection space.
    \deprecated This function will be deprecated in SequoiaDB1.6, use sdb::dropCollectionSpace instead of it.
    \retval SDB_OK Operation Success
    \retval Others Operation Fail
*/
      INT32 drop ()
      {
         if ( !pCollectionSpace )
            return SDB_NOT_CONNECTED ;
         return pCollectionSpace->drop () ;
      }


/** \fn const CHAR *getCSName ()
    \brief Get the current collection space name.
    \return The name of  current collection space.
*/
      const CHAR *getCSName ()
      {
         if ( !pCollectionSpace )
            return NULL ;
         return pCollectionSpace->getCSName () ;
      }
   } ;

   class DLLEXPORT _sdb
   {
   private :
      _sdb ( const _sdb& other ) ; // non construction-copyable
      _sdb& operator=( const _sdb& ) ; // non copyable
   public :
      _sdb () {}
      virtual ~_sdb () {}
      virtual INT32 connect ( const CHAR *pHostName,
                              UINT16 port
                            ) = 0 ;
      virtual INT32 connect ( const CHAR *pHostName,
                              UINT16 port,
                              const CHAR *pUsrName,
                              const CHAR *pPasswd ) = 0 ;
      virtual INT32 connect ( const CHAR *pHostName,
                              const CHAR *pServiceName
                            ) = 0 ;
      virtual INT32 connect ( const CHAR *pHostName,
                              const CHAR *pServiceName,
                              const CHAR *pUsrName,
                              const CHAR *pPasswd ) = 0 ;

      virtual void disconnect () = 0 ;

      virtual INT32 createUsr( const CHAR *pUsrName,
                               const CHAR *pPasswd ) = 0 ;

      virtual INT32 removeUsr( const CHAR *pUsrName,
                               const CHAR *pPasswd ) = 0 ;

      virtual INT32 getSnapshot ( _sdbCursor **cursor,
                                  INT32 snapType,
                                  const bson::BSONObj &condition = _sdbStaticObject,
                                  const bson::BSONObj &selector  = _sdbStaticObject,
                                  const bson::BSONObj &orderBy   = _sdbStaticObject
                                ) = 0 ;

      virtual INT32 getSnapshot ( sdbCursor &cursor,
                                  INT32 snapType,
                                  const bson::BSONObj &condition = _sdbStaticObject,
                                  const bson::BSONObj &selector  = _sdbStaticObject,
                                  const bson::BSONObj &orderBy   = _sdbStaticObject
                                ) = 0 ;

      virtual INT32 resetSnapshot ( const bson::BSONObj &condition = _sdbStaticObject ) = 0 ;

      virtual INT32 getList ( _sdbCursor **cursor,
                              INT32 listType,
                              const bson::BSONObj &condition = _sdbStaticObject,
                              const bson::BSONObj &selector  = _sdbStaticObject,
                              const bson::BSONObj &orderBy   = _sdbStaticObject
                            ) = 0 ;
      virtual INT32 getList ( sdbCursor &cursor,
                              INT32 listType,
                              const bson::BSONObj &condition = _sdbStaticObject,
                              const bson::BSONObj &selector  = _sdbStaticObject,
                              const bson::BSONObj &orderBy   = _sdbStaticObject
                            ) = 0 ;

      virtual INT32 getCollection ( const CHAR *pCollectionFullName,
                                    _sdbCollection **collection
                                  ) = 0 ;

      virtual INT32 getCollection ( const CHAR *pCollectionFullName,
                                    sdbCollection &collection
                                  ) = 0 ;

      virtual INT32 getCollectionSpace ( const CHAR *pCollectionSpaceName,
                                         _sdbCollectionSpace **cs
                                       ) = 0 ;

      virtual INT32 getCollectionSpace ( const CHAR *pCollectionSpaceName,
                                         sdbCollectionSpace &cs
                                       ) = 0 ;

      virtual INT32 createCollectionSpace ( const CHAR *pCollectionSpaceName,
                                            INT32 iPageSize,
                                            _sdbCollectionSpace **cs
                                          ) = 0 ;

      virtual INT32 createCollectionSpace ( const CHAR *pCollectionSpaceName,
                                            INT32 iPageSize,
                                            sdbCollectionSpace &cs
                                          ) = 0 ;

      virtual INT32 dropCollectionSpace ( const CHAR *pCollectionSpaceName )
            = 0 ;

      virtual INT32 listCollectionSpaces ( _sdbCursor **result ) = 0 ;

      virtual INT32 listCollectionSpaces ( sdbCursor &result ) = 0 ;

      // list all collections in a given database
      virtual INT32 listCollections ( _sdbCursor **result ) = 0 ;

      virtual INT32 listCollections ( sdbCursor &result ) = 0 ;

      // list all the shards in the given database
      virtual INT32 listShards ( _sdbCursor **result ) = 0 ;

      virtual INT32 listShards ( sdbCursor &result ) = 0 ;

      virtual INT32 getShard ( const CHAR *pName,
                                      _sdbShard **result ) = 0 ;

      virtual INT32 getShard ( const CHAR *pName,
                                      sdbShard &result ) = 0 ;

      virtual INT32 getShard ( INT32 id,
                                      _sdbShard **result ) = 0 ;

      virtual INT32 getShard ( INT32 id,
                                      sdbShard &result ) = 0 ;

      virtual INT32 createShard ( const CHAR *pName,
                                         _sdbShard **shard ) = 0 ;

      virtual INT32 createShard ( const CHAR *pName,
                                         sdbShard &shard ) = 0 ;

   virtual INT32 removeShard ( const CHAR *pName ) = 0 ;

      virtual INT32 createCataShard (  const CHAR *pHostName,
                                        const CHAR *pServiceName,
                                        const CHAR *pDatabasePath,
                                        const bson::BSONObj &configure ) =0 ;

      virtual INT32 activateShard ( const CHAR *pName,
                                           _sdbShard **shard ) = 0 ;

      virtual INT32 execUpdate( const CHAR *sql ) = 0 ;

      virtual INT32 exec( const CHAR *sql,
                          _sdbCursor **result ) = 0 ;

      virtual INT32 exec( const CHAR *sql,
                          sdbCursor &result ) = 0 ;

      virtual INT32 transactionBegin() = 0 ;

      virtual INT32 transactionCommit() = 0 ;

      virtual INT32 transactionRollback() = 0 ;

   virtual INT32 flushConfigure( const bson::BSONObj &options ) = 0 ;
   // stored procedure
      virtual INT32 crtJSProcedure ( const CHAR *code ) = 0 ;
      virtual INT32 rmProcedures( const CHAR *spName ) = 0 ;
      virtual INT32 listProcedures( _sdbCursor **cursor, const bson::BSONObj &condition ) = 0 ;
      virtual INT32 listProcedures( sdbCursor &cursor, const bson::BSONObj &condition ) = 0 ;
      virtual INT32 evalJS( _sdbCursor **cursor,
                             const CHAR *code,
                             SDB_SPD_RES_TYPE *type,
                             const bson::BSONObj &errmsg ) = 0 ;
      virtual INT32 evalJS( sdbCursor &cursor,
                             const CHAR *code,
                             SDB_SPD_RES_TYPE *type,
                             const bson::BSONObj &errmsg ) = 0 ;

   // bakup
      virtual INT32 backupOffline ( const bson::BSONObj &options) = 0 ;
      virtual INT32 listBackup ( _sdbCursor **cursor,
                              const bson::BSONObj &options,
                              const bson::BSONObj &condition = _sdbStaticObject,
                              const bson::BSONObj &selector = _sdbStaticObject,
                              const bson::BSONObj &orderBy = _sdbStaticObject) = 0 ;
      virtual INT32 listBackup ( sdbCursor &cursor,
                              const bson::BSONObj &options,
                              const bson::BSONObj &condition = _sdbStaticObject,
                              const bson::BSONObj &selector = _sdbStaticObject,
                              const bson::BSONObj &orderBy = _sdbStaticObject)  = 0 ;
      virtual INT32 removeBackup ( const bson::BSONObj &options ) = 0 ;

   // task
      virtual INT32 listTasks ( _sdbCursor **cursor,
                        const bson::BSONObj &condition = _sdbStaticObject,
                        const bson::BSONObj &selector = _sdbStaticObject,
                        const bson::BSONObj &orderBy = _sdbStaticObject,
                        const bson::BSONObj &hint = _sdbStaticObject) = 0 ;


      virtual INT32 listTasks ( sdbCursor &cursor,
                        const bson::BSONObj &condition = _sdbStaticObject,
                        const bson::BSONObj &selector = _sdbStaticObject,
                        const bson::BSONObj &orderBy = _sdbStaticObject,
                        const bson::BSONObj &hint = _sdbStaticObject) = 0 ;

      virtual INT32 waitTasks ( const SINT64 *taskIDs,
                        SINT32 num ) = 0 ;

      virtual INT32 cancelTask ( SINT64 taskID,
                        BOOLEAN isAsync ) = 0 ;

/*      virtual INT32 modifyConfig ( INT32 nodeID,
                       std::map<std::string,std::string> &config ) = 0 ;

      virtual INT32 getConfig ( INT32 nodeID,
                       std::map<std::string,std::string> &config ) = 0 ;

      virtual INT32 modifyConfig (
                       std::map<std::string,std::string> &config ) = 0 ;

      virtual INT32 getConfig (
                       std::map<std::string,std::string> &config ) = 0 ;
*/
      static _sdb *getObj () ;
   } ;
/** \typedef class _sdb _sdb
*/
   typedef class _sdb _sdb ;
/** \class sdb
    \brief Database operation interfaces of admin.
*/
   class DLLEXPORT sdb
   {
   private:
/** \fn sdb ( const sdb& other )
    \brief Copy constructor.
    \param[in] A const reference of class sdb.
*/
      sdb ( const sdb& other ) ;
/** \fn sdb& operator=( const sdb& )
    \brief Assignment constructor.
    \param[in] A const object reference of class sdb.
    \retval A const object reference of class sdb.
*/
      sdb& operator=( const sdb& ) ;
   public :
/** \var pSDB
    \breif A pointer of virtual base class _sdbCursor

    Class sdb is a shell for _sdb.We use pSDB to
    call the methods in class _sdb.
*/
      _sdb *pSDB ;

/** \fn sdb()
    \brief Default constructor.
*/
      sdb () :
      pSDB ( _sdb::getObj() )
      {
      }

/** \fn ~sdb()
    \brief Destructor.
*/
      ~sdb ()
      {
         if ( pSDB )
            delete pSDB ;
      }

/** \fn INT32 connect ( const CHAR *pHostName,
                      UINT16 port
                    )
    \brief Connect to remote Database Server.
    \param [in] pHostName The Host Name or IP Address of Database Server.
    \param [in] port The Port of Database Server.
    \retval SDB_OK Operation Success
    \retval Others Operation Fail
*/
      INT32 connect ( const CHAR *pHostName,
                      UINT16 port
                    )
      {
         if ( !pSDB )
            return SDB_SYS ;
         return pSDB->connect ( pHostName, port ) ;
      }

/** \fn INT32 connect ( const CHAR *pHostName,
                     UINT16 port,
                     const CHAR *pUsrName,
                     const CHAR *pPasswd
                     )
    \brief Connect to remote Database Server.
    \param [in] pHostName The Host Name or IP Address of Database Server.
    \param [in] port The Port of Database Server.
    \param [in] pUsrName The connection user name.
    \param [in] pPasswd The connection password.
    \retval SDB_OK Operation Success
    \retval Others Operation Fail
*/
     INT32 connect ( const CHAR *pHostName,
                     UINT16 port,
                     const CHAR *pUsrName,
                     const CHAR *pPasswd
                     )
     {
        if ( !pSDB )
           return SDB_SYS ;
        return pSDB->connect ( pHostName, port,
                               pUsrName, pPasswd ) ;
     }

/** \fn INT32 connect ( const CHAR *pHostName,
                      const CHAR *pServiceName
                    )
    \brief Connect to remote Database Server.
    \param [in] pHostName The Host Name or IP Address of Database Server.
    \param [in] pServiceName The Service Name of Database Server.
    \retval SDB_OK Operation Success
    \retval Others Operation Fail
*/
      INT32 connect ( const CHAR *pHostName,
                      const CHAR *pServiceName
                    )
      {
         if ( !pSDB )
            return SDB_SYS ;
         return pSDB->connect ( pHostName, pServiceName ) ;
      }

/** \fn INT32 connect ( const CHAR *pHostName,
                     const CHAR *pServiceName,
                     const CHAR *pUsrName,
                     const CHAR *pPasswd
                     )
    \brief Connect to remote Database Server.
    \param [in] pHostName The Host Name or IP Address of Database Server.
    \param [in] pServiceName The Service Name of Database Server.
    \param [in] pUsrName The connection user name.
    \param [in] pPasswd The connection password.
    \retval SDB_OK Operation Success
    \retval Others Operation Fail
*/
      INT32 connect ( const CHAR *pHostName,
                      const CHAR *pServiceName,
                      const CHAR *pUsrName,
                      const CHAR *pPasswd )
      {
         if ( !pSDB )
            return SDB_SYS ;
         return pSDB->connect ( pHostName, pServiceName,
                                 pUsrName, pPasswd ) ;
      }

/** \fn INT32 createUsr( const CHAR *pUsrName,
                       const CHAR *pPasswd )
    \brief Add an user in current database.
    \param [in] pUsrName The connection user name.
    \param [in] pPasswd The connection password.
    \retval SDB_OK Operation Success
    \retval Others Operation Fail
*/
      INT32 createUsr( const CHAR *pUsrName,
                       const CHAR *pPasswd )
      {
         if ( !pSDB )
            return SDB_SYS ;
         return pSDB->createUsr( pUsrName, pPasswd ) ;
      }

/** \fn INT32 removeUsr( const CHAR *pUsrName,
                           const CHAR *pPasswd )
    \brief Remove the spacified user from current database.
    \param [in] pUsrName The connection user name.
    \param [in] pPasswd The connection password.
    \retval SDB_OK Operation Success
    \retval Others Operation Fail
*/
      INT32 removeUsr( const CHAR *pUsrName,
                       const CHAR *pPasswd )
      {
         if ( !pSDB )
            return SDB_SYS ;
         return pSDB->removeUsr( pUsrName, pPasswd ) ;
      }

/** \fn void disconnect ()
    \brief Disconnect the remote Database Server.
*/
      void disconnect ()
      {
         if ( !pSDB )
            return ;
         pSDB->disconnect () ;
      }

/** \fn  INT32 getSnapshot ( sdbCursor &cursor,
                          INT32 snapType,
                          const bson::BSONObj &condition,
                          const bson::BSONObj &selector,
                          const bson::BSONObj &orderBy
                        )
    \brief Get the snapshots of specified type.
    \param [in] snapType The snapshot type as below

        SDB_SNAP_CONTEXTS         : Get all contexts' snapshot
        SDB_SNAP_CONTEXTS_CURRENT : Get the current context's snapshot
        SDB_SNAP_SESSIONS         : Get all sessions' snapshot
        SDB_SNAP_SESSIONS_CURRENT : Get the current session's snapshot
        SDB_SNAP_COLLECTIONS        : Get the collections' snapshot
        SDB_SNAP_COLLECTIONSPACES        : Get the collection spaces' snapshot
        SDB_SNAP_DATABASE         : Get database's snapshot
        SDB_SNAP_SYSTEM           : Get system's snapshot
        SDB_SNAP_CATA           : Get catalog's snapshot
       \param [in] condition The matching rule, match all the documents if not provided.
       \param [in] select The selective rule, return the whole document if not provided.
       \param [in] orderBy The ordered rule, result set is unordered if not provided.
       \param [out] cursor The return cursor object of query.
       \retval SDB_OK Operation Success
       \retval Others Operation Fail
*/
      INT32 getSnapshot ( sdbCursor &cursor,
                          INT32 snapType,
                          const bson::BSONObj &condition = _sdbStaticObject,
                          const bson::BSONObj &selector  = _sdbStaticObject,
                          const bson::BSONObj &orderBy   = _sdbStaticObject
                        )
      {
         if ( !pSDB )
            return SDB_SYS ;
         return pSDB->getSnapshot ( cursor, snapType, condition,
                                    selector, orderBy ) ;
      }


/* \fn  INT32 getSnapshot (_sdbCursor **cursor,
                          INT32 snapType,
                          const bson::BSONObj &condition,
                          const bson::BSONObj &selector,
                          const bson::BSONObj &orderBy
                        )
    \brief Get the snapshots of specified type.
    \param [in] snapType The snapshot type as below

        SDB_SNAP_CONTEXTS         : Get all contexts' snapshot
        SDB_SNAP_CONTEXTS_CURRENT : Get the current context's snapshot
        SDB_SNAP_SESSIONS         : Get all sessions' snapshot
        SDB_SNAP_SESSIONS_CURRENT : Get the current session's snapshot
        SDB_SNAP_COLLECTIONS        : Get the collections' snapshot
        SDB_SNAP_COLLECTIONSPACES        : Get the collection spaces' snapshot
        SDB_SNAP_DATABASE         : Get database snapshot
        SDB_SNAP_SYSTEM           : Get system snapshot
        SDB_SNAP_CATALOG           : Get catalog snapshot
     \param [in] condition The matching rule, match all the documents if not provided.
     \param [in] select The selective rule, return the whole document if not provided.
     \param [in] orderBy The ordered rule, result set is unordered if not provided.
     \param [out] cursor The return cursor handle of query.
     \retval SDB_OK Operation Success
     \retval Others Operation Fail
*/
      INT32 getSnapshot ( _sdbCursor **cursor,
                          INT32 snapType,
                          const bson::BSONObj &condition = _sdbStaticObject,
                          const bson::BSONObj &selector = _sdbStaticObject,
                          const bson::BSONObj &orderBy = _sdbStaticObject
                        )
      {
         if ( !pSDB )
            return SDB_SYS ;
         return pSDB->getSnapshot ( cursor, snapType, condition,
                                    selector, orderBy ) ;
      }

/** \fn INT32 resetSnapshot ( const bson::BSONObj &condition )
    \brief Reset the snapshot.
    \param [in] condition The matching rule, usually specifies the node in sharding environment,
                   in standalone mode, this option is ignored.
    \retval SDB_OK Operation Success
    \retval Others Operation Fail
*/
      INT32 resetSnapshot ( const bson::BSONObj &condition = _sdbStaticObject )
      {
         if ( !pSDB )
            return SDB_SYS ;
         return pSDB->resetSnapshot ( condition ) ;
      }

/* \fn INT32 getList ( _sdbCursor **cursor,
                      INT32 listType,
                      const bson::BSONObj &condition,
                      const bson::BSONObj &selector,
                      const bson::BSONObj &orderBy
                    )
    \brief Get the informations of specified type.
    \param [in] listType The list type as below

        SDB_LIST_CONTEXTS         : Get all contexts list
        SDB_LIST_CONTEXTS_CURRENT : Get contexts list for the current session
        SDB_LIST_SESSIONS         : Get all sessions list
        SDB_LIST_SESSIONS_CURRENT : Get the current session
        SDB_LIST_COLLECTIONS      : Get all collections list
        SDB_LIST_COLLECTIONSPACES : Get all collecion spaces' list
        SDB_LIST_STORAGEUNITS     : Get storage units list
        SDB_LIST_SHARDS           : Get shard list ( only applicable in sharding env )
        SDB_LIST_STOREPROCEDURES           : Get stored procedure list ( only applicable in sharding env )
   \param [in] condition The matching rule, match all the documents if null.
   \param [in] select The selective rule, return the whole document if null.
   \param [in] orderBy The ordered rule, never sort if null.
   \param [out] cursor The return cursor handle of query.
   \retval SDB_OK Operation Success
   \retval Others Operation Fail
*/
    INT32 getList ( _sdbCursor **cursor,
                    INT32 listType,
                    const bson::BSONObj &condition = _sdbStaticObject,
                    const bson::BSONObj &selector  = _sdbStaticObject,
                    const bson::BSONObj &orderBy   = _sdbStaticObject
                  )
      {
         if ( !pSDB )
            return SDB_SYS ;
         return pSDB->getList ( cursor,
                                listType,
                                condition,
                                selector,
                                orderBy ) ;
      }


/** \fn INT32 getList ( sdbCursor &cursor,
                      INT32 listType,
                      const bson::BSONObj &condition,
                      const bson::BSONObj &selector,
                      const bson::BSONObj &orderBy
                    )
    \brief Get the informations of specified type.
    \param [in] listType The list type as below

        SDB_LIST_CONTEXTS         : Get all contexts list
        SDB_LIST_CONTEXTS_CURRENT : Get contexts list for the current session
        SDB_LIST_SESSIONS         : Get all sessions list
        SDB_LIST_SESSIONS_CURRENT : Get the current session
        SDB_LIST_COLLECTIONS      : Get all collections list
        SDB_LIST_COLLECTIONSPACES : Get all collecion spaces' list
        SDB_LIST_STORAGEUNITS     : Get storage units list
        SDB_LIST_SHARDS           : Get shard list ( only applicable in sharding env )
   \param [in] condition The matching rule, match all the documents if null.
   \param [in] select The selective rule, return the whole document if null.
   \param [in] orderBy The ordered rule, never sort if null.
   \param [out] cursor The return cursor object of query.
   \retval SDB_OK Operation Success
   \retval Others Operation Fail
*/

      INT32 getList ( sdbCursor &cursor,
                      INT32 listType,
                      const bson::BSONObj &condition = _sdbStaticObject,
                      const bson::BSONObj &selector  = _sdbStaticObject,
                      const bson::BSONObj &orderBy   = _sdbStaticObject
                    )
      {
         if ( !pSDB )
            return SDB_SYS ;
         return pSDB->getList ( cursor,
                                listType,
                                condition,
                                selector,
                                orderBy ) ;
      }

/* \fn INT32 getCollection ( const CHAR *pCollectionFullName,
                            _sdbCollection **collection
                          )
    \biref Get the specified collection.
    \param [in] pCollectionFullName The full name of collection.
    \param [out] collection The return collection handle of query.
    \retval SDB_OK Operation Success
    \retval Others Operation Fail
*/
      INT32 getCollection ( const CHAR *pCollectionFullName,
                            _sdbCollection **collection
                          )
      {
         if ( !pSDB )
            return SDB_SYS ;
         return pSDB->getCollection ( pCollectionFullName,
                                      collection ) ;
      }

/** \fn INT32 getCollection ( const CHAR *pCollectionFullName,
                            sdbCollection &collection
                          )
    \biref Get the specified collection.
    \param [in] pCollectionFullName The full name of collection.
    \param [out] collection The return collection object of query.
    \retval SDB_OK Operation Success
    \retval Others Operation Fail
*/
      INT32 getCollection ( const CHAR *pCollectionFullName,
                            sdbCollection &collection
                          )
      {
         if ( !pSDB )
            return SDB_SYS ;
         return pSDB->getCollection ( pCollectionFullName,
                                      collection ) ;
      }

/* \fn INT32 getCollectionSpace ( const CHAR *pCollectionSpaceName,
                                 _sdbCollectionSpace **cs)
     \brief Get the specified collection space.
     \param [in] pCollectionSpaceName The name of collection space.
    \param [out] cs The return collection space handle of query.
     \retval SDB_OK Operation Success
     \retval Others Operation Fail
*/
      INT32 getCollectionSpace ( const CHAR *pCollectionSpaceName,
                                 _sdbCollectionSpace **cs
                               )
      {
         if ( !pSDB )
            return SDB_SYS ;
         return pSDB->getCollectionSpace ( pCollectionSpaceName,
                                           cs ) ;
      }


/** \fn INT32 getCollectionSpace ( const CHAR *pCollectionSpaceName,
                                  sdbCollectionSpace &cs)
     \brief Get the specified collection space.
     \param [in] pCollectionSpaceName The name of collection space.
     \param [out] cs The return collection space object of query.
     \retval SDB_OK Operation Success
     \retval Others Operation Fail
*/
      INT32 getCollectionSpace ( const CHAR *pCollectionSpaceName,
                                 sdbCollectionSpace &cs
                               )
      {
         if ( !pSDB )
            return SDB_SYS ;
         return pSDB->getCollectionSpace ( pCollectionSpaceName,
                                           cs ) ;
      }

/* \fn INT32 createCollectionSpace ( const CHAR *pCollectionSpaceName,
                                    INT32 iPageSize,
                                    _sdbCollectionSpace **cs
                                  )
    \brief Create the specified collection space with default SDB_PAGESIZE_4K.
    \param [in] pCollectionSpaceName The name of collection space.
    \param [in] iPageSize The Page Size as below

        SDB_PAGESIZE_4K
        SDB_PAGESIZE_8K
        SDB_PAGESIZE_16K
        SDB_PAGESIZE_32K
        SDB_PAGESIZE_64K
        SDB_PAGESIZE_DEFAULT
    \param [out] cs The return collection space handle of creation.
    \retval SDB_OK Operation Success
    \retval Others Operation Fail
*/
      INT32 createCollectionSpace ( const CHAR *pCollectionSpaceName,
                                    INT32 iPageSize,
                                    _sdbCollectionSpace **cs
                                  )
      {
         if ( !pSDB )
            return SDB_SYS ;
         return pSDB->createCollectionSpace ( pCollectionSpaceName,
                                              iPageSize,
                                              cs ) ;
      }


/** \fn       INT32 createCollectionSpace ( const CHAR *pCollectionSpaceName,
                                    INT32 iPageSize,
                                    sdbCollectionSpace &cs
                                  )
    \brief Create the specified collection space with default SDB_PAGESIZE_4K.
    \param [in] pCollectionSpaceName The name of collection space.
    \param [in] iPageSize The Page Size as below

        SDB_PAGESIZE_4K
        SDB_PAGESIZE_8K
        SDB_PAGESIZE_16K
        SDB_PAGESIZE_32K
        SDB_PAGESIZE_64K
        SDB_PAGESIZE_DEFAULT
    \param [out] cs The return collection space object of creation.
    \retval SDB_OK Operation Success
    \retval Others Operation Fail
*/
      INT32 createCollectionSpace ( const CHAR *pCollectionSpaceName,
                                    INT32 iPageSize,
                                    sdbCollectionSpace &cs
                                  )
      {
         if ( !pSDB )
            return SDB_SYS ;
         return pSDB->createCollectionSpace ( pCollectionSpaceName,
                                              iPageSize,
                                              cs ) ;
      }

/** \fn INT32 dropCollectionSpace ( const CHAR *pCollectionSpaceName )
    \brief Remove the specified collection space.
    \param [in] pCollectionSpaceName The name of collection space.
    \retval SDB_OK Operation Success
    \retval Others Operation Fail
*/
      INT32 dropCollectionSpace ( const CHAR *pCollectionSpaceName )
      {
         if ( !pSDB )
            return SDB_SYS ;
         return pSDB->dropCollectionSpace ( pCollectionSpaceName ) ;
      }

/* \fn INT32 listCollectionSpaces  ( _sdbCursor **result )
    \brief List all collection space of current database(include temporary collection space).
    \param [out] result The return cursor handle of query.
    \retval SDB_OK Operation Success
    \retval Others Operation Fail
*/
      INT32 listCollectionSpaces ( _sdbCursor **result )
      {
         if ( !pSDB )
            return SDB_SYS ;
         return pSDB->listCollectionSpaces ( result ) ;
      }

/** \fn INT32 listCollectionSpaces  ( sdbCursor &result )
    \brief List all collection space of current database(include temporary collection space).
    \param [out] result The return cursor object of query.
    \retval SDB_OK Operation Success
    \retval Others Operation Fail
*/
      INT32 listCollectionSpaces ( sdbCursor &result )
      {
         if ( !pSDB )
            return SDB_SYS ;
         return pSDB->listCollectionSpaces ( result ) ;
      }

/* \fn INT32 listCollections ( _sdbCursor **result )
    \brief list all collections in current database.
    \param [out] result The return cursor handle of query.
    \retval SDB_OK Operation Success
    \retval Others Operation Fail
*/
      INT32 listCollections ( _sdbCursor **result )
      {
         if ( !pSDB )
            return SDB_SYS ;
         return pSDB->listCollections ( result ) ;
      }

/** \fn  INT32 listCollections ( sdbCursor &result )
    \brief list all collections in current database.
    \param [out] result The return cursor object of query.
    \retval SDB_OK Operation Success
    \retval Others Operation Fail
*/
      INT32 listCollections ( sdbCursor &result )
      {
         if ( !pSDB )
            return SDB_SYS ;
         return pSDB->listCollections ( result ) ;
      }

/* \fn INT32 listShards ( _sdbCursor **result )
    \brief List all shards of current database.
    \param [out] result The return cursor handle of query.
    \retval SDB_OK Operation Success
    \retval Others Operation Fail
*/
      INT32 listShards ( _sdbCursor **result )
      {
         if ( !pSDB )
            return SDB_SYS ;
         return pSDB->listShards ( result ) ;
      }


/** \fn INT32 listShards ( sdbCursor &result )
    \brief List all shards of current database.
    \param [out] result The return cursor object of query.
    \retval SDB_OK Operation Success
    \retval Others Operation Fail
*/
      INT32 listShards ( sdbCursor &result )
      {
         if ( !pSDB )
            return SDB_SYS ;
         return pSDB->listShards ( result ) ;
      }

/* \fn INT32 getShard ( const CHAR *pName, _sdbShard **result )
    \brief Get the specified shard.
    \param [in] pName The name of shard.
    \param [out] result The return shard handle of query.
    \retval SDB_OK Operation Success
    \retval Others Operation Fail
*/
      INT32 getShard ( const CHAR *pName, _sdbShard **result )
      {
         if ( !pSDB )
            return SDB_SYS ;
         return pSDB->getShard ( pName, result ) ;
      }


/** \fn INT32 getShard ( const CHAR *pName, sdbShard &result )
    \brief Get the specified shard.
    \param [in] pName The name of shard.
    \param [out] result The return shard object of query.
    \retval SDB_OK Operation Success
    \retval Others Operation Fail
*/
      INT32 getShard ( const CHAR *pName, sdbShard &result )
      {
         if ( !pSDB )
            return SDB_SYS ;
         return pSDB->getShard ( pName, result ) ;
      }

/* \fn INT32 getShard ( INT32 id, _sdbShard **result )
    \brief Get the specified shard.
    \param [in] id The id of shard.
    \param [out] result The return shard object of query.
    \retval SDB_OK Operation Success
    \retval Others Operation Fail
*/
      INT32 getShard ( INT32 id, _sdbShard **result )
      {
         if ( !pSDB )
            return SDB_SYS ;
         return pSDB->getShard ( id, result ) ;
      }

/** \fn INT32 getShard ( INT32 id, sdbShard &result )
    \brief Get the specified shard.
    \param [in] id The id of shard.
    \param [out] result The return shard object of query.
    \retval SDB_OK Operation Success
    \retval Others Operation Fail
*/
      INT32 getShard ( INT32 id, sdbShard &result )
      {
         if ( !pSDB )
            return SDB_SYS ;
         return pSDB->getShard ( id, result ) ;
      }

/* \fn INT32 createShard ( const CHAR *pName, _sdbShard **shard )
    \brief Create the specified shard.
    \param [in] pName The name of the shard.
    \param [out] shard The return shard handle.
    \retval SDB_OK Operation Success
    \retval Others Operation Fail
*/
      INT32 createShard ( const CHAR *pName, _sdbShard **shard )
      {
         if ( !pSDB )
            return SDB_SYS ;
         return pSDB->createShard ( pName, shard ) ;
      }

/** \fn INT32 createShard ( const CHAR *pName, sdbShard &shard )
    \brief Create the specified shard.
    \param [in] pName The name of the shard.
    \param [out] shard The return shard object.
    \retval SDB_OK Operation Success
    \retval Others Operation Fail
*/
      INT32 createShard ( const CHAR *pName, sdbShard &shard )
      {
         if ( !pSDB )
            return SDB_SYS ;
         return pSDB->createShard ( pName, shard ) ;
      }

/** \fn INT32 removeShard ( const CHAR *pName )
    \brief Remove the specified shard
    \param [in] pName The name of the shard
    \retval SDB_OK Operation Success
    \retval Others Operation Fail
*/
      INT32 removeShard ( const CHAR *pName )
      {
         if ( !pSDB )
            return SDB_SYS ;
         return pSDB->removeShard ( pName ) ;
      }

/** \fn INT32 createCataShard (  const CHAR *pHostName,
                                        const CHAR *pServiceName,
                                        const CHAR *pDatabasePath,
                                        const bson::BSONObj &configure )
    \brief Create a catalog shard
    \param [in] pHostName The hostname for the catalog shard
    \param [in] pServiceName The servicename for the catalog shard
    \param [in] pDatabasePath The path for the catalog shard
    \param [in] configure The configurations for the catalog shard
    \retval SDB_OK Operation Success
    \retval Others Operation Fail
*/
      INT32 createCataShard (  const CHAR *pHostName,
                               const CHAR *pServiceName,
                               const CHAR *pDatabasePath,
                               const bson::BSONObj &configure )
      {
         if ( !pSDB )
            return SDB_SYS ;
         return pSDB->createCataShard ( pHostName, pServiceName,
                                        pDatabasePath, configure ) ;
      }

/** \fn INT32 activateShard ( const CHAR *pName, _sdbShard **shard )
    \brief Activate the specified shard
    \param [in] pName The name of the shard.
    \param [out] shard The return shard handle.
    \retval SDB_OK Operation Success
    \retval Others Operation Fail
*/
      INT32 activateShard ( const CHAR *pName, _sdbShard **shard )
      {
         if ( !pSDB )
            return SDB_SYS ;
         return pSDB->activateShard ( pName, shard ) ;
      }

/** \fn INT32 execUpdate( const CHAR *sql )
    \brief Executing SQL command for updating.
    \param [in] sql The SQL command.
    \retval SDB_OK Operation Success
    \retval Others Operation Fail
*/
      INT32 execUpdate( const CHAR *sql )
      {
         if ( !pSDB )
            return SDB_SYS ;
         return pSDB->execUpdate( sql ) ;
      }

/* \fn INT32 exec( const CHAR *sql,
                  _sdbCursor **result )
    \brief Executing SQL command.
    \param [in] sql The SQL command.
    \param [out] result The return cursor handle of matching documents.
    \retval SDB_OK Operation Success
    \retval Others Operation Fail
*/
      INT32 exec( const CHAR *sql,
                  _sdbCursor **result )
      {
         if ( !pSDB )
            return SDB_SYS ;
         return pSDB->exec( sql, result ) ;
      }

/** \fn INT32 exec( const CHAR *sql,
                 sdbCursor &result )
    \brief Executing SQL command.
    \param [in] sql The SQL command.
    \param [out] result The return cursor object of matching documents.
    \retval SDB_OK Operation Success
    \retval Others Operation Fail
*/
      INT32 exec( const CHAR *sql,
                  sdbCursor &result )
      {
         if ( !pSDB )
            return SDB_SYS ;
         return pSDB->exec( sql, result ) ;
      }

/** \fn INT32 transactionBegin()
    \brief Transaction commit.
    \retval SDB_OK Operation Success
    \retval Others Operation Fail
*/
      INT32 transactionBegin()
      {
         if ( !pSDB )
            return SDB_SYS ;
         return pSDB->transactionBegin() ;
      }

/** \fn INT32 transactionCommit()
    \brief Transaction commit.
    \retval SDB_OK Operation Success
    \retval Others Operation Fail
*/
      INT32 transactionCommit()
      {
         if ( !pSDB )
            return SDB_SYS ;
         return pSDB->transactionCommit() ;
      }

/** \fn INT32 transactionRollback()
    \brief Transaction rollback.
    \retval SDB_OK Operation Success
    \retval Others Operation Fail
*/
      INT32 transactionRollback()
      {
         if ( !pSDB )
            return SDB_SYS ;
         return pSDB->transactionRollback() ;
      }
/** \fn INT32 flushConfigure( BSONObj &options )
    \brief flush the options to configure file.
    \param [in] options The configure infomation, pass {"Global":true} or {"Global":false}
                    In cluster environment, passing {"Global":true} will flush data's and catalog's configuration file,
                    while passing {"Global":false} will flush coord's configuration file.
                    In stand-alone environment, both them have the same behaviour.
    \retval SDB_OK Operation Success
    \retval Others Operation Fail
*/
      INT32 flushConfigure( const bson::BSONObj &options )
      {
         if ( !pSDB )
            return SDB_SYS ;
         return pSDB->flushConfigure( options ) ;
      }

/** \fn INT32 crtJSProcedure ( const CHAR *code )
 *  \brief create a store procedures.
 *  \param [in] code The code of store procedures
 *  \retval SDB_OK Operation Success
 *  \retval Others  Operation Fail
*/
      INT32 crtJSProcedure ( const CHAR *code )
      {
         if ( !pSDB )
            return SDB_SYS ;
         return pSDB->crtJSProcedure( code ) ;
      }

/** \fn INT32 rmProcedures( const CHAR *spName )
 *  \brief remove a store procedures.
 *  \param [in] spName The name of store procedure
 *  \retval SDB_OK Operation Success
 *  \retval Others  Operation Fail
 */
      INT32 rmProcedures( const CHAR *spName )
      {
         if ( !pSDB )
            return SDB_SYS ;
         return pSDB->rmProcedures( spName ) ;
      }

      INT32 listProcedures( _sdbCursor **cursor, const bson::BSONObj &condition )
      {
         if ( !pSDB )
            return SDB_SYS ;
         return pSDB->listProcedures( cursor, condition ) ;
      }

/** \fn INT32 listProcedures( sdbCursor &cursor, const bson::BSONObj &condition )
 *  \brief list store procedures.
 *  \param [in] condition The condition of list
 *  \param [out] cursor The cursor of the result
 *  \retval SDB_OK Operation Success
 *  \retval Others  Operation Fail
 */
      INT32 listProcedures( sdbCursor &cursor, const bson::BSONObj &condition )
      {
         if ( !pSDB )
            return SDB_SYS ;
         return pSDB->listProcedures( cursor, condition ) ;
      }

     INT32 evalJS( _sdbCursor **cursor,
                             const CHAR *code,
                             SDB_SPD_RES_TYPE *type,
                             const bson::BSONObj &errmsg )
     {
         if ( !pSDB )
            return SDB_SYS ;
         return pSDB->evalJS( cursor, code, type, errmsg ) ;
     }

/** \fn INT32 evalJS( sdbCursor &cursor,
                             const CHAR *code,
                             SDB_SPD_RES_TYPE *type,
                             const bson::BSONObj &errmsg )
 * \brief eval a func.
 * \      type is declared in spd.h. see SDB_FMP_RES_TYPE.
 * \param [in] code The code to eval
 * \param [out] type The type of value
 * \param [out] cursor The cursor handle of current eval
 * \param [out] errmsg The errmsg from eval
 * \retval SDB_OK Operation Success
 * \retval Others  Operation Fail
 */
     INT32 evalJS( sdbCursor &cursor,
                             const CHAR *code,
                             SDB_SPD_RES_TYPE *type,
                             const bson::BSONObj &errmsg )
     {
         if ( !pSDB )
            return SDB_SYS ;
         return pSDB->evalJS( cursor, code, type, errmsg ) ;
     }

/** \fn INT32 backupOffline ( const bson::BSONObj &options)
    \brief Backup the whole database or specifed shard.
    \param [in] options Contains a series of backup configuration infomations. Backup the whole cluster if null. The "options" contains 5 options as below. All the elements in options are optional. eg: {"GroupName":["shardName1", "shardName2"], "Path":"/opt/sequoiadb/backup", "Name":"backupName", "Description":description, "EnsureInc":true, "OverWrite":true}

        GroupName   : The shards which to be backuped
        Path        : The backup path, if not assign, use the backup path assigned in configuration file
        Name        : The name for the backup
        Description : The description for the backup
        EnsureInc   : Whether excute increment synchronization, default to be false
        OverWrite   : Whether overwrite the old backup file, default to be false
    \retval SDB_OK Operation Success
    \retval Others Operation Fail
*/
      INT32 backupOffline ( const bson::BSONObj &options)
      {
         if ( !pSDB )
            return SDB_SYS ;
         return pSDB->backupOffline( options ) ;
      }

      INT32 listBackup ( _sdbCursor **cursor,
                              const bson::BSONObj &options,
                              const bson::BSONObj &condition = _sdbStaticObject,
                              const bson::BSONObj &selector = _sdbStaticObject,
                              const bson::BSONObj &orderBy = _sdbStaticObject)
      {
         if ( !pSDB )
            return SDB_SYS ;
         return pSDB->listBackup( cursor, options, condition, selector, orderBy ) ;
      }

/** \fn INT32 listBackup ( sdbCursor &cursor,
                              const bson::BSONObj &options,
                              const bson::BSONObj &condition = _sdbStaticObject,
                              const bson::BSONObj &selector = _sdbStaticObject,
                              const bson::BSONObj &orderBy = _sdbStaticObject);
    \brief List the backups.
    \param [in] options Contains configuration infomations for remove backups, list all the backups in the default backup path if null. The "options" contains 3 options as below. All the elements in options are optional. eg: {"GroupName":["shardName1", "shardName2"], "Path":"/opt/sequoiadb/backup", "Name":"backupName"}

        GroupName   : Assign the backups of specifed shards to be list
        Path        : Assign the backups in specifed path to be list, if not assign, use the backup path asigned in the configuration file
        Name        : Assign the backups with specifed name to be list
    \param [in] select The selective rule, return the whole document if null
    \param [in] orderBy The ordered rule, never sort if null
    \param [out] handle The cusor handle of result
    \retval SDB_OK Operation Success
    \retval Others Operation Fail
*/
      INT32 listBackup ( sdbCursor &cursor,
                              const bson::BSONObj &options,
                              const bson::BSONObj &condition = _sdbStaticObject,
                              const bson::BSONObj &selector = _sdbStaticObject,
                              const bson::BSONObj &orderBy = _sdbStaticObject)
      {
         if ( !pSDB )
            return SDB_SYS ;
         return pSDB->listBackup( cursor, options, condition, selector, orderBy ) ;
      }

/** \fn INT32 removeBackup ( const bson::BSONObj &options);
    \brief Remove the backups.
    \param [in] options Contains configuration infomations for remove backups, remove all the backups in the default backup path if null. The "options" contains 3 options as below. All the elements in options are optional. eg: {"GroupName":["shardName1", "shardName2"], "Path":"/opt/sequoiadb/backup", "Name":"backupName"}

        GroupName   : Assign the backups of specifed shards to be remove
        Path        : Assign the backups in specifed path to be remove, if not assign, use the backup path asigned in the configuration file
        Name        : Assign the backups with specifed name to be remove
    \retval SDB_OK Operation Success
    \retval Others Operation Fail
*/
      INT32 removeBackup ( const bson::BSONObj &options)
      {
         if ( !pSDB )
            return SDB_SYS ;
         return pSDB->removeBackup( options ) ;
      }

      INT32 listTasks ( _sdbCursor **cursor,
                        const bson::BSONObj &condition = _sdbStaticObject,
                        const bson::BSONObj &selector = _sdbStaticObject,
                        const bson::BSONObj &orderBy = _sdbStaticObject,
                        const bson::BSONObj &hint = _sdbStaticObject)
      {
         if ( !pSDB )
            return SDB_SYS ;
         return pSDB->listTasks ( cursor,
                                  condition,
                                  selector,
                                  orderBy,
                                  hint ) ;
      }
/** \fn INT32 listTasks ( sdbCursor &cursor,
                          const bson::BSONObj &condition = _sdbStaticObject,
                          const bson::BSONObj &selector = _sdbStaticObject,
                          const bson::BSONObj &orderBy = _sdbStaticObject,
                          const bson::BSONObj &hint = _sdbStaticObject) ;
    \brief List the tasks.
    \param [in] condition The matching rule, return all the documents if null
    \param [in] selector The selective rule, return the whole document if null
    \param [in] orderBy The ordered rule, never sort if null
    \param [in] hint The hint, automatically match the optimal hint if null
    \param [out] cursor The connection handle
    \retval SDB_OK Operation Success
    \retval Others Operation Fail
*/
      INT32 listTasks ( sdbCursor &cursor,
                        const bson::BSONObj &condition = _sdbStaticObject,
                        const bson::BSONObj &selector = _sdbStaticObject,
                        const bson::BSONObj &orderBy = _sdbStaticObject,
                        const bson::BSONObj &hint = _sdbStaticObject)
      {
         if ( !pSDB )
            return SDB_SYS ;
         return pSDB->listTasks ( cursor,
                                  condition,
                                  selector,
                                  orderBy,
                                  hint ) ;
      }

/** \fn INT32 waitTasks ( const SINT64 *taskIDs,
                             SINT32 num ) ;
    \brief Wait the tasks to finish.
    \param [in] taskIDs The array of task id
    \param [in] num The number of task id
    \retval SDB_OK Operation Success
    \retval Others Operation Fail
*/
      INT32 waitTasks ( const SINT64 *taskIDs,
                        SINT32 num )
      {
         if ( !pSDB )
            return SDB_SYS ;
         return pSDB->waitTasks ( taskIDs,
                                  num ) ;
      }

/** \fn INT32 cancelTask ( SINT64 taskID,
                           BOOLEAN isAsync ) ;
    \brief Cancel the specified task.
    \param [in] taskID The task id
    \param [in] isAsync The operation "cancel task" is async or not,
                "true" for async, "false" for sync. Default sync
    \retval SDB_OK Operation Success
    \retval Others Operation Fail
*/
      INT32 cancelTask ( SINT64 taskID,
                         BOOLEAN isAsync )
      {
         if ( !pSDB )
            return SDB_SYS ;
         return pSDB->cancelTask ( taskID,
                                   isAsync ) ;
      }



/*      INT32 modifyConfig ( INT32 nodeID,
                           std::map<std::string,std::string> &config )
      {
         if ( !pSDB )
            return SDB_SYS ;
         return pSDB->modifyConfig ( nodeID, config ) ;
      }

      INT32 getConfig ( INT32 nodeID,
                        std::map<std::string,std::string> &config )
      {
         if ( !pSDB )
            return SDB_SYS ;
         return pSDB->getConfig ( nodeID, config ) ;
      }

      INT32 modifyConfig ( std::map<std::string,std::string> &config )
      {
         if ( !pSDB )
            return SDB_SYS ;
         return pSDB->modifyConfig ( CURRENT_NODEID, config ) ;
      }

      INT32 getConfig ( std::map<std::string,std::string> &config )
      {
         if ( !pSDB )
            return SDB_SYS ;
         return pSDB->getConfig ( CURRENT_NODEID, config ) ;
      }*/

   } ;
/** \typedef class sdb sdb
      \brief Class sdb definition for sdb.
*/
   typedef class sdb sdb ;
}

#endif
