/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = spdFuncDownloader.hpp

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

#ifndef SPDFUNCDOWNLOADER_HPP_
#define SPDFUNCDOWNLOADER_HPP_

#include "core.hpp"
#include "../bson/bson.h"

using namespace bson ;

namespace engine
{
   class _pmdEDUCB ;

   class _spdFuncDownloader : public SDBObject
   {
   public:
      _spdFuncDownloader(){}
      virtual ~_spdFuncDownloader() {}

   public:
      virtual INT32 next( BSONObj &func ) = 0 ;

      virtual INT32 download( const BSONObj &match ) = 0 ;

   } ;
   typedef class _spdFuncDownloader spdFuncDownloader ;
}

#endif

