/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = sptParamContainer.hpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          31/03/2014  YW  Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef SPT_PARAMCONTAINER_HPP_
#define SPT_PARAMCONTAINER_HPP_

#include "core.hpp"
#include "oss.hpp"
#include "../bson/bson.hpp"

namespace engine
{
   class _sptParamContainer : public SDBObject
   {
   public:
      _sptParamContainer(){}
      virtual ~_sptParamContainer(){}

   public:
      virtual INT32 getINT32( UINT32 skip, BOOLEAN optional, INT32 &value ) = 0 ;
      virtual INT32 getFLOAT64( UINT32 skip, BOOLEAN optional, FLOAT64 &value ) = 0 ;
      virtual INT32 getString( UINT32 skip, BOOLEAN optional, std::string &value ) = 0 ;
      virtual INT32 getBSONObj( UINT32 skip, BOOLEAN optional, bson::BSONObj &value ) = 0 ;
      virtual INT32 getBOOLEAN( UINT32 skip, BOOLEAN optional, BOOLEAN &value ) = 0 ;
   } ;
   typedef class _sptParamContainer sptParamContainer ;
}

#endif

