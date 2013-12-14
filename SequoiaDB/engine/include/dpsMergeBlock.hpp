/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = dpsMergeBlock.hpp

   Descriptive Name =

   When/how to use: this program may be used on binary and text-formatted
   versions of OSS component. This file contains declare for data types used in
   SequoiaDB.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          09/14/2012  YW  Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef DPSMERGEBLOCK_H_
#define DPSMERGEBLOCK_H_

#include "core.hpp"
#include "oss.hpp"
#include "dpsLogDef.hpp"
#include "dpsLogRecord.hpp"
#include "dpsPageMeta.hpp"

namespace engine
{
   class _dpsLogPage ;
   class _dpsReplicaLogMgr ;
   class _dpsMergeInfo ;

   class _dpsMergeBlock : public SDBObject
   {
      friend class _dpsReplicaLogMgr ;
      friend class _dpsMergeInfo ;

      private:
         dpsLogRecord _record ;
         dpsPageMeta _pageMeta ;
         BOOLEAN _isRow;

      public:
         _dpsMergeBlock();

         ~_dpsMergeBlock();

      public:
         inline BOOLEAN isRow()
         {
            return _isRow;
         }

         inline void setRow( BOOLEAN isRow )
         {
            _isRow = isRow;
         }

         inline dpsLogRecord &record()
         {
            return _record ;
         }

         inline dpsPageMeta &pageMeta()
         {
            return _pageMeta ;
         }

      public:
         void clear();
   };

   typedef class _dpsMergeBlock dpsMergeBlock;

   class _dpsMergeInfo : public SDBObject
   {
      friend class _dpsReplicaLogMgr ;

      public:
         _dpsMergeInfo () ;
         _dpsMergeInfo ( dpsMergeBlock &block ) ;
         ~_dpsMergeInfo () ;

         dpsMergeBlock &getDummyBlock () ;
         dpsMergeBlock &getMergeBlock () ;
         BOOLEAN       hasDummy () ;
         void clear()
         {
            _mergeBlock.clear() ;
            _dummyBlock.clear() ;
            _hasDummy = FALSE ;
         }

      private:
         dpsMergeBlock        _mergeBlock ;
         dpsMergeBlock        _dummyBlock ;
         dpsMergeBlock        &_refer ;
         BOOLEAN              _hasDummy ;
   } ;

   typedef class _dpsMergeInfo dpsMergeInfo ;

}

#endif
