/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = qgmOptiSelect.hpp

   Descriptive Name =

   When/how to use: this program may be used on binary and text-formatted
   versions of PMD component. This file contains functions for agent processing.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          09/14/2012  JHL  Initial Draft

   Last Changed =

******************************************************************************/

#ifndef QGMOPTIMTHMATCHSELECT_HPP_
#define QGMOPTIMTHMATCHSELECT_HPP_

#include "qgmOptiSelect.hpp"
#include "mthMatcher.hpp"
#include "../bson/bsonobj.h"

namespace engine
{
   class qgmOptiMthMatchSelect : public _qgmOptiSelect
   {
   public:
      qgmOptiMthMatchSelect( _qgmPtrTable *pTable, _qgmParamTable *pParam );
      virtual ~qgmOptiMthMatchSelect();

      virtual BOOLEAN isEmpty();

      virtual INT32 init (){ return SDB_OK; }

      INT32 fromBson( const bson::BSONObj &matcher );

   protected:
      virtual INT32 _extend( _qgmOptiTreeNode *&exNode ) ;

   public:
      bson::BSONObj        _matcher;
   };
}

#endif
