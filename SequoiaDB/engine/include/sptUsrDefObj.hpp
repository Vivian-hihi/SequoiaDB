/*******************************************************************************


   Copyright (C) 2011-2014 SequoiaDB Ltd.

   This program is free software: you can redistribute it and/or modify
   it under the term of the GNU Affero General Public License, version 3,
   as published by the Free Software Foundation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warrenty of
   MARCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU Affero General Public License for more details.

   You should have received a copy of the GNU Affero General Public License
   along with this program. If not, see <http://www.gnu.org/license/>.

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

