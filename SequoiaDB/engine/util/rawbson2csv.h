/*******************************************************************************

   Copyright (C) 2012-2014 SequoiaDB Ltd.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

   Source File Name = csv2rawbson.h

   Descriptive Name = CSV To Raw BSON

   When/how to use: this program may be used on binary and text-formatted
   versions of UTIL component. This file contains declare of json2rawbson. Note
   this function should NEVER be directly called other than fromjson.cpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          05/04/2014  JWH Initial Draft

   Last Changed =

*******************************************************************************/
#ifndef UTIL_BSON_2_CSV_H__
#define UTIL_BSON_2_CSV_H__

#include "core.h"
#include "oss.h"
#include "ossUtil.h"
#include "ossMem.h"
#include "msg.h"

#define CSV_STR_UNDEFINED  "undefined"
#define CSV_STR_MINKEY     "minKey"
#define CSV_STR_MAXKEY     "maxKey"

#define CSV_STR_UNDEFINED_SIZE   ( sizeof( CSV_STR_UNDEFINED ) - 1 )
#define CSV_STR_MINKEY_SIZE      ( sizeof( CSV_STR_MINKEY ) - 1 )
#define CSV_STR_MAXKEY_SIZE      ( sizeof( CSV_STR_MAXKEY ) - 1 )

SDB_EXTERN_C_START

SDB_EXPORT INT32 getCSVSize ( CHAR delChar, CHAR delField,
                              CHAR delRecord, CHAR *pbson,
                              INT32 *pCSVSize ) ;
SDB_EXPORT INT32 bson2csv( CHAR delChar, CHAR delField,
                           CHAR delRecord, CHAR *pbson,
                           CHAR **ppBuffer, INT32 *pCSVSize ) ;
SDB_EXTERN_C_END

#endif