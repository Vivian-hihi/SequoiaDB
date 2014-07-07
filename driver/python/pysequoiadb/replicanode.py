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
   """Replica Node of SequoiaDB

   All operation need deal with the error code returned first, if it has. 
   Every error code is not SDB_OK(or 0), it means something error has appeared,
   and user should deal with it according the meaning of error code printed.

   @author : SequoiaDB Ltd.
   @license: See Apache License, Version 2.0
   @see    : http://www.sequoiadb.com
   @version: 1.8

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