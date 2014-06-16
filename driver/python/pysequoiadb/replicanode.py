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

class replicanode(object):
    """Entrance of SequoiaDB


    """

    def __init__(self, client, node):
        self.client = client
        self.node = node

    def connect(self):
        rc = sdbreplicanode.connect(self.node, self.client)
        if const.SDB_OK != rc:
           raise error.OperationError("sdb error msg", rc)
        else:
            return rc

    def get_status(self):
        result = sdbreplicanode.get_status(self.node)
        return error.err_process(result)

    def get_hostname(self):
        result = sdbreplicanode.get_hostname(self.node)
        return error.err_process(result)

    def get_servicename(self):
        result = sdbreplicanode.get_servicename(self.node)
        return error.err_process(result)

    def get_nodename(self):
        result = sdbreplicanode.get_nodename(self.node)
        return error.err_process(result)

    def stop(self):
        ret = sdbreplicanode.stop(self.node)
        if const.SDB_OK != ret:
            raise error.OperationError("sdb error msg", ret)
        return ret

    def start(self):
        ret = sdbreplicanode.start(self.node)
        if 0 != ret:
            raise error.OperationError("sdb error msg", ret)
        return ret


