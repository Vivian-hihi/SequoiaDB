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

