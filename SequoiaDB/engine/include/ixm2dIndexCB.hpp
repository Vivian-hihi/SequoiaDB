/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = ixm2dIndexCB.hpp

   Descriptive Name =

   When/how to use: this program may be used on binary and text-formatted
   versions of Index Manager component. This file contains functions for index
   key generator, which is used to create key pairs from data record and index
   definition.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          09/14/2012  YW  Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef IXM2DINDEXCB_HPP_
#define IXM2DINDEXCB_HPP_

#include "core.hpp"
#include "oss.hpp"
#include "ixmGeoDef.hpp"
#include "../bson/bson.h"


namespace engine
{
   class _ixm2dIndexCB : public SDBObject
   {
   public:
      _ixm2dIndexCB() ;
      INT32 init( const BSONObj &pattern ) ;

   public:
      geoTuple min ;
      geoTuple max ;
      FLOAT64 xScaling ;
      FLOAT64 yScaling ;
      UINT32 cut ;
   } ;

   typedef class _ixm2dIndexCB geoIndexCB ;
}

#endif

