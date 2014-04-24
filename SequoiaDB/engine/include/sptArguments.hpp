/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = sptArguments.hpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          31/03/2014  YW  Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef SPT_ARGUMENTS_HPP_
#define SPT_ARGUMENTS_HPP_

#include "core.hpp"
#include "oss.hpp"
#include "../bson/bson.hpp"

namespace engine
{
   class _sptArguments : public SDBObject
   {
   public:
      _sptArguments() {}
      virtual ~_sptArguments(){}

   public:
      /// start with zero.
      virtual INT32 getNative( UINT32 pos, void *value ) const = 0 ;
      virtual INT32 getString( UINT32 pos, std::string &value ) const = 0 ;
      virtual INT32 getBsonobj( UINT32 pos, bson::BSONObj &value ) const = 0 ;
   } ;
   typedef class _sptArguments sptArguments ;
}

#endif

