"""ReplicaNode interface of  SequoiaDB
"""

import sdbreplicanode

class replicanode(object):
    """Entrance of SequoiaDB


    """

    def __init__(self, client, node):
        self.client = client
        self.node = node

    def err_process(self, result):
        if 2 != len(result):
            pass
        if 0 != result[0]:
            pass
        return result[1]

    def connect(self):
        rc = sdbreplicanode.connect(self.node, self.client)
        if rc:
           pass

    def get_status(self):
        result = sdbreplicanode.get_status(self.node)
        return self.err_process(result)

    def get_hostname(self):
        result = sdbreplicanode.get_hostname(self.node)
        return self.err_process(result)

    def get_servicename(self):
        result = sdbreplicanode.get_servicename(self.node)
        self.err_process(result)

    def get_nodename(self):
        result = sdbreplicanode.get_nodename(self.node)
        self.err_process(result)

    def stop(self):
        ret = sdbreplicanode.stop(self.node)
        if 0 != ret:
            pass
        return ret

    def start(self):
        ret = sdbreplicanode.start(self.node)
        if 0 != ret:
            pass
        return ret


