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
   try:
      import libsdbcursor
   except ImportError:
      raise Exception("cannot find C module file: sdbcursor")

import bson
import pysequoiadb
from pysequoiadb.common import const
from pysequoiadb.error import SequoiaDBError

class cursor(object):
   """Cursor of SequoiaDB

   All operation need deal with the error code returned first, if it has. 
   Every error code is not SDB_OK(or 0), it means something error has appeared,
   and user should deal with it according the meaning of error code printed.

   @version: execute to get version
             >>> import pysequoiadb
             >>> print pysequoiadb.get_version()

   @notice : The dict of built-in Python is hashed and non-ordered. so the
             element in dict may not the order we make it. we make a dict and
             print it like this:
             ...
             >>> a = {"avg_age":24, "major":"computer science"}
             >>> a
             >>> {'major': 'computer science', 'avg_age': 24}
             ...
             the elements order it is not we make it!!
             therefore, we use bson.SON to make the order-sensitive dict if the
             order is important such as operations in "$sort", "$group",
             "split_by_condition", "aggregate","create_collection"...
             In every scene which the order is important, please make it using
             bson.SON and list. It is a subclass of built-in dict
             and order-sensitive
   """
   def __init__(self):
      """constructor of cursor

      Exceptions:
         pysequoiadb.error.SequoiaDBError
      """
      self._cursor = None
      try:
         self._cursor = sdbcursor.create_cursor()
      except SystemError:
         raise SequoiaDBError("Failed to alloc cursor", const.SDB_OOM)

   def __del__(self):
      """release cursor

      Exceptions:
         pysequoiadb.error.SequoiaDBError
      """
      if self._cursor is not None:
         try:
            rc = sdbcursor.release_cursor(self._cursor)
            pysequoiadb._raise_if_error("Failed to release cursor", rc)
         except SequoiaDBError:
            raise
         self._cursor = None

   def next(self):
      """Return the next document of current cursor, and move forward.

      Return values:
         an dict object of record
      Exceptions:
         pysequoiadb.error.SequoiaDBError
      """
      try:
         rc, bson_string = sdbcursor.next(self._cursor)
         if const.SDB_OK != rc:
            if const.SDB_DMS_EOC == rc:
               record = None
            else:
               raise SequoiaDBError("Failed to get next record", rc)
         else:
            record, size = bson._bson_to_dict(bson_string, dict, False,
                                              bson.OLD_UUID_SUBTYPE, True)
      except SequoiaDBError:
         raise

      return rc, record

   def current(self):
      """Return the current document of cursor, and don't move.

      Return values:
         an dict object of record
      Exceptions:
         pysequoiadb.error.SequoiaDBError
      """
      try:
         rc, bson_string = sdbcursor.current(self._cursor)
         if const.SDB_OK != rc:
            if const.SDB_DMS_EOC == rc:
               record = None
            else:
               raise SequoiaDBError("Failed to get current record", rc)
         else:
            record, size = bson._bson_to_dict(bson_string, dict, False,
                                           bson.OLD_UUID_SUBTYPE, True)
      except SequoiaDBError:
         raise

      return rc, record

   def close(self):
      """Close the cursor's connection to database, we can't use this handle to
         get data again.

      Exceptions:
         pysequoiadb.error.SequoiaDBError
      """
      try:
         rc = sdbcursor.close(self._cursor)
         pysequoiadb._raise_if_error("Failed to close cursor", rc)
      except SequoiaDBError:
         raise