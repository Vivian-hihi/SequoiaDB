/*******************************************************************************
   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = clsFSDef.hpp

   Descriptive Name =

   When/how to use: this program may be used on binary and text-formatted
   versions of Replication component. This file contains structure for
   replication control block.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          09/14/2012  YW  Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef CLSFSDEF_HPP_
#define CLSFSDEF_HPP_

#include "core.hpp"

namespace engine
{
   #define CLS_FS_CS_NAME "cs"
   #define CLS_FS_COLLECTION_NAME "collection"
   #define CLS_FS_CS_META_NAME "csmeta"
   #define CLS_FS_PAGE_SIZE "pagesize"
   #define CLS_FS_KEYOBJ "keyobj"
   #define CLS_FS_END_KEYOBJ "endkeyobj"
   #define CLS_FS_NOMORE "nomore"
   #define CLS_FS_INDEXES "indexes"
   #define CLS_FS_INDEX "index"
   #define CLS_FS_FULLNAME "fullname"
   #define CLS_FS_FULLNAMES "fullnames"
   #define CLS_FS_CSNAME    "csname"
   #define CLS_FS_CSNAMES   "csnames"
   #define CLS_FS_NEEDDATA  "needdata"
   #define CLS_FS_ATTRIBUTES "attributes"

   enum CLS_FS_STATUS
   {
      CLS_FS_STATUS_BEGIN = 0,
      CLS_FS_STATUS_META,
      CLS_FS_STATUS_INDEX,
      CLS_FS_STATUS_NOTIFY_DOC,
      CLS_FS_STATUS_NOTIFY_LOG,
      CLS_FS_STATUS_END,
      CLS_FS_STATUS_NONE,
   } ;
}

#endif

