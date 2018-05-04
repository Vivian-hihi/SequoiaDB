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

   Source File Name = sptConvertorHelper.hpp

   Descriptive Name =

   When/how to use: this program may be used on binary and text-formatted
   versions of Script component. This file contains structures for javascript
   engine wrapper

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          01/13/2013  YW Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef SPTCONVERTORHELPER_HPP_
#define SPTCONVERTORHELPER_HPP_

#include "core.hpp"
#include "jsapi.h"
#include <string>
#include "../bson/bson.hpp"
using bson::BSONObj ;

namespace engine
{
   INT32 JSVal2String( JSContext *cx, const jsval &val, std::string &str ) ;
   // caller should free the return pointer using SAFE_JS_FREE
   CHAR *convertJsvalToString ( JSContext *cx , jsval val ) ;

   INT32 cursorNextRecord( void *cursor, BSONObj &record ) ;
}

#endif

