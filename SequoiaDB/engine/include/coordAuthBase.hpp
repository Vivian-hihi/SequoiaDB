/*******************************************************************************


   Copyright (C) 2011-2018 SequoiaDB Ltd.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

   Source File Name = coordAuthBase.hpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          04/18/2017  XJH Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef COORD_AUTHBASE_HPP__
#define COORD_AUTHBASE_HPP__

#include "coordOperator.hpp"

namespace engine
{

   /*
      _coordAuthBase define
   */
   class _coordAuthBase : public _coordOperator
   {
      public:
         _coordAuthBase() ;
         virtual ~_coordAuthBase() ;

      protected:
         INT32 forward( MsgHeader *pMsg,
                        pmdEDUCB *cb,
                        BOOLEAN sWhenNoPrimary,
                        INT64 &contextID,
                        const CHAR **ppUserName = NULL,
                        const CHAR **ppPass = NULL ) ;

   } ;
   typedef _coordAuthBase coordAuthBase ;

}

#endif // COORD_AUTHBASE_HPP__

