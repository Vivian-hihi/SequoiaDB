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

   Source File Name = monClass.cpp

   Descriptive Name = Monitor Class source

   When/how to use: this program may be used on binary and text-formatted
   versions of OSS component. This file contains functions for OSS operations.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          08/14/2019  CW  Initial Draft

   Last Changed =

*******************************************************************************/
#include "monLatch.hpp"
#include "pmd.hpp"

namespace engine
{
monSpinXLatch::monSpinXLatch( MON_LATCH_IDENTIFIER latchID )
{
   _latchID = latchID ;
}

monSpinXLatch::~monSpinXLatch()
{
}

void monSpinXLatch::get()
{
   _latch.get() ;
}

void monSpinXLatch::release()
{
   _latch.release() ;
   _ownerTID = 0 ;
}


BOOLEAN monSpinXLatch::try_get()
{
   return _latch.try_get() ;
}

/**
 * monSpinSLatch implements
 */

monSpinSLatch::monSpinSLatch( MON_LATCH_IDENTIFIER latchID )
{
   _latchID = latchID ;
}

monSpinSLatch::~monSpinSLatch()
{
}

void monSpinSLatch::get()
{
   _latch.get() ;
}

void monSpinSLatch::release()
{
   _latch.release() ;
   _ownerTID = 0 ;
}

void monSpinSLatch::get_shared ()
{
  _latch.get_shared() ;
}
void monSpinSLatch::release_shared ()
{
   _latch.release_shared() ;
}

BOOLEAN monSpinSLatch::try_get_shared()
{
   return _latch.try_get_shared() ;
}

BOOLEAN monSpinSLatch::try_get()
{
   return _latch.try_get() ;
}

}
