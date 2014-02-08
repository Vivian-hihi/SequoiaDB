/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = rawbson2json.c

   Descriptive Name = Raw BSON to JSON ( or CSV )

   When/how to use: this program may be used on binary and text-formatted
   versions of UTIL component.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          09/14/2012  TW  Initial Draft

   Last Changed =

*******************************************************************************/
#include "rawbson2json.h"
#include "../client/jstobs.h"

// The caller pass the pointer for raw bson, and output buffer pointer and
// buffer length.
// the function returns true if bufferLen is good enough to hold data, otherwise
// it will return FALSE and the content in outputbuffer is not defined
BOOLEAN rawbson2json ( const CHAR *bsonObj,
                      CHAR *pOutputBuffer,
                      INT32 bufferLen )
{
   bson obj ;
   bson_init ( &obj ) ;
   bson_init_finished_data ( &obj, (char*)bsonObj ) ;
   return bsonToJson ( pOutputBuffer, bufferLen, &obj,
                       FALSE ) ;
}

BOOLEAN rawbson2csv ( const CHAR *bsonObj,
                      CHAR *pOutputBuffer,
                      INT32 bufferLen )
{
   bson obj ;
   bson_init ( &obj ) ;
   bson_init_finished_data ( &obj, (char*)bsonObj ) ;
   return bsonToJson ( pOutputBuffer, bufferLen, &obj,
                       TRUE ) ;
}
