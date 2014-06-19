"""Python Driver for SequoiaDB
   Copyright (C) SequoiaDB Inc. China
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

from pysequoiadb import client