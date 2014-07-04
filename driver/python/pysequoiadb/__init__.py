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

   @author : SequoiaDB Ltd
   @license: See Apache License, Version 2.0
   @see    : http://www.sequoiadb.com
   @version: 1.8

   @notice : The dict of built-in Python is hashed and non-ordered. so the
             element in dict may not the order we make it. we make a dict and
             print it like this:
             ...
             >>> a = {"avg_age":24, "major":"computer science"}
             >>> a
             >>> {'major': 'computer science', 'avg_age': 24}
             ...
             the elements order print is not we make it!!
             therefore, we use bson.SON to make the order-sensitive dict if the
             order is important such as operations in "$sort", "$group",
             "split_by_condition", "aggregate","create_collection"...
             In every scene which the order is important when make dict, please
             make it using bson.SON and list. It is a subclass of built-in dict
             and order-sensitive
"""

default_host = "localhost"
"""Default host."""

default_port = 11810
"""Default port."""

default_user = ""
"""Default user name."""

default_psw  = ""
"""Default password."""

static_object = None
"""Default bson"""

from pysequoiadb.client import client
from pysequoiadb.common import const
from pysequoiadb.error import ( SequoiaDBError,
                                ConnectError,
                                OperationError )

import sys
import sdbclient

def get_version():
   ver, sub_version, release, build = sdbclient.get_version()
   return { "Version"    : ver,
            "subVersion" : sub_version,
            "Release"    : release,
            "build"      : build }

PY3 = sys.version_info[0] == 3

# OPEN THE SWITCH WHEN DEBUG
_DEBUG = False

driver_version = get_version()
"""Current version of python driver for SequoiaDB."""

def cout(what):
   if PY3:
      print(what)
   else:
      print what

def ASSERT( condition ):
   """Check and make sure the parameter is always true.
   """
   if condition != True:
      cout("Error: SquoiaDB Assert Failed.")
      raise SequoiaDBError

def check_error(rc):
   """Check error occurred, and print error message if error occurred.

   """
   if const.SDB_OK != rc and _DEBUG:
      cout( OperationError("  Error code: ", rc) )

def getErr(rc):
   """Display error message of code specified

   """
   return OperationError(" ", rc)
