/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = ixmGeo.cpp

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

#include "ixmGeo.hpp"
#include "ixm2dIndexCB.hpp"
#include "ixm2dPoint.hpp"
#include "ixm2dHash.hpp"
#include "pd.hpp"
#include "pdTrace.hpp"
#include "ixmTrace.hpp"

namespace engine
{
   PD_TRACE_DECLARE_FUNCTION ( SDB_IXMGEOHASH, "ixmGeoHash" )
   INT32 ixmGeoHash( const BSONElement &ele, BSONElement &hash,
                     BOOLEAN isArrFixed, vector<BSONObj *> &objs,
                     const BSONObj &keyPattern )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB_IXMGEOHASH );
      geoIndexCB cb ;
      if ( SDB_OK != cb.init( keyPattern ) )
      {
         goto error ;
      }
      {
      geoPoint po( ele, cb ) ;
      if ( !po.initialized() )
      {
         goto error ;
      }
      {
      geoHash vh = po.hash( cb ) ;
      BSONObjBuilder b ;
      b.appendNumber( "",(SINT64)vh.hash() ) ;
// why need to allocate new object?
// can we do objs.push_back ( b.obj() ) ?
      BSONObj *obj = SDB_OSS_NEW BSONObj() ;
      if ( NULL == obj )
      {
         PD_LOG( PDERROR, "failed to allocate mem." ) ;
         rc = SDB_OOM ;
         goto error ;
      }
      *obj = b.obj() ;
      objs.push_back( obj ) ;
      hash = obj->firstElement() ;
      }
      }
   done:
      PD_TRACE_EXITRC ( SDB_IXMGEOHASH, rc );
      return rc ;
   error:
      rc = SDB_INVALIDARG ;
      goto done ;
   }

   PD_TRACE_DECLARE_FUNCTION ( SDB_IXMEXTRACTTUPLE, "ixmExtractTuple" )
   INT32 ixmExtractTuple( const BSONElement &ele,
                          FLOAT64 &x,
                          FLOAT64 &y )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB_IXMEXTRACTTUPLE );
      if ( ele.eoo() ||
           !ele.isABSONObj() )
      {
         goto error ;
      }
      else
      {
         BSONObjIterator i( ele.embeddedObject() ) ;
         if ( i.more() )
         {
            BSONElement ex = i.next() ;
            BSONElement ey ;
            if ( i.more() )
            {
               ey = i.next() ;
            }
            else
            {
               goto error ;
            }

            if ( ex.eoo() || !ex.isNumber() ||
                 ey.eoo() || !ey.isNumber() )
            {
               goto error ;
            }
            else
            {
               x = ex.Number() ;
               y = ey.Number() ;
            }
         }
         else
         {
            goto error ;
         }
      }
   done:
      PD_TRACE_EXITRC ( SDB_IXMEXTRACTTUPLE, rc );
      return rc ;
   error:
      rc = SDB_INVALIDARG ;
      goto done ;
   }
}
