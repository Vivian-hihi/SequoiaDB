/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = rtnSorting.hpp

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

#ifndef RTNSORTING_HPP_
#define RTNSORTING_HPP_

#include "rtnSortDef.hpp"
#include "rtnContext.hpp"
#include "dmsTmpBlkUnit.hpp"
#include "rtnMergeSorting.hpp"

using namespace bson ;

namespace engine
{
   class _pmdEDUCB ;
   class _rtnInternalSorting ;


   class _rtnSorting : public SDBObject
   {
   public:
      _rtnSorting();
      virtual ~_rtnSorting();

   public:
      INT32 init( UINT64 bufSize,
                  const BSONObj &orderby,
                  rtnContext *context,
                  SINT64 fino,
                  SINT64 limit,
                  _pmdEDUCB *cb );

      /// do not ensure that the next is get owned.
      INT32 fetch( BSONObj &next, _pmdEDUCB *cb ) ;

   private:
      INT32 _crtSortedBlks( RTN_SORT_BLKS &blks, _pmdEDUCB *cb ) ;

      INT32 _moveToExternalBlks( _rtnInternalSorting *inter,
                                 RTN_SORT_BLKS &blks,
                                 _pmdEDUCB *cb ) ;

      INT32 _fetchFromInter( BSONObj &next ) ;

      INT32 _fetchFromExter( BSONObj &next,
                             _pmdEDUCB *cb ) ;
   private:
      _dmsTmpBlkUnit _unit ;
      BSONObj _orderby ;
      CHAR *_sortBuf ;
      UINT64 _totalBufSize ;
      RTN_SORT_STEP _step ;
      _pmdEDUCB *_cb ;
      rtnContext *_context ;
      _rtnInternalSorting *_internalBlk ;
      _rtnMergeSorting *_mergeBlk ;
      RTN_SORT_BLKS _blks ;
      UINT64 _blkBegin ;
      SINT64 _fino ;
      SINT64 _limit ;
   };

   typedef class _rtnSorting rtnSorting;
}

#endif

