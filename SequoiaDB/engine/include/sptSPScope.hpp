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

   Source File Name = sptSPScope.hpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          31/03/2014  YW  Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef SPT_SPSCOPE_HPP_
#define SPT_SPSCOPE_HPP_

#include "sptScope.hpp"
#include "jsapi.h"

namespace engine
{
   class _sptSPScope : public _sptScope
   {
   public:
      _sptSPScope() ;
      virtual ~_sptSPScope() ;

   public:
      virtual INT32 start() ;

      virtual void shutdown() ;

   public:
      virtual INT32 eval(const CHAR *code, UINT32 len, bson::BSONObj &detail ) ;

   private:
      virtual INT32 _loadUsrDefObj( _sptObjDesc *desc ) ;

   private:
      JSRuntime *_runtime ;
      JSContext *_context ;
      JSObject *_global ;
      JSErrorReporter _errReporter ;
   } ;
   typedef class _sptSPScope sptSPScope ;
}

#endif

