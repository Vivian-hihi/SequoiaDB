/*******************************************************************************


   Copyright (C) 2011-2014 SequoiaDB Ltd.

   This program is free software: you can redistribute it and/or modify
   it under the term of the GNU Affero General Public License, version 3,
   as published by the Free Software Foundation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warrenty of
   MARCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU Affero General Public License for more details.

   You should have received a copy of the GNU Affero General Public License
   along with this program. If not, see <http://www.gnu.org/license/>.

   Source File Name = optPlanPath.hpp

   Descriptive Name = Optimizer Plan Path Header

   When/how to use: this program may be used on binary and text-formatted
   versions of Optimizer component. This file contains structure for access
   plan, which is indicating how to run a given query.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          03/14/2017  HGM Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef OPTPLANPATH_HPP__
#define OPTPLANPATH_HPP__

#include "core.hpp"
#include "oss.hpp"
#include "mthMatchRuntime.hpp"
#include "optStatUnit.hpp"
#include "optCommon.hpp"
#include "utilAllocator.hpp"

using namespace std ;
using namespace bson ;

namespace engine
{

#define OPT_NODE_FIELD_NAME            "Operator"
#define OPT_NODE_FIELD_INPUT           "Input"
#define OPT_NODE_FIELD_OUTPUT          "Output"
#define OPT_NODE_FIELD_INDEX_FILTER    "IndexFilter"
#define OPT_NODE_FIELD_ESTIMATE        "Estimate"
#define OPT_NODE_FIELD_CHILDNODES      "ChildOperators"

#define OPT_NODE_FIELD_COLLECTION      FIELD_NAME_COLLECTION
#define OPT_NODE_FIELD_INDEX           FIELD_NAME_INDEX
#define OPT_NODE_FIELD_PAGES           "Pages"
#define OPT_NODE_FIELD_RECORDS         "Records"
#define OPT_NODE_FIELD_RECORD_SIZE     "RecordSize"
#define OPT_NODE_FIELD_SORTED          "Sorted"
#define OPT_NODE_FIELD_READ_RECORDS    "ReadRecords"
#define OPT_NODE_FIELD_OUTPUT_RECORDS  "OutputRecords"
#define OPT_NODE_FIELD_SORT_TYPE       "SortType"
#define OPT_NODE_FIELD_SORT_IN_MEM     "InMemory"
#define OPT_NODE_FIELD_SORT_EXTERNAL   "External"
#define OPT_NODE_FIELD_START_COST      "StartCost"
#define OPT_NODE_FIELD_RUN_COST        "RunCost"
#define OPT_NODE_FIELD_TOTAL_COST      "TotalCost"
#define OPT_NODE_FIELD_CL_STAT_EST     "CLEstFromStat"
#define OPT_NODE_FIELD_CL_STAT_TIME    "CLStatTime"
#define OPT_NODE_FIELD_IX_STAT_EST     "IXEstFromStat"
#define OPT_NODE_FIELD_IX_STAT_TIME    "IXStatTime"

#define OPT_NODE_ALLOCATOR_SIZE        1024

   enum OPT_PLAN_NODE_TYPE
   {
      OPT_PLAN_TB_SCAN,
      OPT_PLAN_IDX_SCAN,
      OPT_PLAN_SORT,
      OPT_PLAN_RETURN
   } ;

   enum optScanType
   {
      TBSCAN = 0,
      IXSCAN,
      UNKNOWNSCAN
   } ;

   enum OPT_PLAN_PATH_PRIORITY
   {
      // Must use index
      OPT_PLAN_IDX_REQUIRED,

      // Must use sorted index
      OPT_PLAN_SORTED_IDX_REQUIRED,

      // Index scan is preferred when matched fields or matched orders
      // otherwise, goto table scan
      OPT_PLAN_IDX_PREFERRED,

      // Default
      OPT_PLAN_DEFAULT_PRIORITY
   } ;

   typedef _utilAllocator<OPT_NODE_ALLOCATOR_SIZE> optPlanAllocator ;

   /*
      _optPlanNode define
    */
   class _optPlanNode : public SDBObject
   {
      public :
         _optPlanNode () ;

         virtual ~_optPlanNode () {}

         OSS_INLINE void setLNode ( _optPlanNode *pLNode )
         {
            _pLNode = pLNode ;
         }

         OSS_INLINE void setRNode ( _optPlanNode *pRNode )
         {
            _pRNode = pRNode ;
         }

         OSS_INLINE _optPlanNode *getLNode () const
         {
            return _pLNode ;
         }

         OSS_INLINE _optPlanNode *getRNode () const
         {
            return _pRNode ;
         }

         OSS_INLINE UINT64 getStartCost () const
         {
            return _startCost ;
         }

         OSS_INLINE UINT64 getRunCost () const
         {
            return _runCost ;
         }

         OSS_INLINE UINT64 getTotalCost () const
         {
            return _totalCost ;
         }

         OSS_INLINE UINT64 getOutputRecords () const
         {
            return _outputRecords ;
         }

         OSS_INLINE UINT32 getOutputRecordSize () const
         {
            return _outputRecordSize ;
         }

         OSS_INLINE UINT32 getOutputNumFields () const
         {
            return _outputNumFields ;
         }

         OSS_INLINE BOOLEAN isSorted () const
         {
            return _sorted ;
         }

         virtual void toBSON ( BSONObjBuilder &builder ) const ;

         virtual OPT_PLAN_NODE_TYPE getType () const = 0 ;

         virtual const CHAR * getName () const = 0 ;

         OSS_INLINE void *operator new ( size_t size,
                                         optPlanAllocator *pAllocator,
                                         std::nothrow_t )
         {
            void *p = NULL ;
            if ( size > 0 )
            {
               if ( pAllocator )
               {
                  p = pAllocator->allocate( size ) ;
               }

               if ( NULL == p )
               {
                  p = SDB_OSS_MALLOC( size ) ;
               }
            }

            return p ;
         }

         OSS_INLINE void operator delete ( void *p )
         {
            SDB_OSS_FREE( p ) ;
         }

         // Overload delete operator to keep compiler quiet
         OSS_INLINE void operator delete ( void *p,
                                           optPlanAllocator *pAllocator,
                                           std::nothrow_t )
         {
            if ( pAllocator && pAllocator->isAllocatedByme( p ) )
            {
               // Do nothing
            }
            else
            {
               SDB_OSS_FREE( p ) ;
            }
         }

         virtual void release ( optPlanAllocator *pAllocator ) = 0 ;

      protected :
         virtual void _toBSON ( BSONObjBuilder &builder ) const = 0 ;
         void _toOutputBSON ( BSONObjBuilder &builder ) const ;
         void _toEstimateBSON ( BSONObjBuilder &builder ) const ;

      protected :
         _optPlanNode *    _pLNode ;
         _optPlanNode *    _pRNode ;

         // Start cost of this node
         UINT64            _startCost ;

         // Run cost of this node
         UINT64            _runCost ;

         // Total cost of this node
         UINT64            _totalCost ;

         // Number of records will be output
         UINT64            _outputRecords ;

         // Average size of output records
         UINT32            _outputRecordSize ;

         // Average number of fields in output records
         UINT32            _outputNumFields ;

         // If output is sorted by required
         BOOLEAN           _sorted ;
   } ;

   typedef class _optPlanNode optPlanNode ;

   /*
      _optScanNode define
    */
   class _optScanNode : public _optPlanNode
   {
      public :
         _optScanNode ( const CHAR *pCollection, INT32 estCacheSize ) ;

         virtual ~_optScanNode () {}

         OSS_INLINE double getMthSelctivity () const
         {
            return _mthSelectivity ;
         }

         OSS_INLINE UINT32 getMthCPUCost () const
         {
            return _mthCPUCost ;
         }

         OSS_INLINE UINT32 getPageSize () const
         {
            return _pageSize ;
         }

         OSS_INLINE BOOLEAN isCandidate () const
         {
            return _isCandidate ;
         }

         OSS_INLINE BOOLEAN isSorted () const
         {
            return _sorted ;
         }

         virtual void evaluate () = 0 ;

         virtual optScanType getScanType () const = 0 ;

      protected :
         void _preEvaluate ( const BSONObj &selector,
                             mthMatchHelper &matchHelper,
                             optCollectionStat *collectionStat ) ;

         void _evalOutRecordSize () ;

         void _toInputBSON( BSONObjBuilder &builder ) const ;

      protected :
         const CHAR *      _pCollection ;

         // Selectivity of matcher
         double            _mthSelectivity ;

         // CPU cost of matcher
         UINT32            _mthCPUCost ;

         // Number of records in the collection
         UINT64            _inputRecords ;

         // Number of pages in the collection
         UINT32            _inputPages ;

         // Average number of fields in records
         UINT32            _inputNumFields ;

         // Average size of records
         UINT32            _inputRecordSize ;

         // Page size
         UINT32            _pageSize ;

         // Estimate cache size ( from --optestcachesize )
         INT32             _estCacheSize ;

         BOOLEAN           _isCandidate ;

         // If the estimation is based on statistics
         BOOLEAN           _clFromStat ;
         UINT64            _clStatTime ;
   } ;

   typedef class _optScanNode optScanNode ;

   /*
      _optTbScanNode define
    */
   class _optTbScanNode : public _optScanNode
   {
      public :
         _optTbScanNode ( const CHAR *pCollection, INT32 estCacheSize ) ;
         virtual ~_optTbScanNode () {}

         void preEvaluate ( const BSONObj &selector,
                            mthMatchHelper &matchHelper,
                            optCollectionStat *collectionStat ) ;

         virtual void evaluate () ;

         OSS_INLINE virtual OPT_PLAN_NODE_TYPE getType () const
         {
            return OPT_PLAN_TB_SCAN ;
         }

         OSS_INLINE virtual const CHAR * getName () const
         {
            return "TBSCAN" ;
         }

         OSS_INLINE virtual optScanType getScanType () const
         {
            return TBSCAN ;
         }

         OSS_INLINE virtual void release ( optPlanAllocator *pAllocator )
         {
            if ( pAllocator && pAllocator->isAllocatedByme( this ) )
            {
               this->~_optTbScanNode() ;
            }
            else
            {
               SDB_OSS_DEL this ;
            }
         }

      protected :
         virtual void _toBSON ( BSONObjBuilder &builder ) const ;
   } ;

   typedef class _optTbScanNode optTbScanNode ;

   /*
      _optIxScanNode define
    */
   class _optIxScanNode : public _optScanNode
   {
      public :
         _optIxScanNode ( const CHAR *pCollection, const ixmIndexCB &indexCB,
                          INT32 estCacheSize ) ;

         virtual ~_optIxScanNode () {}

         void preEvaluate ( const BSONObj &selector,
                            mthMatchHelper &matchHelper,
                            const BSONObj &boOrder,
                            OPT_PLAN_PATH_PRIORITY priority,
                            optCollectionStat *collectionStat,
                            optIndexStat *indexStat ) ;

         virtual void evaluate () ;

         OSS_INLINE virtual OPT_PLAN_NODE_TYPE getType () const
         {
            return OPT_PLAN_IDX_SCAN ;
         }

         OSS_INLINE virtual const CHAR * getName () const
         {
            return "IXSCAN" ;
         }

         OSS_INLINE const CHAR *getIndexName () const
         {
            return _pIndexName ;
         }

         OSS_INLINE INT32 getDirection () const
         {
            return _direction ;
         }

         OSS_INLINE BOOLEAN isMatchAll () const
         {
            return _matchAll ;
         }

         OSS_INLINE dmsExtentID getIndexExtID () const
         {
            return _indexExtID ;
         }

         OSS_INLINE dmsExtentID getIndexLID () const
         {
            return _indexLID ;
         }

         OSS_INLINE BSONObj getKeyPattern () const
         {
            return _keyPattern ;
         }

         OSS_INLINE virtual optScanType getScanType () const
         {
            return IXSCAN ;
         }

         OSS_INLINE virtual void release ( optPlanAllocator *pAllocator )
         {
            if ( pAllocator && pAllocator->isAllocatedByme( this ) )
            {
               this->~_optIxScanNode() ;
            }
            else
            {
               SDB_OSS_DEL this ;
            }
         }

         OSS_INLINE double getScanSelectivity () const
         {
            return _scanSelectivity ;
         }

         OSS_INLINE double getPredSelectivity () const
         {
            return _predSelectivity ;
         }

      protected :
         void _evalPredEstimation ( mthMatchHelper &matchHelper,
                                    const BSONObj &boOrder,
                                    BOOLEAN isBestIndex,
                                    const optIndexStat *indexStat ) ;

         virtual void _toBSON ( BSONObjBuilder &builder ) const ;

         void _toIndexBSON ( BSONObjBuilder &builder ) const ;

      protected :
         CHAR              _pIndexName [ IXM_INDEX_NAME_SIZE + 1 ] ;

         // Scan direction of index
         INT32             _direction ;

         // Operators in matchers are covered by predicates
         BOOLEAN           _matchAll ;

         // Need evaluate matchers after scan
         BOOLEAN           _needMatch ;

         // Number of matched fields in index
         UINT32            _matchedFields ;

         // Number of matched order by fields in index
         UINT32            _matchedOrders ;

         // Information of index
         dmsExtentID       _indexExtID ;
         dmsExtentID       _indexLID ;
         BSONObj           _keyPattern ;

         // The range to scan the index: from the start key to the stop key
         // of each key-pairs in the predicates
         double            _scanSelectivity ;

         // The selectivity of output from index with each start and stop
         // key-pairs in predicates
         double            _predSelectivity ;

         // CPU cost to evaluate predicates
         UINT32            _predCPUCost ;

         // Number of index pages
         UINT32            _indexPages ;

         // Number of index levels
         UINT32            _indexLevels ;

         // Number of records could be read in index
         // ( based on _scanSelectivity )
         UINT64            _idxReadRecords ;

         // Number of records will be output from index
         // ( based on _predSelectivity )
         UINT64            _idxOutRecords ;

         // If the estimation is based on statistics
         BOOLEAN           _ixFromStat ;
         UINT64            _ixStatTime ;
   } ;

   typedef class _optIxScanNode optIxScanNode ;

   /*
      _optSortNode define
    */
   class _optSortNode : public _optPlanNode
   {
      public :
         _optSortNode () ;

         virtual ~_optSortNode () {}

         void evaluate ( const BSONObj &boOrder,
                         UINT64 sortBufferSize ) ;

         OSS_INLINE virtual OPT_PLAN_NODE_TYPE getType () const
         {
            return OPT_PLAN_SORT ;
         }

         OSS_INLINE virtual const CHAR * getName () const
         {
            return "SORT" ;
         }

         OSS_INLINE virtual void release ( optPlanAllocator *pAllocator )
         {
            if ( pAllocator && pAllocator->isAllocatedByme( this ) )
            {
               this->~_optSortNode() ;
            }
            else
            {
               SDB_OSS_DEL this ;
            }
         }

      protected :

         enum OPT_PLAN_SORT_TYPE
         {
            OPT_PLAN_IN_MEM_SORT,
            OPT_PLAN_EXT_SORT
         } ;

         virtual void _toBSON ( BSONObjBuilder &builder ) const ;

      protected :
         OPT_PLAN_SORT_TYPE   _sortType ;
         UINT64               _sortBufferSize ;
         UINT32               _numOrders ;
   } ;

   typedef class _optSortNode optSortNode ;

   /*
      _optReturnNode define
    */
   class _optReturnNode : public _optPlanNode
   {
      public :
         _optReturnNode () ;

         virtual ~_optReturnNode () {}

         void evaluate ( SINT64 numToSkip,
                         SINT64 numToReturn ) ;

         OSS_INLINE virtual OPT_PLAN_NODE_TYPE getType () const
         {
            return OPT_PLAN_RETURN ;
         }

         OSS_INLINE virtual const CHAR * getName () const
         {
            return "RETURN" ;
         }

         OSS_INLINE virtual void release ( optPlanAllocator *pAllocator )
         {
            if ( pAllocator && pAllocator->isAllocatedByme( this ) )
            {
               this->~_optReturnNode() ;
            }
            else
            {
               SDB_OSS_DEL this ;
            }
         }

      protected :
         virtual void _toBSON ( BSONObjBuilder &builder ) const ;

      protected :
         SINT64            _numToSkip ;
         SINT64            _numToReturn ;
   } ;

   typedef class _optReturnNode optReturnNode ;

   /*
      _optPlanPath define
    */
   class _optPlanPath ;
   typedef class _optPlanPath optPlanPath ;

   class _optPlanPath : public SDBObject
   {
      public :
         _optPlanPath ( optPlanAllocator *pAllocator ) ;

         virtual ~_optPlanPath () ;

         OSS_INLINE BOOLEAN isEmpty () const
         {
            return _pRootNode == NULL ;
         }

         OSS_INLINE UINT64 getTotalCost () const
         {
            return _pRootNode ? _pRootNode->getTotalCost() : OSS_UINT64_MAX ;
         }

         OSS_INLINE virtual void toBSON ( BSONObjBuilder &builder ) const
         {
            if ( _pRootNode )
            {
               _pRootNode->toBSON( builder ) ;
            }
         }

         OSS_INLINE virtual string toString () const
         {
            BSONObjBuilder builder ;
            toBSON( builder ) ;
            return builder.obj().toString( FALSE, TRUE ) ;
         }

         OSS_INLINE virtual void clearPath ()
         {
            _deleteNodes () ;
         }

      protected :
         void _deleteNodes () ;

         void _deleteChildNodes ( optPlanNode *pNode ) ;

         void _setRootNode ( optPlanNode *pNode ) ;

         void _joinPath ( optPlanNode *pJoinNode, optPlanPath &path ) ;

         void _swap ( optPlanPath &path ) ;

         INT32 _createSortNode ( const BSONObj &boOrder,
                                 UINT64 sortBufferSize ) ;

         INT32 _createReturnNode ( SINT64 numToSkip,
                                   SINT64 numToReturn ) ;

      protected :
         optPlanAllocator * _pAllocator ;
         optPlanNode *      _pRootNode ;
   } ;

   /*
      _optScanPath define
    */
   class _optScanPath ;
   typedef class _optScanPath optScanPath ;

   class _optScanPath : public _optPlanPath
   {
      public :
         _optScanPath ( optPlanAllocator *pAllocator ) ;

         virtual ~_optScanPath () {}

         INT32 createTbScan ( const CHAR *pCollection,
                              const BSONObj &selector,
                              mthMatchHelper &matchHelper,
                              INT32 estCacheSize,
                              optCollectionStat *collectionStat ) ;

         INT32 createIxScan ( const CHAR *pCollection,
                              const ixmIndexCB &indexCB,
                              const BSONObj &selector,
                              mthMatchHelper &matchHelper,
                              const BSONObj &boOrder,
                              OPT_PLAN_PATH_PRIORITY priority,
                              INT32 estCacheSize,
                              optCollectionStat *collectionStat,
                              optIndexStat *indexStat ) ;

         OSS_INLINE BOOLEAN isCandidate () const
         {
            return _pScanNode && _pScanNode->isCandidate() ;
         }

         OSS_INLINE BOOLEAN isMatchAll () const
         {
            return isIxScan() ?
                   ((optIxScanNode *)_pScanNode)->isMatchAll() :
                   FALSE ;
         }

         OSS_INLINE INT32 getDirection () const
         {
            return isIxScan() ?
                   ((optIxScanNode *)_pScanNode)->getDirection() :
                   1 ;
         }

         OSS_INLINE dmsExtentID getIndexExtID () const
         {
            return isIxScan() ?
                   ((optIxScanNode *)_pScanNode)->getIndexExtID() :
                   DMS_INVALID_EXTENT ;
         }

         OSS_INLINE dmsExtentID getIndexLID () const
         {
            return isIxScan() ?
                   ((optIxScanNode *)_pScanNode)->getIndexLID() :
                   DMS_INVALID_EXTENT ;
         }

         OSS_INLINE BSONObj getKeyPattern () const
         {
            return isIxScan() ?
                   ((optIxScanNode *)_pScanNode)->getKeyPattern() :
                   BSONObj() ;
         }

         OSS_INLINE double getSelectivity () const
         {
            return isIxScan() ?
                   ((optIxScanNode *)_pScanNode)->getScanSelectivity() :
                   ( _pScanNode ? _pScanNode->getMthSelctivity() :
                                  OPT_PRED_DEFAULT_SELECTIVITY ) ;
         }

         OSS_INLINE BOOLEAN isSortRequired () const
         {
            return _sortRequired ;
         }

         OSS_INLINE const CHAR *getIndexName () const
         {
            return isIxScan() ?
                   ((optIxScanNode *)_pScanNode)->getIndexName() :
                   "" ;
         }

         OSS_INLINE optScanType getScanType () const
         {
            return _pScanNode ? _pScanNode->getScanType() : UNKNOWNSCAN ;
         }

         INT32 evaluate ( const BSONObj &boOrder,
                          SINT64 numToSkip,
                          SINT64 numToReturn,
                          UINT64 sortBufferSize ) ;

         void swap ( optScanPath &path ) ;

         OSS_INLINE virtual void clearPath ()
         {
            _optPlanPath::clearPath() ;
            _pScanNode = NULL ;
            _sortRequired = FALSE ;
         }

         OSS_INLINE BOOLEAN isIxScan () const
         {
            return ( _pScanNode &&
                     OPT_PLAN_IDX_SCAN == _pScanNode->getType() ) ;
         }

      protected :
         optScanNode *     _pScanNode ;
         BOOLEAN           _sortRequired ;
   } ;

}

#endif //OPTPLANPATH_HPP__

