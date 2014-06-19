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

import sdbreplicagroup
import bson
import types

from pysequoiadb.replicanode import replicanode
from pysequoiadb.common import const
from pysequoiadb import common

class replicagroup(object):
   """Entrance of SequoiaDB

   """
   def __init__(self, client):
      self._group = sdbreplicagroup.create_replicagroup()
      self._client = client

   def __del__(self):
      if self._group is not None:
         sdbreplicagroup.release_replicagroup(self._group)
         self._group = None
      self._client = None

   def get_nodenum(self, nodestatus):
      """
       get node number  of group
       retcode SDB_OK Operation Success
       return node number after retcode
       retcode other Operation Failure
       return node number is Node
      """
      if nodestatus not in common.NODE_STATUS.available_options() :
         return const.INVALIDARG,None

      ret,nodenum = sdbreplicagroup.get_nodenum(self._group, nodestatus)
      if  const.SDB_OK != ret:
          nodenum = None
      return ret,nodenum

   def get_detail(self):
      """
       get detail of group
       retcode SDB_OK Operation Success
       return detail after retcode
       retcode other Operation Failure
       return detail is Node
      """
      ret,bson_string = sdbreplicagroup.get_detail(self._group)
      if const.SDB_OK != ret:
          detail=None
      detail,_= bson._bson_to_dict(bson_string, dict, False, bson.OLD_UUID_SUBTYPE, True)
      return ret,detail


   def get_master(self):
      """
       get master of group
       retcode SDB_OK Operation Success
       return node after retcode
       retcode other Operation Failure
       return node is Node
      """
      node = replicanode(self._client)
      ret = sdbreplicagroup.get_master(self._group, node._node)
      if const.SDB_OK != ret:
         node = None
      return ret,node

   def get_slave(self):
      """
       get slave of group
       retcode SDB_OK Operation Success
       return node after retcode
       retcode other Operation Failure
       return node is Node
      """
      node = replicanode(self._client)
      ret = sdbreplicagroup.get_slave(self._group, node._node)
      if const.SDB_OK != ret:
         node = None
      return ret,node

   def get_nodebyendpoint(self, hostname, servicename):
      """
       get node by endpoint of group
       retcode SDB_OK Operation Success
       return node after retcode
       retcode other Operation Failure
       return node is Node
      """
      node = replicanode(self._client)
      ret = sdbreplicagroup.get_nodebyendpoint(self._group, node._node,
                                                    hostname, servicename)
      if const.SDB_OK != ret:
          node = None
      return ret,node

   def get_nodebyname(self,nodename):
      """
       get node by name of group
       retcode SDB_OK Operation Success
       return node after retcode
       retcode other Operation Failure
       return node is Node
      """
      node = replicanode(self._client)
      ret = sdbreplicagroup.get_nodebyname(self._group, node._node, nodename)
      if const.SDB_OK != ret:
         node = None
      return ret,node


   def create_node(self, hostname, servicename, dbpath, config = None):
      if config is None:
         config = {}
      if types.DictType != type(config):
         return const.INVALIDARG
      rc = sdbreplicagroup.create_node(self._group, hostname, servicename, dbpath, config)
      return rc

   def remove_node(self, hostname, servicename, config=None):
      """
       remove node of group
       retcode SDB_OK Operation Success
       retcode other Operation Failure
      """
      if None != config:
         pybson = bson.BSON.encode(config)
         rc = sdbreplicagroup.remove_node(self._group, hostname, servicename, pybson)
      else:
         rc = sdbreplicagroup.remove_node(self._group, hostname, servicename)
      return rc

   def start(self):
      """
       start group
       retcode SDB_OK Operation Success
       retcode other Operation Failure
      """
      rc = sdbreplicagroup.start(self._group)
      return rc

   def stop(self):
      """
       stop group
       retcode SDB_OK Operation Success
       retcode other Operation Failure
      """
      rc = sdbreplicagroup.stop(self._group)
      return rc

   def is_catalog(self):
      """
       group is catalog group
       retcode SDB_OK Operation Success
         True is catalog group
         False is not catalog group
       retcode other Operation Failure
      """
      rc = sdbreplicagroup.is_catalog(self._group)
      if (const.SDB_OK == rc):
          iscatalog = False
      elif (const.TRUE == rc):
          rc = const.SDB_OK
          iscatalog = True
      else:
          iscatalog = None
      return rc,iscatalog


   if '__main__' == __name__:
    from pysequoiadb.client import client
    from pysequoiadb import common
    sdb = client("192.168.30.61")
    rc,group = sdb.creat_replica_group('newgroup')
    if -153 == rc:
        rc,group = sdb.get_replica_group_by_name("newgroup")
    if const.SDB_OK == rc:
        rc = group.create_node('r520-3','11840','/data/disk3/sequoiadb/database/data/11840',
                          {'numpagecleaners':'1','pagecleaninterval':'1000',})
    if -157 != rc:
        raise Exception("Test Failure")
    rc = group.start()
    if const.SDB_OK == rc:
        rc,node  = group.get_master()
        rc,nodename = node.get_nodename()
        if (nodename != 'r520-3:11840'):
            raise Exception("Test Failure")
    if const.SDB_OK == rc:
        rc,node = group.get_slave();
        rc,nodename = node.get_nodename()
        if (nodename != 'r520-3:11840'):
            raise Exception("Test Failure")
    if const.SDB_OK == rc:
        rc,detail = group.get_detail()
    #rc = group.get_nodenum(common.NODE_STATUS.ALL)
    if const.SDB_OK == rc:
       rc,iscatalog = group.is_catalog()
    if const.SDB_OK == rc:
        if iscatalog:
            raise Exception("Test Failure")

    rc = group.stop()
    if const.SDB_OK != rc:
        raise Exception("Test Failure")
    rc,nodenum = group.get_nodenum(common.NODE_STATUS.INACTIVE)
    if const.SDB_OK == rc:
        if 1 != nodenum:
            raise Exception("Test Failure")
    rc,node = group.get_nodebyendpoint("r520-3", '11840')
    if const.SDB_OK == rc:
        rc,node = group.get_nodebyname("r520-3:11840")
    if const.SDB_OK != rc:
        raise Exception("Test Failure")
    rc = group.remove_node("r520-3","11840")
    if -204 != rc:
        raise Exception("Test Failure")
    rc = sdb.remove_replica_group("newgroup")
    if const.SDB_OK != rc:
        raise  Exception("Test Failure")


