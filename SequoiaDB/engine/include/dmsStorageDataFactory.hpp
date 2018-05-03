/*******************************************************************************

   Copyright (C) 2011-2018 SequoiaDB Ltd.

   This program is free software: you can redistribute it and/or modify
   it under the term of the GNU Affero General Public License, version 3,
   as published by the Free Software Foundation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warrenty of
   MARCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU Affero General Public License for more details.

   You should have received a copy of the GNU Affero General Public License
   along with this program. If not, see <http://www.gnu.org/license/>.

   Source File Name = dmsStorageDataFactory.hpp

   Descriptive Name = dms Storage Data Object Factory.

   When/how to use: this program may be used on binary and text-formatted
   versions of Runtime component. This file contains structure for SU cache
   management ( including plan cache, statistics cache, etc. )

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          04/14/2017  YSD  Initial Draft

   Last Changed =

*******************************************************************************/
#ifndef DMSSTORAGE_DATAFACTORY_HPP__
#define DMSSTORAGE_DATAFACTORY_HPP__

#include "dmsStorageDataCommon.hpp"

namespace engine
{
   class _dmsStorageDataFactory : public SDBObject
   {
   public:
      _dmsStorageDataFactory() {}
      ~_dmsStorageDataFactory() {}

      dmsStorageDataCommon* createProduct( DMS_STORAGE_TYPE type,
                                           const CHAR *suFileName,
                                           dmsStorageInfo *info,
                                           _IDmsEventHolder *pEventHolder ) ;
   } ;
   typedef _dmsStorageDataFactory dmsStorageDataFactory ;

   dmsStorageDataFactory* getDMSStorageDataFactory() ;
}

#endif /* DMSSTORAGE_DATAFACTORY_HPP__ */

