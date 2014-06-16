"""ReplicaNode interface of  SequoiaDB
"""

import sdbreplicagroup
import bson
import types
from pysequoiadb import replicanode

class replicagroup(object):
    """Entrance of SequoiaDB

    """
    node_status_all = 0
    node_status_active = 1
    node_status_inactive = 2
    node_status_unknown = 3

    def __init__(self, group, client):
        self.group = group
        self.client = client

    def err_process(self, result):
        if 2 != len(result):
            pass
        if 0 != result[0]:
            pass
        return result[1]

    def get_nodenum(self, nodestatus):
        if nodestatus < self.node_status_all or nodestatus >= self.node_status_unknown :
            pass

        result = sdbreplicagroup.get_detail(self.group, nodestatus)
        return self.err_process(result)

    def get_detail(self):
        result = sdbreplicagroup.get_detail(self.group)
        bson_string = self.err_process(result)
        return bson._bson_to_dict(bson_string, dict, False, bson.OLD_UUID_SUBTYPE, True)


    def get_master(self):
        result = sdbreplicagroup.get_master(self.group)
        node = self.err_process(result)
        return replicanode(self.client, node)

    def get_slave(self):
        result = sdbreplicagroup.get_slave(self.group)
        node = self.err_process(result)
        return replicanode(self.client, node)

    def get_nodebyendpoint(self, hostname, servicename):
        result = sdbreplicagroup.get_nodebyendpoint(self.group, hostname, servicename)
        node = self.err_process(result)
        return replicanode(self.client, node)

    def get_nodebyname(self,nodename):
        result = sdbreplicagroup.get_nodebyname(self.group, nodename)
        node = self.err_process(result)
        return replicanode(self.client, node)


    def create_node(self, hostname, servicename, dbpath, config):
        if types.DictType != type(config):
            pass
        rc = sdbreplicagroup.remove_node(self.group, hostname, servicename, dbpath, config)
        return rc


    def remove_node(self, hostname, servicename, config=None):
        if None != config:
            pybson = bson.BSON.encode(config)
            rc = sdbreplicagroup.remove_node(self.group, hostname, servicename, pybson)
        else:
            rc = sdbreplicagroup.remove_node(self.group, hostname, servicename)
        return rc

    def start(self):
        rc = sdbreplicagroup.start(self.group)
        return

    def stop(self):
        rc = sdbreplicagroup.stop(self.group)
        return rc

    def is_catalog(self):
        rc = sdbreplicagroup.is_catalog(self.group)
        return rc
