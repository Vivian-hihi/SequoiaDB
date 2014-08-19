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

"""Python Driver for SequoiaDB

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
from bson.son import SON

from pysequoiadb.client import client
from pysequoiadb.common import const
from pysequoiadb.error import ( SequoiaDBError,
                                InvalidParameter)

import sys
try:
   import sdbclient
except ImportError:
   raise Exception("cannot find C module file: sdbclient")

EMPTY_BSON = {}

def get_version():
   ver, sub_version, release, build = sdbclient.get_version()
   return ("( Version: %s , subVersion: %s , Release: %s , build: %s )"
            % (ver, sub_version, release, build))

PY3 = sys.version_info[0] == 3

driver_version = get_version()
"""Current version of python driver for SequoiaDB."""

def _print(what):
   if PY3:
      print(what)
   else:
      print what

def _raise_if_error(msg, rc):
   """Check return value, raise a SequoiaDBError if error occurred.
   """
   if msg is None:
      msg = "  Error code: "
   if const.SDB_OK != rc:
      raise OperationError(msg, rc)
   
def _getErr(rc):
   """Display error message of code specified

   """
   return OperationError(" ", rc)
