/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = rtnContextSort.hpp

   Descriptive Name = RunTime Context Header

   When/how to use: this program may be used on binary and text-formatted
   versions of Runtime component. This file contains structure for Runtime
   Context.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          09/14/2012  TW  Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef RTNCONTEXTSORT_HPP_
#define RTNCONTEXTSORT_HPP_

#include "rtnContext.hpp"
#include "rtnSorting.hpp"

namespace engine
{
   class _rtnContextSort : public _rtnContextBase
   {
   public:
      _rtnContextSort( INT64 contextID, UINT64 eduID ) ;
      virtual ~_rtnContextSort() ;

   public:
      virtual RTN_CONTEXT_TYPE getType() const ;
      virtual _dmsStorageUnit*  getSU () { return NULL ; }

      INT32 open( const BSONObj &orderBy,
                  rtnContext *context,
                  _pmdEDUCB *cb,
                  SINT64 numToSkip = 0,
                  SINT64 numToReturn = -1 ) ;

   protected:
      virtual INT32 _prepareData( _pmdEDUCB *cb ) ;

   private:
      _rtnSorting _sorting ;
      SINT64 _skip ;
      SINT64 _limit ;
   } ;
   typedef class _rtnContextSort rtnContextSort ;
}

#endif

