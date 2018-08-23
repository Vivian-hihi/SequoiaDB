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

   Source File Name = utilUniqueID.hpp

   Descriptive Name =

   When/how to use: Process CS/CL Unique ID

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who      Description
   ====== =========== ======== ==============================================
          05/24/2018  Ting YU  Initial Draft

   Last Changed =

*******************************************************************************/
#ifndef UTIL_UNIQUEID_HPP_
#define UTIL_UNIQUEID_HPP_

#include "ossTypes.hpp"
#include "ossUtil.hpp"
#include "../bson/bsonobj.h"

using namespace bson ;

namespace engine
{
   #define UTIL_INVALID_UNIQUEID      0

   typedef UINT32 utilCSUniqueID ;
   typedef UINT64 utilCLUniqueID ;

   typedef std::pair<std::string, utilCLUniqueID> PAIR_CLNAME_ID ;

   OSS_INLINE utilCSUniqueID utilGetCSUniqueID( utilCLUniqueID clUniqueID )
   {
      return clUniqueID >> 32 ;
   }

   vector<PAIR_CLNAME_ID> utilBson2ClPair( const BSONObj& clInfoObj ) ;

   BSONObj utilUnsetUniqueID( const BSONObj& clInfoObj ) ;
}

#endif //UTIL_UNIQUEID_HPP_

