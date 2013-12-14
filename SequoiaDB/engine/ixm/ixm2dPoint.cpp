/*******************************************************************************
   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

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
