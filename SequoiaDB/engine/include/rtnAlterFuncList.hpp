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
#include <map>

namespace engine
{
   class _rtnAlterFuncList : public SDBObject
   {
   public:
      _rtnAlterFuncList() ; 
      ~_rtnAlterFuncList() ;

   private:
      struct comp
      {
         BOOLEAN operator()( const CHAR *l,
                             const CHAR *r ) const
         {
            return ossStrcmp( l, r ) < 0 ;
         }
      } ;

      typedef std::map<const CHAR *, RTN_ALTER_FUNC, comp> FUNC_LST ;

      struct funcObj
      {
         RTN_ALTER_TYPE type ;
         FUNC_LST fl ;
      } ;

      typedef std::map<RTN_ALTER_TYPE, funcObj> ALL_FUNCS ;

      typedef std::map<const CHAR *, RTN_ALTER_TYPE, comp> ALL_TYPES ;

   public:
      INT32 init() ;

      INT32 getFunc( RTN_ALTER_TYPE type,
                     const CHAR *name,
                     RTN_ALTER_FUNC &func ) ;

      INT32 getObjType( const CHAR *name,
                        RTN_ALTER_TYPE &type ) ;

   private:
      INT32 _init() ;

   private:
      ALL_FUNCS _fl ;
      ALL_TYPES _tl ;
      _ossSpinXLatch _latch ;
      BOOLEAN _inited ;
   } ;

}

#endif

