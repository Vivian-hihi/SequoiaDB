#   Copyright (C) 2012-2014 SequoiaDB Ltd.
#
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.

try:
   import sdbcursor
except ImportError:
   raise Exception("cannot find C module file: sdbcursor")

import bson
import pysequoiadb
from pysequoiadb.common import const
from pysequoiadb.error import SequoiaDBError

class cursor(object):
   """Cursor of SequoiaDB


   """
   def __init__(self):

      self._cursor = None
      try:
         self._cursor = sdbcursor.create_cursor()
      except SystemError:
         pysequoiadb.check_error(const.SDM_OOM)
         raise SequoiaDBError

   def __del__(self):

      if self._cursor is not None:
         sdbcursor.release_cursor(self._cursor)
         self._cursor = None

   def next(self):
      """Return the next document of current cursor, and move forward.
       
      Parameters:
              Name         Type     Info:
         N/A
      Return values:
         Success: SDB_OK and an dict object of record
         Fail   : Others and None
      """
      result, bson_string = sdbcursor.next(self._cursor)
      if const.SDB_OK != result:
         record = None
      else:
         record,_ = bson._bson_to_dict(bson_string, dict, False, bson.OLD_UUID_SUBTYPE, True)
      return result,record

   def current(self):
      """Return the current document of cursor, and don't move.

      Parameters:
              Name         Type     Info:
         N/A
      Return values:
         Success: SDB_OK and an dict object of record
         Fail   : Others and None
      """
      result, bson_string = sdbcursor.current(self._cursor)
      if const.SDB_OK != result:
         record = None
      else:
         record,_ = bson._bson_to_dict(bson_string, dict, False, bson.OLD_UUID_SUBTYPE, True)
      return result,record

   def close(self):
      """Close the cursor's connection to database, we can't use this handle to
         get data again.

      Parameters:
              Name         Type     Info:
         N/A
      Return values:
         Success: SDB_OK
         Fail   : Others
      """

      rc = sdbcursor.close(self._cursor)
      pysequoiadb.check_error(rc)

      return rc

if __name__ == '__main__':
    from pysequoiadb.client import client
    from pysequoiadb.collectionspace import collectionspace
    from pysequoiadb.common import const
    sdb = client("192.168.20.111", 50000)
    rc = sdb.connect_by_host("192.168.20.111", 50000)
    if const.SDB_OK != rc:
        raise Exception("Test Failure")
    rc,cs = sdb.get_collection_space('tst')
    if -34 == rc:
        rc,cs = sdb.create_collection_space('tst')
        if const.SDB_OK != rc:
            raise Exception("Test Failure")
    if const.SDB_OK != rc:
       raise Exception("Test Failure")
    rc,cl = cs.get_collection('tst')
    if -23 == rc:
        rc, cl = cs.create_collection('tst')
        if const.SDB_OK != rc:
            raise Exception("Test Failure")
    for i in range(1000):
        rc, oid = cl.insert({'id':i})
        if const.SDB_OK != rc:
            raise Exception("Test Failure")
    import sdbcl
    print dir(sdbcl)
    print cl.get_collection_name()
    rc,cr = cl.query()
    if const.SDB_OK != rc:
        raise Exception("Test Failure")
    while const.SDB_OK ==rc and rc != const.SDB_DMS_EOC:
        rc,record = cr.next()
        print record
        rc,record = cr.current()
        print record

    if rc == const.SDB_DMS_EOC:
        cr.close()
