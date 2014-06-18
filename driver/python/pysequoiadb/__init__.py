"""Python Driver for SequoiaDB
   Copyright (C) SequoiaDB Inc. China
"""

default_host = "localhost"
default_port = 11810
default_user = ""
default_psw  = ""

static_object = None

version_tuple = (2, 7)

def get_version():
    if isinstance( version_tuple[-1], basestring):
        return '.'.join(map(str, version_tuple[:-1])) + version_tuple[-1]
    return '.'.join(map(str, version_tuple))

driver_version = get_version()