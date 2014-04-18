/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = sptUsrDefObj.hpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          31/03/2014  YW  Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef SPT_USRDEFOBJ_HPP_
#define SPT_USRDEFOBJ_HPP_

#include "sptDef.hpp"
#include "sptFuncInvoker.hpp"
#include "sptReturnVal.hpp"

namespace engine
{
   class _sptUsrDefObj : public SDBObject
   {
   public:
      virtual ~_sptUsrDefObj() {}

   protected:
      _sptUsrDefObj(){}

   public:
      virtual INT32 construct( _sptParamContainer &param,
                               bson::BSONObj &detail ) = 0 ;

      virtual INT32 destruct( bson::BSONObj &detail ) = 0 ;

      virtual std::string toString() const = 0 ;

      SPT_MEMBER_FUNC_DISPATCH_DECLARE ;
   public:
      OSS_INLINE _sptReturnVal &getRVal()
      {
         return _rval ;
      }

   protected:
      virtual SPT_FUNC getFuncFromParent( const CHAR *funcName )
      {
         return NULL ;
      }

   protected:
      _sptReturnVal _rval ;
   } ;

   typedef class _sptUsrDefObj sptUsrDefObj ;
}

#endif

