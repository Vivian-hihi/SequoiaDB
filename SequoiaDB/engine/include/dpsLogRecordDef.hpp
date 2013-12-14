/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = dpsLogRecordDef.hpp

   Descriptive Name =

   When/how to use: this program may be used on binary and text-formatted
   versions of DPS component. This file contains declare for dpsLogWrapper.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          11/27/2012  YW  Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef DPSLOGRECORDDEF_HPP_
#define DPSLOGRECORDDEF_HPP_

#include "dpsDef.hpp"

namespace engine
{
   enum DPS_LOG_PUBLIC
   {
      DPS_LOG_PUBLIC_INVALID = 0,
      DPS_LOG_PUBLIC_BEGIN = 200,
      DPS_LOG_PULIBC_FULLNAME = 201,
      DPS_LOG_PUBLIC_TRANSID = 202,
      DPS_LOG_PUBLIC_PRETRANS = 203,
   } ;

/// number in public can not be used in definition !

   enum DPS_LOG_INSERT
   {
      DPS_LOG_INSERT_OBJ = 1,
   } ;

   enum DPS_LOG_UPDATE
   {
      DPS_LOG_UPDATE_OLDMATCH =1,
      DPS_LOG_UPDATE_OLDOBJ = 2,
      DPS_LOG_UPDATE_NEWMATCH = 3,
      DPS_LOG_UPDATE_NEWOBJ = 4,
   } ;

   enum DPS_LOG_DELETE
   {
      DPS_LOG_DELETE_OLDOBJ = 1,
   } ;

   enum DPS_LOG_CSCRT
   {
      DPS_LOG_CSCRT_CSNAME = 1,
      DPS_LOG_CSCRT_PAGESIZE = 2,
   } ;

   enum DPS_LOG_CSDEL
   {
      DPS_LOG_CSDEL_CSNAME = 1,
   } ;

   enum DPS_LOG_CLCRT
   {
      DPS_LOG_CLCRT_ATTRIBUTE = 1,
   } ;

   enum DPS_LOG_CLDEL
   {
   } ;

   enum DPS_LOG_IXCRT
   {
      DPS_LOG_IXCRT_IX = 1,
   } ;

   enum DPS_LOG_IXDEL
   {
      DPS_LOG_IXDEL_IX = 1,
   } ;

   enum DPS_LOG_CLRENAME
   {
      DPS_LOG_CLRENAME_CSNAME =1,
      DPS_LOG_CLRENAME_CLOLDNAME =2,
      DPS_LOG_CLRENAME_CLNEWNAME =3,
   } ;

   enum DPS_LOG_CLTRUNC
   {
   } ;

   enum DPS_LOG_TS_COMMIT
   {
   } ;

   enum DPS_LOG_TS_ROLLBACK
   {
   } ;

   enum DPS_LOG_INVALIDCATA
   {
   } ;

   enum DPS_LOG_ROW
   {
      DPS_LOG_ROW_ROWDATA = 1,
   };
}


#endif

