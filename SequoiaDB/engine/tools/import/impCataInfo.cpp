/*******************************************************************************

   Copyright (C) 2011-2015 SequoiaDB Ltd.

   This program is free software: you can redistribute it and/or modify
   it under the term of the GNU Affero General Public License, version 3,
   as published by the Free Software Foundation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warrenty of
   MARCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU Affero General Public License for more details.

   You should have received a copy of the GNU Affero General Public License
   along with this program. If not, see <http://www.gnu.org/license/>.

   Source File Name = impCataInfo.cpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          7/7/2015  David Li  Initial Draft

   Last Changed =

*******************************************************************************/
#include "impCataInfo.hpp"
#include "coordDef.hpp"

using namespace bson;

namespace import
{
   CataInfo::CataInfo()
   {
      _cataInfo = NULL;
   }

   CataInfo::~CataInfo()
   {
      SAFE_OSS_DELETE(_cataInfo);
   }

   INT32 CataInfo::init(const std::string& collectionName, const char* bsonData)
   {
      INT32 rc = SDB_OK;
      
      SDB_ASSERT(NULL == _cataInfo, "already inited");
      SDB_ASSERT(NULL != bsonData, "bsonData can't be NULL");

      _collectionName = collectionName;

      try
      {
         BSONObj obj(bsonData);

         _cataInfo = SDB_OSS_NEW
            engine::_CoordCataInfo(-1, _collectionName.c_str());
         if (NULL == _cataInfo)
         {
            rc = SDB_OOM;
            PD_LOG(PDERROR, "failed to alloc CoordCataInfo");
            goto error;
         }

         rc = _cataInfo->fromBSONObj(obj);
         if (SDB_OK != rc)
         {
            PD_LOG(PDERROR, "failed to init _cataInfo from bson");
            goto error;
         }
      }
      catch (std::exception &e)
      {
         rc = SDB_INVALIDARG;
         PD_LOG(PDERROR, "failed to init _cataInfo from bson,"
                "received unexcepted error:%s", e.what());
         goto error;
      }

   done:
      return rc;
   error:
      SAFE_OSS_DELETE(_cataInfo);
      goto done;
   }

   INT32 CataInfo::getGroupNum()
   {
      SDB_ASSERT(NULL != _cataInfo, "must be inited");

      return _cataInfo->getGroupNum();
   }

   INT32 CataInfo::getGroupByRecord(const char* bsonData, UINT32& groupId)
   {
      INT32 rc = SDB_OK;

      SDB_ASSERT(NULL != _cataInfo, "must be inited");
      SDB_ASSERT(NULL != bsonData, "bsonData can't be NULL");

      try
      {
         BSONObj obj(bsonData);

         rc = _cataInfo->getGroupByRecord(obj, groupId);
         if (SDB_OK != rc)
         {
            PD_LOG(PDERROR, "failed to get group by record, rc=%d", rc);
            goto error;
         }
      }
      catch (std::exception &e)
      {
         rc = SDB_INVALIDARG;
         PD_LOG(PDERROR, "failed to get group by record,"
                "received unexcepted error:%s", e.what());
         goto error;
      }

   done:
      return rc;
   error:
      goto done;
   }
}
