/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = ixm2dHash.cpp

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

#include "ixm2dHash.hpp"
#include "ixm2dBit.hpp"
#include "ixmGeoDef.hpp"

namespace engine
{
   geo2dBit bitSet ;

   void _ixm2dHash::_fill()
   {
      _hash = 0 ;
      for ( UINT32 i = 0; i < IXM_GEO_DEFAULT_CUT; ++i )
      {
         if ( bitSet.masks32[i] & _x )
         {
            _hash |= bitSet.masks64[i*2] ;
         }
         if ( bitSet.masks32[i] & _y )
         {
            _hash |= bitSet.masks64[i*2+1] ;
         }
      }
   }
}
