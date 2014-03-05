/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = qgmDef.cpp

   Descriptive Name =

   When/how to use: this program may be used on binary and text-formatted
   versions of PMD component. This file contains functions for agent processing.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          09/14/2012  YW  Initial Draft

   Last Changed =

*******************************************************************************/

#include "qgmDef.hpp"
#include "qgmUtil.hpp"
#include "pdTrace.hpp"
#include "qgmTrace.hpp"

namespace engine
{
   // PD_TRACE_DECLARE_FUNCTION( SDB__QGMFETCHOUT_ELEMENT, "_qgmFetchOut::element" )
   INT32 _qgmFetchOut::element( const _qgmDbAttr &attr,
                                BSONElement &ele )const
   {
      PD_TRACE_ENTRY( SDB__QGMFETCHOUT_ELEMENT ) ;
      INT32 rc = SDB_OK ;
      BSONElement next, local ;

      SDB_ASSERT( !attr.empty(), "impossible" )

      if ( NULL == this->next )
      {
         try
         {
            local = obj.getFieldDotted( attr.attr().toString() );
         }
         catch ( std::exception &e )
         {
            PD_LOG( PDERROR, "unexpected err happend:%s", e.what() ) ;
            rc = SDB_SYS ;
            goto error ;
         }

         ele = local ;
      }
      else
      {
         if ( attr.relegation().empty() )
         {
            try
            {
               local = obj.getFieldDotted( attr.attr().toString() ) ;
               next = this->next->obj.getFieldDotted( attr.attr().toString() ) ;
            }
            catch ( std::exception &e )
            {
               PD_LOG( PDERROR, "unexpected err happend:%s", e.what() ) ;
               rc = SDB_SYS ;
               goto error ;
            }

            if ( local.eoo() && !next.eoo() )
            {
               ele = next ;
            }
            else if ( !local.eoo() && next.eoo() )
            {
               ele = local ;
            }
            else if ( !local.eoo() && !next.eoo() )
            {
               rc = SDB_QGM_AMBIGUOUS_FIELD ;
               PD_LOG( PDERROR, "ambiguous filed name:%s, obj: %s, "
                       "next obj: %s", attr.attr().toString().c_str(),
                       obj.toString().c_str(),
                       this->next->obj.toString().c_str() ) ;
               goto error ;
            }
            else
            {
               PD_LOG( PDERROR, "field [%s] not found from fetchout, obj: %s, "
                       "next obj: %s", attr.attr().toString().c_str(),
                       obj.toString().c_str(),
                       this->next->obj.toString().c_str() ) ;
               rc = SDB_INVALIDARG ;
               goto error ;
            }
         }
         else
         {
            BSONObj srcObj ;
            if ( attr.relegation() == this->next->alias )
            {
               srcObj = this->next->obj ;
            }
            else if (  attr.relegation() == this->alias )
            {
               srcObj = this->obj ;
            }
            else
            {
               rc = SDB_INVALIDARG ;
               PD_LOG( PDERROR, "relegaion [%s] not found, alias: %s, "
                       "next alias: %s", attr.relegation().toString().c_str(),
                       alias.toString().c_str(),
                       this->next->alias.toString().c_str() ) ;
               goto error ;
            }

            try
            {
               local = srcObj.getFieldDotted( attr.attr().toString() ) ;
            }
            catch ( std::exception &e )
            {
               PD_LOG( PDERROR, "unexpected err happend:%s", e.what() ) ;
               rc = SDB_SYS ;
               goto error ;
            }

            ele = local ;
         }
      }

   done:
      PD_TRACE_EXITRC( SDB__QGMFETCHOUT_ELEMENT, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION( SDB__QGMFETCHOUT_ELEMENTS, "_qgmFetchOut::elements" )
   void _qgmFetchOut::elements( std::vector<BSONElement> &eles ) const
   {
      PD_TRACE_ENTRY( SDB__QGMFETCHOUT_ELEMENTS ) ;
      BSONObjIterator itr( obj ) ;
      while ( itr.more() )
      {
         eles.push_back( itr.next() ) ;
      }

      if ( NULL != next )
      {
         next->elements( eles ) ;
      }

      PD_TRACE_EXIT( SDB__QGMFETCHOUT_ELEMENTS ) ;
      return ;
   }

   BSONObj _qgmFetchOut::mergedObj()const
   {
      return NULL == next ?
             this->obj.getOwned() : qgmMerge( obj, next->mergedObj() ) ;
   }
}
