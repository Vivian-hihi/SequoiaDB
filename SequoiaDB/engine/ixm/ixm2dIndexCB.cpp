/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = ixm2dIndexCB.cpp

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

#include "ixm2dIndexCB.hpp"
#include "ixm.hpp"
#include "ixmGeo.hpp"
#include "pdTrace.hpp"
#include "ixmTrace.hpp"

namespace engine
{
   const FLOAT64 IXM_GEO_SCALING_RANGE = 4.0 * 1024 * 1024 * 1024 ;

   _ixm2dIndexCB::_ixm2dIndexCB()
   : min( IXM_GEO_MIN_X, IXM_GEO_MIN_Y ),
     max( IXM_GEO_MAX_X, IXM_GEO_MAX_Y ),
     xScaling( 1 ),
     yScaling( 1 ),
     cut( IXM_GEO_DEFAULT_CUT )
   {
      xScaling = IXM_GEO_SCALING_RANGE / ( max.x - min.x ) ;
      yScaling = IXM_GEO_SCALING_RANGE / ( max.y - min.y ) ;
   }

   PD_TRACE_DECLARE_FUNCTION ( SDB__IXM2DINXCB_INIT, "_ixm2dIndexCB::init" )
   INT32 _ixm2dIndexCB::init( const BSONObj &pattern )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB__IXM2DINXCB_INIT ) ;
      BSONElement range = pattern.getField( IXM_2DRANGE_FIELD ) ;
      if ( range.eoo() )
      {
         goto done ;
      }
      else if ( !range.isABSONObj() )
      {
         goto error ;
      }
      else
      {
         BSONObjIterator i( range.embeddedObject() ) ;
         if ( i.more() )
         {
            BSONElement minE = i.next() ;
            rc = ixmExtractTuple( minE, min.x, min.y ) ;
            if ( SDB_OK != rc )
            {
               goto error ;
            }
         }
         else
         {
            goto error ;
         }

         if ( i.more() )
         {
            BSONElement maxE = i.next() ;
            rc = ixmExtractTuple( maxE, max.x, max.y ) ;
            if ( SDB_OK != rc )
            {
               goto error ;
            }
         }
         else
         {
            goto error ;
         }
      }

      /// todo: do not use == to compare float
      if ( max.x <= min.x ||
           max.y <= min.y )
      {
         goto error ;
      }

      xScaling = IXM_GEO_SCALING_RANGE / ( max.x - min.x ) ;
      yScaling = IXM_GEO_SCALING_RANGE / ( max.y - min.y ) ;
      PD_TRACE2 ( SDB__IXM2DINXCB_INIT, PD_PACK_DOUBLE(xScaling),
	                                                           PD_PACK_DOUBLE(yScaling) );
   done:
      PD_TRACE_EXITRC ( SDB__IXM2DINXCB_INIT, rc ) ;
      return rc ;
   error:
      rc = SDB_INVALIDARG ;
      goto done ;
   }
}
