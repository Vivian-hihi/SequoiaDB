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
from pysequoiadb import replicanode
from pysequoiadb import common
from pysequoiadb import error
from pysequoiadb.error import InvalidParameter
from pysequoiadb.error import SequoiaDBError


class replicagroup(object):
   """Entrance of SequoiaDB

   """
   def __init__(self, client):
      self._group = sdbreplicagroup.create_replicagroup()
      if  self._group == None:
         raise SequoiaDBError()
      self._client = client

   def __del__(self):
      sdbreplicagroup.release_replicagroup(self._group)
      self._group = None
      self._client = None

   def get_nodenum(self, nodestatus):
      if nodestatus not in common.NODE_STATUS.available_options() :
         raise InvalidParameter("invalid node status")

      result = sdbreplicagroup.get_detail(self._group, nodestatus)
      return error.err_process(result)

   def get_detail(self):
      result = sdbreplicagroup.get_detail(self._group)
      bson_string = error.err_process(result)
      return bson._bson_to_dict(bson_string, dict, False, bson.OLD_UUID_SUBTYPE, True)


   def get_master(self):
      node = replicanode(self._client)
      ret = sdbreplicagroup.get_master(self._group, node._node)
      if common.SDB_OK != ret:
            raise error.OperationError("sdb error msg", ret)
      return node

   def get_slave(self):
      node = replicanode(self._client)
      ret = sdbreplicagroup.get_slave(self._group, node._node)
      if common.SDB_OK != ret:
         raise error.OperationError("sdb error msg", ret)
      return node

   def get_nodebyendpoint(self, hostname, servicename):
      node = replicanode(self._client)
      ret = sdbreplicagroup.get_nodebyendpoint(self._group, node._node,
                                                    hostname, servicename)
      if common.SDB_OK != ret:
         raise error.OperationError("sdb error msg", ret)
      return node

   def get_nodebyname(self,nodename):
      node = replicanode(self._client)
      ret = sdbreplicagroup.get_nodebyname(self._group, node._node, nodename)
      if common.SDB_OK != ret:
         raise error.OperationError("sdb error msg", ret)
      return node


   def create_node(self, hostname, servicename, dbpath, config):
      if types.DictType != type(config):
         raise InvalidParameter("invalid parameter type")
      rc = sdbreplicagroup.remove_node(self._group, hostname, servicename, dbpath, config)
      return rc

   def remove_node(self, hostname, servicename, config=None):
      if None != config:
         pybson = bson.BSON.encode(config)
         rc = sdbreplicagroup.remove_node(self._group, hostname, servicename, pybson)
      else:
         rc = sdbreplicagroup.remove_node(self._group, hostname, servicename)
      return rc

   def start(self):
      rc = sdbreplicagroup.start(self._group)
      return

   def stop(self):
      rc = sdbreplicagroup.stop(self._group)
      return rc

   def is_catalog(self):
      rc = sdbreplicagroup.is_catalog(self._group)
      return rc
