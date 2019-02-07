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

   Source File Name = dpsTransLockCallback.cpp

   Descriptive Name = DPS Transaction Lock Callback

   When/how to use: this program may be used on binary and text-formatted
   versions of OSS component. This file contains declare for data types used in
   SequoiaDB.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          19/01/2019  CYX Initial Draft

   Last Changed =

*******************************************************************************/

#include "dpsTransLockCallback.hpp"
#include "pmdEDU.hpp"
#include "dpsTransCB.hpp"
#include "dmsScanner.hpp"
#include "dmsStorageDataCommon.hpp"

namespace engine
{

   _dpsITransLockCallback * dpsTransLockCallbackFactory::create
   (
      const INT32    type,
      pmdEDUCB     * eduCB,
      dmsRecordRW  * recordRW
   )
   {
      dpsTransCB   * transCB = pmdGetKRCB()->getTransCB() ;
      _dpsITransLockCallback * callback = NULL;

      // don't setup callback if transaction is not enabled
      if( !transCB->isTransOn() )
      {
         goto done;
      }

      if( LOCK_CALLBACK_TYPE_DMS == type )
      {
         callback = SDB_OSS_NEW dmsTransLockCallback( transCB,
                                                      eduCB,
                                                      recordRW );
      }

      if ( !callback )
      {
         PD_LOG( PDERROR, "Allocate memory for callback functions failed" ) ;
         goto error ;
      }
      done:
      return callback;

      error:
      goto done;
   }

}

