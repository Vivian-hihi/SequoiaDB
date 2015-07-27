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

   Source File Name = impSharding.hpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          7/7/2015  David Li  Initial Draft

   Last Changed =

*******************************************************************************/
#include "impSharding.hpp"
#include "msgDef.h"
#include "../client/client.h"
#include "pd.hpp"

namespace import
{
   Sharding::Sharding(const string& hostname,
                      const string& svcname,
                      const string& user,
                      const string& password,
                      const string& csname,
                      const string& clname,
                      BOOLEAN useSSL)
   : _hostname(hostname),
     _svcname(svcname),
     _user(user),
     _password(password),
     _csname(csname),
     _clname(clname),
     _useSSL(useSSL)
   {
      _inited = FALSE;
      _groupNum = 0;
   }

   Sharding::~Sharding()
   {
   }

   INT32 Sharding::init()
   {
      INT32 rc = SDB_OK;
      sdbConnectionHandle conn = SDB_INVALID_HANDLE;
      sdbCursorHandle cursor = SDB_INVALID_HANDLE;
      bson cond;
      bson result;
      string collectionName;

      SDB_ASSERT(!_inited, "alreay inited");

      bson_init(&cond);
      bson_init(&result);

      collectionName = _csname + "." + _clname;

      if (BSON_OK != bson_append_string(&cond, FIELD_NAME_NAME,
                                         collectionName.c_str()))
      {
         rc = SDB_SYS;
         PD_LOG(PDERROR, "failed to append string to bson");
         goto error;
      }

      if (BSON_OK != bson_finish(&cond))
      {
         rc = SDB_SYS;
         PD_LOG(PDERROR, "failed to finish bson");
         goto error;
      }

      if (_useSSL)
      {
         rc = sdbSecureConnect(_hostname.c_str(), _svcname.c_str(),
                               _user.c_str(), _password.c_str(), &conn);
      }
      else
      {
         rc = sdbConnect(_hostname.c_str(), _svcname.c_str(),
                         _user.c_str(), _password.c_str(), &conn);
      }

      if (SDB_OK != rc)
      {
         PD_LOG(PDERROR, "Failed to connect to database %s:%s, rc=%d, usessl=%d",
                _hostname.c_str(), _svcname.c_str(), rc, _useSSL);
      }

      rc = sdbGetSnapshot(conn, SDB_SNAP_CATALOG, &cond, NULL, NULL, &cursor);
      if (SDB_OK != rc)
      {
         if (SDB_RTN_COORD_ONLY == rc)
         {
            // may be standalone node or data node in replica,
            // so we just set _hostname to it
            PD_LOG(PDWARNING, "%s:%s is not coordinator",
                  _hostname.c_str(), _svcname.c_str());
            rc = SDB_OK;
            _inited = TRUE;
            _groupNum = 1;
            goto done;
         }

         PD_LOG(PDERROR, "failed to get coordinator group from %s:%s, rc = %d",
                _hostname.c_str(), _svcname.c_str(), rc);
         goto error;
      }

      rc = sdbNext(cursor, &result);
      if (SDB_OK != rc)
      {
         PD_LOG(PDERROR, "failed to get result from cursor, rc=%d", rc);
         goto error;
      }

      rc = _cataInfo.init(collectionName, bson_data(&result));
      if (SDB_OK != rc)
      {
         PD_LOG(PDERROR, "failed to init cata info, rc=%d", rc);
         goto error;
      }

      _groupNum = _cataInfo.getGroupNum();
      _inited = TRUE;

   done:
      bson_destroy(&cond);
      bson_destroy(&result);
      if (SDB_INVALID_HANDLE != cursor)
      {
         sdbCloseCursor(cursor);
         sdbReleaseCursor(cursor);
      }
      if (SDB_INVALID_HANDLE != conn)
      {
         sdbDisconnect(conn);
         sdbReleaseConnection(conn);
         conn = SDB_INVALID_HANDLE;
      }
      return rc;
   error:
      goto done;
   }

   INT32 Sharding::getGroupByRecord(bson* record, UINT32& groupId)
   {
      INT32 rc = SDB_OK;

      SDB_ASSERT(_inited, "must be inited");
      SDB_ASSERT(NULL != record, "record can't be NULL");

      if (_groupNum > 1)
      {
         rc = _cataInfo.getGroupByRecord(bson_data(record), groupId);
         if (SDB_OK != rc)
         {
            PD_LOG(PDERROR, "failed to get group by record, rc=%d", rc);
         }
      }
      else
      {
         groupId = 0;
      }

      return rc;
   }
}
