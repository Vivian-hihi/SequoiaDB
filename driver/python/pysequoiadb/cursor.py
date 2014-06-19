"""
   Copyright (C) 2012-2014 SequoiaDB Ltd.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
"""

import sdbcursor
import bson
from pysequoiadb import error
from pysequoiadb.error import SequoiaDBError

class cursor(object):
   """Entrance of SequoiaDB


   """
   def __init__(self):
      self._cursor = sdbcursor.create_cursor()
      if  self._cursor == None:
         raise SequoiaDBError()

   def __del__(self):
      if self._cursor is not None:
         sdbcursor.release_cursor(self._cursor)
         self._cursor = None

   def next(self):
      result = sdbcursor.next(self._cursor)
      bson_string = error.err_process(result)
      return bson._bson_to_dict(bson_string, dict, False, bson.OLD_UUID_SUBTYPE, True)


   def current(self):
      result = sdbcursor.current(self._cursor)
      bson_string = error.err_process(result)
      return bson._bson_to_dict(bson_string, dict, False, bson.OLD_UUID_SUBTYPE, True)

   def close(self):
      rc = sdbcursor.close(self._cursor)
      return rc

