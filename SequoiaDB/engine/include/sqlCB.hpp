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

   Source File Name = sqlCB.hpp

   Descriptive Name =

   When/how to use: this program may be used on binary and text-formatted
   versions of PMD component. This file contains functions for agent processing.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          09/14/2012  YW  Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef SQLCB_HPP_
#define SQLCB_HPP_

#include "sqlGrammar.hpp"
#include "qgmBuilder.hpp"

namespace engine
{
   class _pmdEDUCB ;
   class _qgmPlanContainer ;
   class _rtnSQLFunc ;

   class _sqlCB : public SDBObject
   {
   public:
      _sqlCB() ;
      virtual ~_sqlCB() ;
   public:
      INT32 exec( const CHAR *sql, _pmdEDUCB *cb,
                  SINT64 &contextID ) ;

      INT32 getFunc( const CHAR *name,
                     UINT32 paramNum,
                     _rtnSQLFunc *&func ) ;

   private:
      INT32 _createContext( _qgmPlanContainer *container,
                            _pmdEDUCB *cb, SINT64 &contextID ) ;

   private:
      SQL_GRAMMAR _grammar ;
   } ;

   typedef class _sqlCB SQL_CB ;
}

#endif

