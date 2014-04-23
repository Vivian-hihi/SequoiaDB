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

   Source File Name = authCB.hpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          07/12/2012  YW  Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef AUTHCB_HPP_
#define AUTHCB_HPP_

#include "core.hpp"
#include "oss.hpp"
#include "../bson/bson.h"

using namespace bson ;

namespace engine
{
   class _pmdEDUCB ;

   class _authCB : public SDBObject
   {
   public:
      _authCB() ;
      ~_authCB() ;

   public:
      INT32 init( _pmdEDUCB *cb ) ;

      INT32 createUsr( BSONObj &obj,
                       _pmdEDUCB *cb ) ;

      INT32 removeUsr( BSONObj &obj,
                       _pmdEDUCB *cb ) ;

      INT32 authenticate( BSONObj &obj,
                          _pmdEDUCB *cb ) ;

      INT32 checkNeedAuth( _pmdEDUCB *cb, BOOLEAN forcecheck = FALSE ) ;

      BOOLEAN needAuthenticate()
      {
         return _needAuth ;
      }

   private:
      INT32 _initAuthentication( _pmdEDUCB *cb ) ;
      INT32 _createUsr( BSONObj &obj,
                        _pmdEDUCB *cb ) ;
      INT32 _valid( BSONObj &obj, BOOLEAN notEmpty ) ;
   private:
      BOOLEAN _needAuth ;
   } ;

   typedef class _authCB SDB_AUTHCB ;
}

#endif

