/*******************************************************************************


   Copyright (C) 2011-2018 SequoiaDB Ltd.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU Affero General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Affero General Public License for more details.

   You should have received a copy of the GNU Affero General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.

   Source File Name = dpsTransVersionCtrl.cpp

   Descriptive Name = dps transaction version control

   When/how to use: this program may be used on binary and text-formatted
   versions of Data Protection component. This file contains functions for
   transaction isolation control through version control implmenetation.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          05/08/2019  Linyoub  Initial Draft

   Last Changed =

*******************************************************************************/

#include "dpsUtil.hpp"

namespace engine
{
   static BOOLEAN g_TimeonFlag = FALSE ;

   void dpsSetTimeonFlag( BOOLEAN flag )
   {
      g_TimeonFlag = flag ;
   }

   BOOLEAN dpsGetTimeonFlag()
   {
      return g_TimeonFlag ;
   }
}


