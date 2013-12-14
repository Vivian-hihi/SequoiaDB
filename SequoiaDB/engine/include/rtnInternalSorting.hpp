/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = rtnInternalSorting.hpp

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

#ifndef RTNINTERNALSORTING_HPP_
#define RTNINTERNALSORTING_HPP_

#include "rtnSortDef.hpp"
#include "rtnSortTuple.hpp"
#include "ixmIndexKey.hpp"
#include "../bson/ordering.h"

//#include <vector>

namespace engine
{
   class _pmdEDUCB ;

   class _rtnInternalSorting : public SDBObject
   {
   public:
      _rtnInternalSorting( const BSONObj &orderby,
                           CHAR *buf, UINT64 size ) ;
      virtual ~_rtnInternalSorting() ;

   public:
      INT32 push( const BSONObj &obj ) ;

      void clearBuf() ;

      INT32 sort( _pmdEDUCB *cb ) ;

      BOOLEAN more() const
      {
         return  _fetched < _objNum ;
      }

      INT32 next( _rtnSortTuple **tuple ) ;

//      static INT32 compare(const BSONObj &obj1, _rtnSortTuple *tuple1,
//                           const BSONObj &obj2, _rtnSortTuple *tuple2,
//                           const bson::Ordering &order ) ;

   private:
      INT32 _quickSort( _rtnSortTuple **left,
                        _rtnSortTuple **right,
                        _pmdEDUCB *cb ) ;

      INT32 _partition( _rtnSortTuple **left,
                        _rtnSortTuple **right,
                        _rtnSortTuple **&leftAxis,
                        _rtnSortTuple **&rightAxis ) ;

      INT32 _insertSort( _rtnSortTuple **left,
                         _rtnSortTuple **right ) ;

      INT32 _setHashFromObj( const BSONObj &obj, _rtnSortTuple *tuple ) ;

      INT32 _swapLeftSameKey( _rtnSortTuple **left,
                              _rtnSortTuple **right,
                              _rtnSortTuple **&axis ) ;

      INT32 _swapRightSameKey( _rtnSortTuple **left,
                               _rtnSortTuple **right,
                               _rtnSortTuple **&axis ) ;


   private:
      //std::vector<UINT32> _rands ;
      //UINT32 _rand ;
      bson::BSONObj _orderObj ;
      _ixmIndexKeyGen _keyGen ;
      bson::Ordering _order ;
      CHAR *_begin ;
      UINT64 _totalSize ;
      UINT64 _headOffset ;
      UINT64 _tailOffset ;
      UINT64 _objNum ;
      UINT64 _fetched ;
      UINT64 _recursion ;
   } ;
}

#endif

