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

   Source File Name = rtnAlterFuncList.hpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          08/05/2015  YW  Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef RTN_ALTERFUNCLIST_HPP_
#define RTN_ALTERFUNCLIST_HPP_

#include "rtnAlterDef.hpp"
#include "ossLatch.hpp"
#include "ossUtil.hpp"
#include "msgDef.h"
#include <map>
#include <sstream>

namespace engine
{
   class _rtnAlterFuncList : public SDBObject
   {
   public:
      _rtnAlterFuncList() ; 
      ~_rtnAlterFuncList() ;

   private:
      typedef std::map<rtnAlterFuncKey, RTN_ALTER_FUNC> FUNC_LST ;

   public:
      INT32 init() ;

      INT32 getFunc( RTN_ALTER_TYPE type,
                     const CHAR *name,
                     RTN_ALTER_FUNC &func ) ;

   private:
      INT32 _init() ;

      INT32 _registerFunc( const rtnAlterFuncKey &key,
                           RTN_ALTER_FUNC func ) ;

   private:
      FUNC_LST _fl ;
      _ossSpinXLatch _latch ;
      BOOLEAN _inited ;
   } ;

}

#endif

