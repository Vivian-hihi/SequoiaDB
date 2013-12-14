/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = ixm2dBit.hpp

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

#ifndef IXM2DBIT_HPP_
#define IXM2DBIT_HPP_

#include "core.hpp"
#include "oss.hpp"
namespace engine
{
   class _ixm2dBit : public SDBObject
   {
   public :
      UINT32 masks32[32] ;
      UINT64 masks64[64] ;
      _ixm2dBit()
      {
         for( UINT32 i = 0; i < 32; i++ )
         {
            masks32[i] = ( 1 << ( 31 - i ) );
         }

         for ( UINT32 i = 0; i < 64; i++ )
         {
            masks64[i] = ( 1LL << ( 63 - i ) ) ;
         }
      }
   } ;
   typedef class _ixm2dBit geo2dBit ;
}

#endif

