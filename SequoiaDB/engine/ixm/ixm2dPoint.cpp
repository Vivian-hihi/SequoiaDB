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

   Source File Name = ixm2dPoint.cpp

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

#include "ixm2dPoint.hpp"
#include "ixmGeo.hpp"
#include "pdTrace.hpp"
#include "ixmTrace.hpp"

namespace engine
{
   _ixm2dPoint::_ixm2dPoint( const BSONElement &tuple, const BSONObj &pattern ):
   _initialized( FALSE )
   {
      _init( tuple, pattern ) ;
   }

   _ixm2dPoint::_ixm2dPoint( const BSONElement &tuple, const geoIndexCB &cb ):
   _initialized( FALSE )
   {
      _init( tuple, cb ) ;
   }

   PD_TRACE_DECLARE_FUNCTION ( SDB__IXM2DPNT__INIT1, "_ixm2dPoint::_init" )
   void _ixm2dPoint::_init( const BSONElement &tuple, const BSONObj &pattern )
   {
      PD_TRACE_ENTRY ( SDB__IXM2DPNT__INIT1) ;
      geoIndexCB cb ;
      if ( SDB_OK != cb.init( pattern ) )
      {
         goto done ;
      }
      if ( SDB_OK != ixmExtractTuple( tuple, _tuple.x, _tuple.y ) )
      {
         goto done ;
      }
      /// todo: compare float do not use "=="
      if ( _tuple.x < cb.min.x ||
           _tuple.y < cb.min.y ||
           ( cb.max.x <= _tuple.x ) ||
           ( cb.max.y <= _tuple.y ) )
      {
         goto done ;
      }
      _initialized = TRUE ;
   done:
      PD_TRACE_EXIT( SDB__IXM2DPNT__INIT1 ) ;
      return ;
   }

   PD_TRACE_DECLARE_FUNCTION ( SDB__IXM2DPNT__INIT2, "_ixm2dPoint::_init" )
   void _ixm2dPoint::_init( const BSONElement &tuple, const geoIndexCB &cb )
   {
      PD_TRACE_ENTRY ( SDB__IXM2DPNT__INIT2 );
      if ( SDB_OK != ixmExtractTuple( tuple, _tuple.x, _tuple.y ) )
      {
         goto done ;
      }
      if ( _tuple.x < cb.min.x ||
           _tuple.y < cb.min.y ||
           ( cb.max.x <= _tuple.x ) ||
           ( cb.max.y <= _tuple.y ) )
      {
         goto done ;
      }
      _initialized = TRUE ;
   done:
      PD_TRACE_EXIT ( SDB__IXM2DPNT__INIT2 );
      return ;
   }
}
