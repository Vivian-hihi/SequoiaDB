/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = rtnMergeSorting.hpp

   Descriptive Name =

   When/how to use: this program may be used on binary and text-formatted
   versions of Runtime component. This file contains declare for runtime
   functions.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          09/14/2012  YW  Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef RTNMERGESORTING_HPP_
#define RTNMERGESORTING_HPP_

#include "rtnSortDef.hpp"
#include "dmsTmpBlkUnit.hpp"
#include "rtnSortTuple.hpp"
#include "../bson/ordering.h"

namespace engine
{
   class _pmdEDUCB ;

   class _rtnMergeBlock : public SDBObject
   {
   public:
      _rtnMergeBlock()
      :_buf( NULL ),
       _size( 0 ),
       _read( 0 ),
       _loadSize( 0 )
      {

      }

      virtual ~_rtnMergeBlock(){}

   public:
      void init( _dmsTmpBlk &blk, CHAR *begin, UINT64 size ) ;

      INT32 next( _dmsTmpBlkUnit *unit, _rtnSortTuple **tuple ) ;

   private:
      INT32 _loadData( _dmsTmpBlkUnit *unit ) ;
   private:
      _dmsTmpBlk _blk ;
      CHAR *_buf ;
      UINT64 _size ;
      UINT64 _read ;
      UINT64 _loadSize ;
   } ;


   class _rtnMergeSorting : public SDBObject
   {
   public:
      _rtnMergeSorting( _dmsTmpBlkUnit *unit,
                        const BSONObj &orderby ) ;
      virtual ~_rtnMergeSorting() ;

   public:
      INT32 init( CHAR *buf, UINT64 size,
                  RTN_SORT_BLKS &src ) ;

      INT32 fetch( BSONObj &next, _pmdEDUCB *cb ) ;

   private:
      INT32 _merge( _pmdEDUCB *cb ) ;

      INT32 _mergeBlks( RTN_SORT_BLKS &src,
                        RTN_SORT_BLKS &dst,
                        _pmdEDUCB *cb ) ;

      INT32 _makeHeap( RTN_SORT_BLKS &src,
                       _pmdEDUCB *cb ) ;

      INT32 _pushObjFromSink( UINT32 i ) ;
   private:
      struct _rtnMergeUnitHelper
      {
         UINT64 _originalSize ;
         UINT64 _newBlkSize ;
         UINT64 _outputStart ;

         _rtnMergeUnitHelper()
         :_originalSize(0),
          _newBlkSize(0),
          _outputStart(0)
         {

         }
      } ;

   private:
      Ordering _order ;
      CHAR *_buf ;
      UINT64 _size ;
      UINT64 _mergeBufSize ;
      UINT64 _mergePos ;

      _rtnMergeUnitHelper _unitHelper ;

      _rtnMergeBlock _dataSink[RTN_SORT_MAX_MERGESIZE] ;
      RTN_MERGE_HEAP _heap ;
      UINT32 _mergeBlkSize ;
      UINT32 _mergeMax ;
      RTN_SORT_BLKS _merged ;
      RTN_SORT_BLKS *_src ;

      BOOLEAN _mergeDone ;

      _dmsTmpBlkUnit *_unit ;
   } ;
}

#endif

