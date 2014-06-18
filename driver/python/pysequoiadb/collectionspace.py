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

"""Module of collectionspace for python driver for SequoiaDB
"""

import sdbcs
import bson
from pysequoiadb import ( static_object,
                          default_host,
                          default_port,
                          default_user,
                          default_psw,
                          driver_version )
from pysequoiadb import collection
from pysequoiadb import cursor
from pysequoiadb import error

class collectionspace(object):
    """CollectionSpace for SequoiaDB"""
    
    def __init__(self):
        """'cs' is short for collection space"""
        self._cs = sdbcs.create_cs()
        if self._cs is None:
            pass

    def __del__(self):
        rc = sdbcs.release_cs(self._cs)
        if rc:
            pass
        self._cs = None

    def __getitem__(self, item_name):
        cl = collection()
        rc = sdbcs.get_collection(self._cs, item_name, cl._cl)
        if rc:
            pass
        return cl

    def get_collection(self, cl_name):
        cl = collection()
        rc = sdbcs.get_collection(self._cs, cl_name, cl._cl)
        if rc:
            pass
        return cl

    def create_collection(self, cl_name, options = static_object):
        cl = collection()
        rc = sdbcs.get_collection(self._cs, cl_name, options, cl._cl)
        if rc:
            pass
        return cl

    def drop_collection(self, cl_name):
        rc = sdbcs.drop_collection(self._cs, cl_name)
        if rc:
            pass

    def get_collection_space_name(self):
        _, cs_name = sdbcs.get_collection_space_name(self._cs)
        if rc:
            pass
        return cs_name