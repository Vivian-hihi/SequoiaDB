/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = dpsMergeBlock.cpp

   Descriptive Name = Data Protection Services Merge Block

   When/how to use: this program may be used on binary and text-formatted
   versions of DPS component. This file contains code logic for merging blocks

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          09/14/2012  YW  Initial Draft
          12/05/2012  TW  remove std

   Last Changed =

*******************************************************************************/
#include "core.hpp"
#include "dpsMergeBlock.hpp"
#include "pd.hpp"
#include "ossUtil.hpp"
#include "dpsLogPage.hpp"
#include "dpsReplicaLogMgr.hpp"
#include "pdTrace.hpp"
#include "dpsTrace.hpp"

namespace engine
{
   _dpsMergeBlock::_dpsMergeBlock():_isRow(FALSE)
   {
  //    clear() ;
   }

   _dpsMergeBlock::~_dpsMergeBlock()
   {
   }


   PD_TRACE_DECLARE_FUNCTION ( SDB__DPSMGBLK_CLEAR, "_dpsMergeBlock::clear" )
   void _dpsMergeBlock::clear()
   {
      PD_TRACE_ENTRY ( SDB__DPSMGBLK_CLEAR );
      _isRow = FALSE;
      _record.clear() ;
      _pageMeta.clear() ;
      PD_TRACE_EXIT ( SDB__DPSMGBLK_CLEAR );
   }

   _dpsMergeInfo::_dpsMergeInfo ()
   :_refer(_mergeBlock),
    _hasDummy(FALSE)
   {
   }

   _dpsMergeInfo::_dpsMergeInfo( dpsMergeBlock &block )
   :_refer(block),
    _hasDummy(FALSE)
   {
   }

   _dpsMergeInfo::~_dpsMergeInfo ()
   {
   }

   dpsMergeBlock &_dpsMergeInfo::getDummyBlock ()
   {
      return _dummyBlock ;
   }

   dpsMergeBlock &_dpsMergeInfo::getMergeBlock ()
   {
      return _refer ;
   }

   BOOLEAN _dpsMergeInfo::hasDummy ()
   {
      return _dummyBlock.pageMeta().valid() ? TRUE : FALSE ;
   }

}

