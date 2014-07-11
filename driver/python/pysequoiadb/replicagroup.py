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

"""Module of replicagroup for python driver of SequoiaDB
"""

try:
   import sdbreplicagroup
except ImportError:
   try:
      import libsdbreplicagroup
   except ImportError:
      raise Exception("cannot find C module file: sdbreplicagroup")

import bson
import types
import pysequoiadb

from pysequoiadb.replicanode import replicanode
from pysequoiadb.common import const
from pysequoiadb import common
from pysequoiadb.error import SequoiaDBError

class replicagroup(object):
   """Replica group of SequoiaDB

   All operation need deal with the error code returned first, if it has. 
   Every error code is not SDB_OK(or 0), it means something error has appeared,
   and user should deal with it according the meaning of error code printed.

   @version: execute to get version
             >>> import pysequoiadb
             >>> print pysequoiadb.get_version()

   @notice : The dict of built-in Python is hashed and non-ordered. so the
             element in dict may not the order we make it. we make a dict and
             print it like this:
             ...
             >>> a = {"avg_age":24, "major":"computer science"}
             >>> a
             >>> {'major': 'computer science', 'avg_age': 24}
             ...
             the elements order it is not we make it!!
             therefore, we use bson.SON to make the order-sensitive dict if the
             order is important such as operations in "$sort", "$group",
             "split_by_condition", "aggregate","create_collection"...
             In every scene which the order is important, please make it using
             bson.SON and list. It is a subclass of built-in dict
             and order-sensitive
   """
   def __init__(self, client):

      self._client = client
      try:
         self._group = sdbreplicagroup.create_replicagroup()
      except SystemError:
         pysequoiadb.check_error(const.SDM_OOM)
         raise SequoiaDBError

   def __del__(self):

      if self._group is not None:
         sdbreplicagroup.release_replicagroup(self._group)
         self._group = None
      self._client = None

   def get_nodenum(self, nodestatus):
      """Get the count of node with given status in current replica group.
      
      Parameters:
         Name         Type     Info:
         nodestatus   int      The specified status, see Info as below.
      Return values:
         Success: SDB_OK  and  the count of node
         Fail   : Others  and  None
      Info:
         flags : 0 or 1. 
             0 : count of all node
             1 : count of actived node
             2 : count of inactived node
             3 : count of unknown node
      """
      if not isinstance(nodestatus, int):
         raise TypeError("nodestatus be an instance of int")

      if nodestatus not in common.NODE_STATUS.available_options() :
         return const.INVALIDARG, None

      ret, nodenum = sdbreplicagroup.get_nodenum(self._group, nodestatus)
      pysequoiadb.check_error(ret)

      if  const.SDB_OK != ret:
          nodenum = None

      return ret, nodenum

   def get_detail(self):
      """Get the detail of the replica group.
      
      Parameters:
         Name         Type     Info:
         N/A
      Return values:
         Success: SDB_OK  and  a dict object of query
         Fail   : Others  and  None
      """
      ret, bson_string = sdbreplicagroup.get_detail(self._group)
      pysequoiadb.check_error(ret)

      if const.SDB_OK != ret:
          detail=None

      detail, _= bson._bson_to_dict(bson_string, dict, False, bson.OLD_UUID_SUBTYPE, True)
      return ret, detail

   def get_master(self):
      """Get the master node of the current replica group.
      
      Parameters:
         Name         Type     Info:
         N/A
      Return values:
         Success: SDB_OK  and  a replicanode object of query
         Fail   : Others  and  None
      """
      node = replicanode(self._client)
      ret = sdbreplicagroup.get_master(self._group, node._node)
      pysequoiadb.check_error(ret)

      if const.SDB_OK != ret:
         del node
         node = None

      return ret, node

   def get_slave(self):
      """Get one of slave node of the current replica group, if no slave exists
         then get master.
      
      Parameters:
         Name         Type     Info:
         N/A
      Return values:
         Success: SDB_OK  and  a replicanode object of query
         Fail   : Others  and  None
      """
      node = replicanode(self._client)
      ret = sdbreplicagroup.get_slave(self._group, node._node)
      pysequoiadb.check_error(ret)

      if const.SDB_OK != ret:
         del node
         node = None

      return ret, node

   def get_nodebyendpoint(self, hostname, servicename):
      """Get specified node from current replica group.
      
      Parameters:
         Name         Type     Info:
         hostname     str      The host name of the node.
         servicename  str      The service name of the node.
      Return values:
         Success: SDB_OK  and  a replicanode object of query
         Fail   : Others  and  None
      """
      if not isintance(hostname, basestring):
         raise TypeError("hostname must be an instance of basestring")
      if not isintance(servicename, basestring):
         raise TypeError("servicename must be an instance of basestring")

      node = replicanode(self._client)
      ret = sdbreplicagroup.get_nodebyendpoint(self._group, node._node,
                                                    hostname, servicename)
      pysequoiadb.check_error(ret)

      if const.SDB_OK != ret:
         del node
         node = None

      return ret, node

   def get_nodebyname(self,nodename):
      """Get specified node from current replica group.
      
      Parameters:
         Name         Type     Info:
         nodename     str      The host name of the node.
      Return values:
         Success: SDB_OK  and  a replicanode object of query
         Fail   : Others  and  None
      """
      if not isintance(nodename, basestring):
         raise TypeError("nodename must be an instance of basestring")

      node = replicanode(self._client)
      ret = sdbreplicagroup.get_nodebyname(self._group, node._node, nodename)
      pysequoiadb.check_error(ret)

      if const.SDB_OK != ret:
         del node
         node = None

      return ret, node

   def create_node(self, hostname, servicename, dbpath, config = None):
      """Create node in a given replica group.

      Parameters:
         Name         Type     Info:
         hostname     str      The host name for the node.
         servicename  str      The servicename for the node.
         dbpath       str      The database path for the node.
         config       dict     The configurations for the node.
      Return values:
         Success: SDB_OK
         Fail   : Others
      """
      if not isinstance(hostname, basestring):
         raise TypeError("host must be an instance of basestring")
      if not isinstance(servicename, basestring):
         raise TypeError("service name must be an instance of basestring")
      if not isinstance(dbpath, basestring):
         raise TypeError("path must be an instance of basestring")

      if config is None:
         config = {}

      if types.DictType != type(config):
         return const.INVALIDARG

      rc = sdbreplicagroup.create_node(self._group, hostname, servicename,
                                                    dbpath, config)
      pysequoiadb.check_error(rc)

      return rc

   def remove_node(self, hostname, servicename, config=None):
      """Remove node in a given replica group.
      
      Parameters:
         Name         Type     Info:
         hostname     str      The host name for the node.
         servicename  str      The servicename for the node.
         config       dict     The configurations for the node.
      Return values:
         Success: SDB_OK
         Fail   : Others
      """
      if not isinstance(hostname, basestring):
         raise TypeError("host must be an instance of basestring")
      if not isinstance(servicename, basestring):
         raise TypeError("service name must be an instance of basestring")

      if None != config:
         bson_config = bson.BSON.encode(config)
         rc = sdbreplicagroup.remove_node(self._group, hostname, servicename,
                                                                 bson_config)
      else:
         rc = sdbreplicagroup.remove_node(self._group, hostname, servicename)
      pysequoiadb.check_error(rc)

      return rc

   def start(self):
      """Start up current replica group.
      
      Parameters:
         Name         Type     Info:
         N/A
      Return values:
         Success: SDB_OK
         Fail   : Others
      """
      rc = sdbreplicagroup.start(self._group)
      pysequoiadb.check_error(rc)
      return rc

   def stop(self):
      """Stop current replica group.
      
      Parameters:
         Name         Type     Info:
         N/A
      Return values:
         Success: SDB_OK
         Fail   : Others
      """
      rc = sdbreplicagroup.stop(self._group)
      pysequoiadb.check_error(rc)
      return rc

   def is_catalog(self):
      """Test whether current replica group is catalog replica group.
      
      Parameters:
         Name         Type     Info:
         N/A
      Return values:
         Success: SDB_OK  and  True, if The replica group is catalog, or False
         Fail   : Others  and  False
      """
      rc = sdbreplicagroup.is_catalog(self._group)
      if (const.SDB_OK == rc):
          iscatalog = False
      elif (const.TRUE == rc):
          rc = const.SDB_OK
          iscatalog = True
      else:
          iscatalog = None

      pysequoiadb.check_error(rc)

      return rc, iscatalog