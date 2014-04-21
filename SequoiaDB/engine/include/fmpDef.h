/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = fmpDef.h

   Descriptive Name =

   When/how to use:

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          06/19/2013  YW  Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef FMPDEF_H_
#define FMPDEF_H_

#include "msgDef.h"
#include "spd.h"

#define SPD_PROCESS_NAME "sdbfmp"

/// WARNING: do not modify this define.
/// spdFMP.cpp::SPD_NEXT is depend on this define.
#define FMP_MSG_MAGIC "-_-!"

#define FMP_FUNC_VALUE FIELD_NAME_FUNC
#define FMP_FUNC_NAME "name"
#define FMP_FUNC_TYPE FIELD_NAME_FUNCTYPE
#define FMP_ERR_MSG "errmsg"
#define FMP_RES_TYPE "resType"
#define FMP_RES_VALUE "value"
#define FMP_RES_CODE "retCode"
#define FMP_CONTROL_FIELD "step"
#define FMP_DIAG_PATH "diag"
#define FMP_FUNCTION_DEF "function"
#define FMP_LOCAL_SERVICE "service"
#define FMP_TMP_USRNAME "usrname"

#define FMP_CONTROL_STEP_INVALID -3
#define FMP_CONTROL_STEP_QUIT -2
#define FMP_CONTROL_STEP_RESET -1
#define FMP_CONTROL_STEP_BEGIN 0
#define FMP_CONTROL_STEP_DOWNLOAD 1
#define FMP_CONTROL_STEP_EVAL 2
#define FMP_CONTROL_STEP_FETCH 3
/// when add step, max must be changed.
#define FMP_CONTROL_SETP_MAX 4


#define FMP_RES_TYPE_VOID SDB_SPD_RES_TYPE_VOID
#define FMP_RES_TYPE_STR SDB_SPD_RES_TYPE_STR
#define FMP_RES_TYPE_NUMBER SDB_SPD_RES_TYPE_NUMBER
#define FMP_RES_TYPE_OBJ SDB_SPD_RES_TYPE_OBJ
#define FMP_RES_TYPE_BOOL SDB_SPD_RES_TYPE_BOOL
#define FMP_RES_TYPE_RECORDSET SDB_SPD_RES_TYPE_RECORDSET
#define FMP_RES_TYPE_CS SDB_SPD_RES_TYPE_CS
#define FMP_RES_TYPE_CL SDB_SPD_RES_TYPE_CL
#define FMP_RES_TYPE_RG SDB_SPD_RES_TYPE_RG
#define FMP_RES_TYPE_RN SDB_SPD_RES_TYPE_RN

#define FMP_FUNC_TYPE_INVALID -1
#define FMP_FUNC_TYPE_JS 0
#define FMP_FUNC_TYPE_C 1
#define FMP_FUNC_TYPE_JAVA 2

#endif

