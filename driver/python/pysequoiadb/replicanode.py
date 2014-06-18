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

import sdbreplicanode
from pysequoiadb.common import const
from pysequoiadb import error
from pysequoiadb.error import PySequoiaDBError

class replicanode(object):
    """Entrance of SequoiaDB


    """
    def __init__(self, client):
        self._node = sdbreplicanode.create_node()
        if self._node == None:
            raise PySequoiaDBError()
        self._client = client

    def __del__(self):
        sdbreplicanode.release_node(self._node)
        self._group = None
        self._client = None

    def connect(self):
        rc = sdbreplicanode.connect(self._node, self._client)
        if const.SDB_OK != rc:
           raise error.OperationError("sdb error msg", rc)
        else:
            return rc

    def get_status(self):
        result = sdbreplicanode.get_status(self._node)
        return error.err_process(result)

    def get_hostname(self):
        result = sdbreplicanode.get_hostname(self._node)
        return error.err_process(result)

    def get_servicename(self):
        result = sdbreplicanode.get_servicename(self._node)
        return error.err_process(result)

    def get_nodename(self):
        result = sdbreplicanode.get_nodename(self._node)
        return error.err_process(result)

    def stop(self):
        ret = sdbreplicanode.stop(self._node)
        if const.SDB_OK != ret:
            raise error.OperationError("sdb error msg", ret)
        return ret

    def start(self):
        ret = sdbreplicanode.start(self._node)
        if 0 != ret:
            raise error.OperationError("sdb error msg", ret)
        return ret


