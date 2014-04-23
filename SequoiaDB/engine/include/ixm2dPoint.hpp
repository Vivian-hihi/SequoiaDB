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

      OSS_INLINE geoHash hash( const geoIndexCB &cb )
      {
         return geoHash( _convert( _tuple.x, cb.xScaling ),
                         _convert( _tuple.y, cb.yScaling ) ) ;
      }

   private:
      void _init( const BSONElement &tuple, const BSONObj &pattern ) ;
      void _init( const BSONElement &tuple, const geoIndexCB &cb ) ;

      OSS_INLINE UINT32 _convert( const FLOAT64 &in, const FLOAT64 scaling )
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

