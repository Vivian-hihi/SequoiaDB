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

   Source File Name = urldecode.h

   Descriptive Name = url decode

   When/how to use: this program may be used on binary and text-formatted
   versions of UTIL component. This file contains declare of json2rawbson. Note
   this function should NEVER be directly called other than fromjson.cpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          06/20/2014  JWH Initial Draft

   Last Changed =

*******************************************************************************/
#ifndef UTIL_URL_H__
#define UTIL_URL_H__

SDB_EXTERN_C_START

SDB_EXPORT INT32 urlDecodeSize( const CHAR *pBuffer, INT32 bufferSize ) ;

SDB_EXPORT void urlDecode( const CHAR *pBuffer, INT32 bufferSize,
                           CHAR **pOut, INT32 outSize ) ;

SDB_EXTERN_C_END

#endif