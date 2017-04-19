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

   Source File Name = optCommon.cpp

   Descriptive Name = Optimizer common functions

   When/how to use: this program may be used on binary and text-formatted
   versions of Optimizer component. This file contains functions for optimizer
   access plan creation. It will calculate based on rules and try to estimate
   a lowest cost plan to access data.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          03/14/2017  HGM Initial Draft

   Last Changed =

*******************************************************************************/

#include "optCommon.hpp"

namespace engine
{
   double optConvertStrToScalar ( const CHAR *pValue, UINT32 valueSize,
                                  UINT8 low, UINT8 high )
   {
      if ( 0 == valueSize )
      {
         // Empty string
         return 0.0 ;
      }

      UINT8 base = high - low + 1;
      double scalar = 0.0 ;
      double denom = base ;

      // Convert initial characters to fraction
      while ( valueSize-- > 0 )
      {
         UINT8 ch = (UINT8) *( pValue++ ) ;

         ch = OPT_ROUND( ch, low, high ) ;
         scalar += ( (double) (ch - low) ) / denom ;
         denom *= base ;
      }

      return scalar ;
   }
}
