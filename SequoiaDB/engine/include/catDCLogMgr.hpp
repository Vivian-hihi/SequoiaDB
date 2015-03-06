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

   Source File Name = catDCLogMgr.hpp

   Descriptive Name =

   When/how to use: this program may be used on binary and text-formatted
   versions of runtime component. This file contains code logic for
   common functions for coordinator node.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================

   Last Changed =     XJH Opt

*******************************************************************************/
#ifndef CAT_DCLOGMGR_HPP__
#define CAT_DCLOGMGR_HPP__

#include "pmd.hpp"
#include "netDef.hpp"

using namespace bson ;

namespace engine
{
   class sdbCatalogueCB ;
   class _SDB_RTNCB ;
   class _dpsLogWrapper ;
   class _SDB_DMSCB ;


   /*
      _catDCLogMgr define
   */
   class _catDCLogMgr : public SDBObject
   {
      public:
         _catDCLogMgr() ;
         ~_catDCLogMgr() ;

      private:

   } ;
   typedef _catDCLogMgr catDCLogMgr ;


}

#endif // CAT_DCLOGMGR_HPP__

