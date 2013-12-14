/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = json2rawbson.h

   Descriptive Name = JSON To Raw BSON Header

   When/how to use: this program may be used on binary and text-formatted
   versions of UTIL component. This file contains declare of json2rawbson. Note
   this function should NEVER be directly called other than fromjson.cpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          12/21/2012  TW  Initial Draft

   Last Changed =

*******************************************************************************/
#ifndef JSON2RAWBSON_H__
#define JSON2RAWBSON_H__

#include "core.h"
SDB_EXTERN_C_START
// this function should NEVER be directly called other than fromjson.cpp
CHAR * json2rawcbson ( const CHAR *str ) ;
CHAR * json2rawbson ( const CHAR *str ) ;
SDB_EXTERN_C_END
#endif
