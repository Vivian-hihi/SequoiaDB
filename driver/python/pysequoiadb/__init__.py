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

   All operation need deal with the error code returned first. 
   ervry error code is not SDB_OK(or 0), it means something error has appeared,
   and user should deal with it according the meaning of error code printed.
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

version_tuple = (1, 8)

def get_version():
   if isinstance( version_tuple[-1], basestring):
      return '.'.join(map(str, version_tuple[:-1])) + version_tuple[-1]
   return '.'.join(map(str, version_tuple))

driver_version = get_version()
"""Current version of python driver for SequoiaDB."""


from pysequoiadb.client import client
from pysequoiadb.common import const
from pysequoiadb.error import ( SequoiaDBError,
                                InvalidParameter,
                                ConnectError,
                                OperationError )

import sys

PY3 = sys.version_info[0] == 3

def printInfo( what ):
   if PY3:
      print (what)
   else:
      print what
def check_error( rc ):
   if const.SDB_OK != rc:
      printInfo (OperationError("  Error: ", rc))