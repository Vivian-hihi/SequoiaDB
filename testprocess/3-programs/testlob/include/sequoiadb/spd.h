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

*******************************************************************************/

/** \file spd.h
    \brief store procedure
*/

#ifndef SPD_H_
#define SPD_H_

#include "core.h"

SDB_EXTERN_C_START

enum _SDB_SPD_RES_TYPE
{
   SDB_SPD_RES_TYPE_VOID = 0,
   SDB_SPD_RES_TYPE_STR,
   SDB_SPD_RES_TYPE_NUMBER,
   SDB_SPD_RES_TYPE_OBJ,
   SDB_SPD_RES_TYPE_BOOL,
   SDB_SPD_RES_TYPE_RECORDSET,
   SDB_SPD_RES_TYPE_CS,
   SDB_SPD_RES_TYPE_CL,
   SDB_SPD_RES_TYPE_RG,
   SDB_SPD_RES_TYPE_RN,
} ;

typedef enum _SDB_SPD_RES_TYPE SDB_SPD_RES_TYPE ;

SDB_EXTERN_C_END

#endif

