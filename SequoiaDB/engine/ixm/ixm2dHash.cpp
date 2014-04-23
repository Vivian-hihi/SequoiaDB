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
