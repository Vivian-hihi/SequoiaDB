/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = sptContainer.hpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          31/03/2014  YW  Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef SPT_CONTAINER_HPP_
#define SPT_CONTAINER_HPP_

#include "core.hpp"
#include "oss.hpp"

namespace engine
{
   enum SPT_SCOPE_TYPE
   {
      SPT_SCOPE_TYPE_SP = 0,
      SPT_SCOPE_TYPE_V8 = 1,
   } ;

   class _sptScope ;

   class _sptContainer : public SDBObject
   {
   public:
      _sptContainer() ;
      virtual ~_sptContainer() ;

   public:
      _sptScope *newScope( SPT_SCOPE_TYPE type ) ;
   } ;

   typedef class _sptContainer sptContainer ;
}

#endif

