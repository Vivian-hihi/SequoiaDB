/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = ixm.cpp

   Descriptive Name = Index Manager

   When/how to use: this program may be used on binary and text-formatted
   versions of Index Manager component. This file contains functions for
   Index Manager Control Block.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          03/07/2014  TW  Initial Draft

   Last Changed =

*******************************************************************************/

#include "ixm_common.hpp"
#include "ossUtil.hpp"
#include "pd.hpp"
#include "pdTrace.hpp"
#include "ixmTrace.hpp"

using namespace bson;

namespace engine
{

   /*
      Tool functions
   */
   void ixmMakeHashValue( const BSONObj &obj, const BSONObj &key,
                          ixmHashValue & hashValue )
   {
      hashValue.hash = 0 ;
      try
      {
         BSONObjIterator itr( key ) ;
         while ( itr.more() )
         {
            BSONElement orderEle = itr.next() ;
            const CHAR* name = orderEle.fieldName() ;
            BSONElement arrEle = obj.getFieldDottedOrArray( name ) ;
            if ( Array == arrEle.type() )
            {
               hashValue.columns.hash1 = ossHash( arrEle.value(),
                                                  arrEle.valuesize() ) ;
               hashValue.columns.hash2 = ossHash( arrEle.value(),
                                                  arrEle.valuesize(), 3 ) ;
               break ;
            }
         }
      }
      catch( std::exception & e )
      {
         PD_LOG( PDWARNING, "Make hash value occurred exception: %s",
                 e.what() ) ;
      }
   }

}

