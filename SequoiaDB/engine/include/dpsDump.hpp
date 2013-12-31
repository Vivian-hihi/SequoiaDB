/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = dpsDump.hpp

   Descriptive Name = Data Management Service Storage Unit Header

   When/how to use: this program may be used on binary and text-formatted
   versions of data management component. This file contains structure for
   DMS storage unit and its methods.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          31/12/2013  XJH Initial Draft

   Last Changed =

*******************************************************************************/
#ifndef DPSDUMP_HPP__
#define DPSDUMP_HPP__

#include "oss.hpp"
#include "core.hpp"

namespace engine
{

   /*
      _dpsDump define
   */
   class _dpsDump : public SDBObject
   {
      public:
         _dpsDump () {}
         ~_dpsDump () {}

      public:

         static UINT32 dumpLogFileHead ( CHAR *inBuf, UINT32 inSize,
                                         CHAR *outBuf, UINT32 outSize,
                                         UINT32 options ) ;

   } ;
   typedef _dpsDump dpsDump ;

}

#endif //DPSDUMP_HPP__

