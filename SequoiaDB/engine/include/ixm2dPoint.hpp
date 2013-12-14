/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = ixm2dPoint.hpp

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

#ifndef IXM2DPOINT_HPP_
#define IXM2DPOINT_HPP_

#include "core.hpp"
#include "oss.hpp"
#include "ixmGeoDef.hpp"
#include "ixm2dIndexCB.hpp"
#include "ixm2dHash.hpp"
#include "../bson/bson.h"

namespace engine
{
   class _ixm2dPoint : public SDBObject
   {
   public:
      _ixm2dPoint( const BSONElement &tuple, const BSONObj &pattern ) ;

      _ixm2dPoint( const BSONElement &tuple, const geoIndexCB &cb ) ;

      BOOLEAN initialized()
      {
         return _initialized ;
      }

      inline geoHash hash( const geoIndexCB &cb )
      {
         return geoHash( _convert( _tuple.x, cb.xScaling ),
                         _convert( _tuple.y, cb.yScaling ) ) ;
      }

   private:
      void _init( const BSONElement &tuple, const BSONObj &pattern ) ;
      void _init( const BSONElement &tuple, const geoIndexCB &cb ) ;

      inline UINT32 _convert( const FLOAT64 &in, const FLOAT64 scaling )
      {
         return ( UINT32 )( in * scaling ) ;
      }

   private:
      geoTuple _tuple ;
      BOOLEAN _initialized ;
   } ;

   typedef class _ixm2dPoint geoPoint ;
}

#endif

