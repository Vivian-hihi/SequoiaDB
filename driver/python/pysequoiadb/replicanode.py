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

"""Module of replicanode for python driver of SequoiaDB
"""

try:
   import sdbreplicanode
except ImportError:
   raise Exception("cannot find C module file: sdbreplicanode")

import pysequoiadb
from pysequoiadb.common import const
from pysequoiadb.error import SequoiaDBError

class replicanode(object):
   """Entrance of SequoiaDB

   """
   def __init__(self, client):

      self._client = client
      try:
         self._node = sdbreplicanode.create_node()
      except SystemError:
         pysequoiadb.check_error(const.SDB_OK)
         raise SequoiaDBError

   def __del__(self):

      if self._node is not None:
         sdbreplicanode.release_node(self._node)
         self._node = None
      self._client = None

   def connect(self):
      """Connect to the current node.
      
      Parameters:
              Name         Type     Info:
         N/A
      Return values:
         Success: SDB_OK
         Fail   : Others
      """
      ret = sdbreplicanode.connect(self._node, self._client)
      pysequoiadb.check_error(ret)
      return ret

   def get_status(self):
      """Get status of the current node
      
      Parameters:
              Name         Type     Info:
         N/A
      Return values:
         Success: SDB_OK  and  the status of node
         Fail   : Others  and  None
      """
      ret, nodestatus = sdbreplicanode.get_status(self._node)
      pysequoiadb.check_error(ret)

      if const.SDB_OK != ret:
          nodestatus = None

      return ret, nodestatus

   def get_hostname(self):
      """Get host name of the current node.
      
      Parameters:
              Name         Type     Info:
         N/A
      Return values:
         Success: SDB_OK  and  the name of host
         Fail   : Others  and  None
      """
      ret, hostname = sdbreplicanode.get_hostname(self._node)
      pysequoiadb.check_error(ret)

      if const.SDB_OK != ret:
         hostname = None

      return ret, hostname

   def get_servicename(self):
      """Get service name of the current node.

      Parameters:
              Name         Type     Info:
         N/A
      Return values:
         Success: SDB_OK  and  the name of service
         Fail   : Others  and  None
      """
      ret, servicename = sdbreplicanode.get_servicename(self._node)
      pysequoiadb.check_error(ret)

      if const.SDB_OK != ret:
         servicename = None

      return ret,servicename

   def get_nodename(self):
      """Get node name of the current node.

      Parameters:
              Name         Type     Info:
         N/A
      Return values:
         Success: SDB_OK  and  the name of node
         Fail   : Others  and  None
      """
      ret, nodename = sdbreplicanode.get_nodename(self._node)
      pysequoiadb.check_error(ret)

      if const.SDB_OK != ret:
         nodename = None

      return ret, nodename

   def stop(self):
      """Stop the node.
      
      Parameters:
              Name         Type     Info:
         N/A
      Return values:
         Success: SDB_OK
         Fail   : Others
      """
      ret = sdbreplicanode.stop(self._node)
      pysequoiadb.check_error(ret)
      return ret

   def start(self):
      """Start the node.
      
      Parameters:
              Name         Type     Info:
         N/A
      Return values:
         Success: SDB_OK
         Fail   : Others
      """
      ret = sdbreplicanode.start(self._node)
      pysequoiadb.check_error(ret)
      return ret


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
    if const.SDB_OK == rc:
       rc,node = group.get_nodebyendpoint("r520-3", '11840')
    if const.SDB_OK == rc:
        rc = node.start()
    if const.SDB_OK == rc:
        rc = node.stop()
    if const.SDB_OK == rc:
        rc,hostname = node.get_hostname()
    if const.SDB_OK == rc:
        if hostname != 'r520-3':
            raise Exception('Test failure')
    rc,servicename = node.get_servicename()
    if const.SDB_OK == rc:
        if servicename != '11840':
            raise Exception('Test failure')
    rc,status = node.get_status()
    if const.SDB_OK == rc:
        if status != common.NODE_STATUS.ACTIVE:
            raise Exception('Test failure')
    rc,nodename = node.get_nodename()
    if const.SDB_OK == rc:
        if status != common.NODE_STATUS.ACTIVE:
            raise Exception('Test failure')
    rc = node.stop()
    if const.SDB_OK == rc:
        raise Exception('Test failure')
    rc = group.remove_node("r520-3","11840")
    if -204 != rc:
        raise Exception("Test Failure")
    rc = sdb.remove_replica_group("newgroup")
    if const.SDB_OK != rc:
        raise  Exception("Test Failure")








