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

"""
Module of client for python driver of SequoiaDB

"""
import socket
import random

try:
   import sdbclient
except ImportError:
   try:
      import libsdbclient
   except ImportError:
      raise Exception("cannot find C module file: sdbclient")

import bson
import pysequoiadb
from pysequoiadb.collectionspace import collectionspace
from pysequoiadb.collection import collection
from pysequoiadb.cursor import cursor
from pysequoiadb.replicagroup import replicagroup
from pysequoiadb.common import const
from pysequoiadb.error import (SequoiaDBError, InvalidParameter)


class client(object):
   """SequoiaDB Client Driver
   
   The client support interfaces to connect to SequoiaDB.
   In order to connect to SequoiaDB, you need use the class first.
   And you should make sure the instance of it released when you don't use it
   any more.

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
   HOST = "localhost"
   SERVICE = "11810"
   USER = ""
   PSW = ""

   def __init__(self, host = None, service = None, user = None, psw  = None):
      """initialize when product a object.
 
         it will try to connect to SequoiaDB using host and port given,
         localhost and 11810 are the default value of host and port,
         user and password are "". 

      Parameters:
         Name    Type      Info:
         host    str       The hostname or IP address of dbserver,
                                 if None, "localhost" will be insteaded
         service str/int   The service name or port number of dbserver,
                                 if None, "localhost" will be insteaded
         user    str       The user name to access to database,
                                 if None, "" will be insteaded
         psw     str       The user password to access to database,
                                 if None, "" will be insteaded
      Exceptions:
         pysequoiadb.error.InvalidParameter
         pysequoiadb.error.SequoiaDBError
      """
      self.__connected = False
      if host is None:
         self.__host = self.HOST
      elif isinstance(host, basestring):
         self.__host = host
      else:
         raise InvalidParameter("host must be an instance of basestring")

      if service is None:
         self.__service = self.SERVICE
      elif isinstance(service, int):
         self.__service = str(service)
      elif isinstance(service, basestring):
         self.__service = service
      else:
         raise InvalidParameter("service name must be an instance of int or basestring")

      if user is None:
         _user = self.USER
      elif isinstance(user, basestring):
         _user = user
      else:
         raise InvalidParameter("user name must be an instance of basestring")

      if psw is None:
         _psw = self.PSW
      elif isinstance(psw, basestring):
         _psw = psw
      else:
         raise InvalidParameter("password must be an instance of basestring")

      try:
         self._client = sdbclient.create_client()
      except SystemError:
         raise SequoiaDBError("Failed to alloc client", const.SDB_OOM)

      # try to connect with default user and password
      try:
         self.connect(self.__host, self.__service,
                           user = _user, password = _psw)
      except SequoiaDBError:
         raise

   def __del__(self):
      """release resource when del called.

      Exceptions:
         pysequoiadb.error.SequoiaDBError
      """
      self.__host = self.HOST
      self.__service = self.SERVICE
      if self._client is not None:
         try:
            rc = sdbclient.release_client(self._client)
            pysequoiadb._raise_if_error("Failed to release client", rc)
         except SequoiaDBError:
            raise
         self._client = None

   def __repr__(self):

      if self.__connected:
         return "Client, connect to: %s:%s" % (self.__host, self.__service)

   def __getitem__(self, name):
      """support [] to access to collection space.

         eg.
         cc = client()
         cs = cc['test'] # access to collection space named 'test'.

      Exceptions:
         pysequoiadb.error.SequoiaDBError
      """
      return self.__getattr__(name)

   def __getattr__(self, name):
      """support client.cs to access to collection space.

         eg.
         cc = client()
         cs = cc.test # access to collection space named 'test'

         and we should pass '__members__' and '__methods__',
         becasue dir(cc) will invoke __getattr__("__members__") and
         __getattr__("__methods__").

         if success, a collection object will be returned, or None.
      Exceptions:
         pysequoiadb.error.InvalidParameter
         pysequoiadb.error.SequoiaDBError
      """
      if '__members__' == name or '__methods__' == name:
         pass
      else:
         try:
            cs = collectionspace()
            rc = sdbclient.get_collection_space(self._client, name, cs._cs)
            pysequoiadb._raise_if_error("Failed to get collection space: %s" %
                                        name, rc)
         except SequoiaDBError:
            del cs;
            cs = None
            raise

         return cs

   def __get_local_ip(self, ifname = 'eth0'):

      import sys
      if sys.platform == 'win32':
         local = socket.gethostname()
         localip = socket.gethostbyname(local)
      else:
         import socket, fcntl, struct
         sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
         inet = fcntl.ioctl( sock.fileno(), 0x8915, 
                             struct.pack('256s', ifname[:15]))
         localip = socket.inet_ntoa(inet[20:24])

      return localip

   def connect_to_hosts(self, hosts, **kwargs):
      """try to connect a host in specified hosts

      Parameters:
         Name        Type  Info:
         hosts       list  The list contains hosts.
                                 eg.
                                 [ {'host':'localhost',     'service':'11810'},
                                    {'host':'192.168.10.30', 'service':'11810'},
                                    {'host':'192.168.20.63', 'service':11810}, ]
         **kwargs          Useful options are below:
         -  user     str         The user name to access to database.
         -  password str         The user password to access to database.
         -  policy   str         The policy of select hosts. it must be string
                                       of 'random' or 'one_by_one'.
      Exceptions:
         pysequoiadb.error.InvalidParameter
         pysequoiadb.error.SequoiaDBError
      """
      if not isinstance(hosts, list):
         raise InvalidParameter("hosts must be an instance of list")
      if "policy" in kwargs:
         policy = kwargs.get("policy")
      else:
         policy = "random"
      if not isinstance(policy, str):
         raise InvalidParameter("policy must be an instance of basestring")

      if len(hosts) == 0:
         raise InvalidParameter("hosts must hava at least 1 item",
                                     const.INVALIDARG)

      local = socket.gethostname()
      localip = self.__get_local_ip()
      if "user" in kwargs:
         if not isinstance(kwargs.get("user", str)):
            raise InvalidParameter("user name in kwargs must be \
                            an instance of basestring")
         _user = kwargs.get("user")
      else:
         _user = self.USER

      if "password" in kwargs:
         if not isinstance(kwargs.get("password", str)):
            raise InvalidParameter("password in kwargs must be \
                            an instance of basestring")
         _psw = kwargs.get("password")
      else:
         _psw = self.PSW

      # connect to localhost first
      for ip in hosts:
         if ("localhost" in ip.values() or
             local in ip.values() or
             localip in ip.values()):

            host = ip['host']
            svc = ip['service']
            if isinstance(host, basestring):
               self.__host = host
            else:
               raise InvalidParameter("policy must be an instance of basestring")

            if isinstance(svc, int):
               self.__service = str(svc)
            elif isinstance(svc, basestring):
               self.__service = svc
            else:
               raise InvalidParameter("policy must be an instance of int or basestring")

            try:
               self.connect(self.__host, self.__service,
                                 user = _user, password = _psw)
            except SequoiaDBError:
               continue

            pysequoiadb._print(self.__repr__())
            return

      # without local host in hosts, check policy
      size = len(hosts)
      if 0 == cmp("random", policy):
         position = random.randint(0, size - 1)
      elif 0 == cmp("one_by_one", policy):
         position = 0;
      else:
         raise InvalidParameter("policy must be 'random' or 'one_by_one'.")

      # try to connect to host one by one
      for index in range(size):
         ip = hosts[position]
         host = ip['host']
         svc = ip['service']

         if isinstance(host, basestring):
            self.__host = host
         else:
            raise InvalidParameter("policy must be an instance of basestring")

         if isinstance(svc, int):
            self.__service = str(svc)
         elif isinstance(svc, basestring):
            self.__service = svc
         else:
            raise InvalidParameter("policy must be an instance of int or str")

         try:
            self.connect(self.__host, self.__service,
                              user = _user, password = _psw)
         except SequoiaDBError:
            position += 1
            if position >= size:
               position %= size
            continue
         #with no error
         pysequoiadb._print(self.__repr__())
         return

      #raise a expection for failed to connect to any host
      raise SequoiaDBError("Failed to connect all specified hosts", rc)

   def connect(self, host, service, **kwargs):
      """connect to specified database

      Parameters:
         Name        Type     Info:
         host        str      The host name or IP address of database server.
         service     int/str  The servicename of database server.
         **kwargs             Useful options are below:
         -  user     str      The user name to access to database.
         -  password str      The user password to access to database.
      Exceptions:
         pysequoiadb.error.InvalidParameter
         pysequoiadb.error.SequoiaDBError
      """
      if isinstance(host, basestring):
         self.__host = host
      else:
         raise InvalidParameter("host must be an instance of basestring")

      if isinstance(service, int):
         self.__service = str(service)
      elif isinstance( service, basestring ):
         self.__service = service
      else:
         raise InvalidParameter("service name must be an instance of int or basestring")

      if "user" in kwargs:
         user = kwargs.get("user")
      else:
         user = self.USER
      if isinstance(user, basestring):
         _user = user
      else:
         raise InvalidParameter("user name must be an instance of basestring")
      
      if "password" in kwargs:
         psw = kwargs.get("password")
      else:
         psw = self.PSW
      if isinstance(psw, basestring):
         _psw = psw
      else:
         raise InvalidParameter("password must be an instance of basestring")

      try:
         rc = sdbclient.connect(self._client, self.__host, self.__service,
                                              _user, _psw)
         pysequoiadb._raise_if_error("Failed to connect to %s:%s" %
                                     (self.__host, self.__service), rc)
      except SequoiaDBError:
         raise

      # success to connect
      self.__connected = True

   def disconnect(self):
      """disconnect to current server.

      Exceptions:
         pysequoiadb.error.SequoiaDBError
      """
      try:
         rc = sdbclient.disconnect(self._client)
         pysequoiadb._raise_if_error("Failed to release object", rc)
      except SequoiaDBError:
         raise

      # success to disconnect
      self.__host = self.HOST
      self.__service = self.PSW
      self.__connected = False

   def create_user(self, name, psw):
      """Add an user in current database.

      Parameters:
         Name         Type     Info:
         name         str      The name of user to be created.
         psw          str      The password of user to be created.
      Exceptions:
         pysequoiadb.error.InvalidParameter
         pysequoiadb.error.SequoiaDBError
      """
      if not isinstance(name, basestring):
         raise InvalidParameter("user name must be an instance of basestring")

      if not isinstance(psw, basestring):
         raise InvalidParameter("password must be an instance of basestring")

      try:
         rc = sdbclient.create_user(self._client, name, psw)
         pysequoiadb._raise_if_error("Failed to create user", rc)
      except SequoiaDBError:
         raise

   def remove_user(self, name, psw):
      """Remove the spacified user from current database.

      Parameters:
         Name         Type     Info:
         user_name    str      The name of user to be removed.
         psw          str      The password of user to be removed.
      Exceptions:
         pysequoiadb.error.InvalidParameter
         pysequoiadb.error.SequoiaDBError
      """
      if not isinstance(user_name, basestring):
         raise InvalidParameter("user name must be an instance of basestring")

      if not isinstance(psw, basestring):
         raise InvalidParameter("password must be an instance of basestring")

      try:
         rc = sdbclient.remove_user(self._client, name, psw)
         pysequoiadb._raise_if_error("Failed to remove user", rc)
      except SequoiaDBError:
         raise

   def get_snapshot(self, snap_type, **kwargs):
      """Get the snapshots of specified type.

      Parameters:
         Name           Type  Info:
         snap_typr      str   The type of snapshot, see Info as below
         **kwargs             Useful options are below
         - condition    dict  The matching rule, match all the documents
                                    if not provided.
         - selector     dict  The selective rule, return the whole
                                    document if not provided.
         - order_by     dict  The ordered rule, result set is unordered
                                    if not provided.
      Return values:
         a cursor object of query
      Exceptions:
         pysequoiadb.error.InvalidParameter
         pysequoiadb.error.SequoiaDBError
      Info:
        snapshot type:
                  0     : Get all contexts' snapshot
                  1     : Get the current context's snapshot
                  2     : Get all sessions' snapshot
                  3     : Get the current session's snapshot
                  4     : Get the collections' snapshot
                  5     : Get the collection spaces' snapshot
                  6     : Get database's snapshot
                  7     : Get system's snapshot
                  8     : Get catalog's snapshot
      """
      if not isinstance(snap_type, int):
         raise InvalidParameter("snap type must be an instance of int")
      if snap_type < 0 or snap_type > 8:
         raise InvalidParameter("snap_type value is invalid")

      bson_condition = None
      bson_selector = None
      bson_order_by = None
      
      if "condition" in kwargs:
         if not isinstance(kwargs.get("condition"), dict):
            raise InvalidParameter("condition in kwargs must be an instance of dict")
         bson_condition = bson.BSON.encode(kwargs.get("condition"))
      if "selector" in kwargs:
         if not isinstance(kwargs.get("selector"), dict):
            raise InvalidParameter("selector in kwargs must be an instance of dict")
         bson_selector = bson.BSON.encode(kwargs.get("selector"))
      if "order_by" in kwargs:
         if not isinstance(kwargs.get("order_by"), dict):
            raise InvalidParameter("order_by in kwargs must be an instance of dict")
         bson_order_by = bson.BSON.encode(kwargs.get("order_by"))

      try:
         result = cursor()
         rc = sdbclient.get_snapshot(self._client, result._cursor, snap_type,
                                     bson_condition, bson_selector, bson_order_by)
         pysequoiadb._raise_if_error("Failed to get snapshot", rc)
      except SequoiaDBError:
         del result
         result = None
         raise

      return result

   def reset_snapshot(self, condition = None):
      """Reset the snapshot.

      Parameters:
         Name         Type     Info:
         condition    dict     The matching rule, usually specifies the
                                     node in sharding environment, in standalone
                                     mode, this option is ignored.
      Exceptions:
         pysequoiadb.error.InvalidParameter
         pysequoiadb.error.SequoiaDBError
      """
      bson_condition = None
      if condition is not None:
         if not isinstance(condition, dict):
            raise InvalidParameter("condition must be an instance of dict")
         bson_condition = bson.BSON.encode(condition)

      try:
         rc = sdbclient.reset_snapshot(self._client, bson_condition)
         pysequoiadb._raise_if_error("Failed to reset snapshot", rc)
      except SequoiaDBError:
         raise

   def get_list(self, list_type, **kwargs):
      """Get the informations of specified type.

      Parameters:
         Name        Type     Info:
         list_type   int      Type of list option, see Info as below.
         **kwargs             Useful options are below
         - condition dict     The matching rule, match all the documents
                                    if None.
         - selector  dict     The selective rule, return the whole
                                    documents if None.
         - order_by  dict     The ordered rule, never sort if None.
      Return values:
         a cursor object of query
      Exceptions:
         pysequoiadb.error.InvalidParameter
         pysequoiadb.error.SequoiaDBError
      Info:
         list type:
                0          : Get all contexts list
                1          : Get contexts list for the current session
                2          : Get all sessions list
                3          : Get the current session
                4          : Get all collections list
                5          : Get all collecion spaces' list
                6          : Get storage units list
                7          : Get replicaGroup list ( only applicable in sharding env )
                8          : Get store procedure list
                9          : Get domains list
                10         : Get tasks list
                11         : Get collection space list in domain
                12         : Get collection list in domain
      """
      if not isinstance(list_type, int):
         raise InvalidParameter("list type must be an instance of int")
      if list_type < 0 or list_type > 12:
         raise InvalidParameter("list type value %d is not defined" %
                                     list_type)

      bson_condition = None
      bson_selector = None
      bson_order_by = None

      if "condition" in kwargs:
         bson_condition = bson.BSON.encode(kwargs.get("condition"))
      if "selector" in kwargs:
         bson_selector = bson.BSON.encode(kwargs.get("selector"))
      if "order_by" in kwargs:
         bson_order_by = bson.BSON.encode(kwargs.get("order_by"))

      try:
         result = cursor()
         rc = sdbclient.get_list(self._client, result._cursor, list_type,
                                 bson_condition, bson_selector, bson_order_by)
         pysequoiadb._raise_if_error("Failed to get list", rc)
      except SequoiaDBError:
         del result
         result = None
         raise

      return result

   def get_collection(self, cl_full_name):
      """Get the specified collection.

      Parameters:
         Name         Type     Info:
         cl_full_name str      The full name of collection
      Return values:
         a collection object of query.
      Exceptions:
         pysequoiadb.error.InvalidParameter
         pysequoiadb.error.SequoiaDBError
      """
      if not isinstance(cl_full_name, basestring):
         raise InvalidParameter("full name of collection must be an instance of basestring")
      if '.' not in cl_full_name:
         raise InvalidParameter("Full name must included '.'")

      try:
         cl = collection()
         rc = sdbclient.get_collection(self._client, cl_full_name, cl._cl)
         pysequoiadb._raise_if_error("Failed to get collection", rc)
      except SequoiaDBError:
         del cl
         cl = None
         raise

      return cl

   def get_collection_space(self, cs_name):
      """Get the specified collection space.

      Parameters:
         Name         Type     Info:
         cs_name      str      The name of collection space.
      Return values:
         a collection space object of query.
      Exceptions:
         pysequoiadb.error.InvalidParameter
         pysequoiadb.error.SequoiaDBError
      """
      if not isinstance(cs_name, basestring):
         raise InvalidParameter("name of collection space must be an instance of basestring")

      try:
         cs = collectionspace()
         rc = sdbclient.get_collection_space(self._client, cs_name, cs._cs)
         pysequoiadb._raise_if_error("Failed to get collection space", rc)
      except SequoiaDBError:
         del cs
         cs = None
         raise

      return cs

   def create_collection_space(self, cs_name, page_size = 0):
      """Create collection space with specified pagesize.

      Parameters:
         Name         Type     Info:
         cs_name      str      The name of collection space to be created.
         page_size    int      The page size of collection space. See Info
                                     as below.
      Return values:
         collection space object created.
      Exceptions:
         pysequoiadb.error.InvalidParameter
         pysequoiadb.error.SequoiaDBError
      Info:
         valid page size value:
                           0  :  64k default page size
                        4096  :  4k
                        8192  :  8k
                       16384  :  16k
                       32768  :  32k
                       65536  :  64k
      """
      if not isinstance(cs_name, basestring):
         raise InvalidParameter("name of collection space must be an instance of basestring")
      if not isinstance(page_size, int):
         raise InvalidParameter("page size must be an instance of int")
      if page_size not in [0, 4096, 8192, 16384, 32768, 65536]:
         raise InvalidParameter("page size is invalid")

      try:
         cs = collectionspace()
         rc = sdbclient.create_collection_space(self._client, cs_name,
                                                page_size, cs._cs)
         pysequoiadb._raise_if_error("Failed to create collection space", rc)
      except SequoiaDBError:
         del cs
         cs = None
         raise

      return cs

   def drop_collection_space(self, cs_name):
      """Remove the specified collection space.

      Parameters:
         Name         Type     Info:
         cs_name      str      The name of collection space to be dropped
      Exceptions:
         pysequoiadb.error.InvalidParameter
         pysequoiadb.error.SequoiaDBError
      """
      if not isinstance(cs_name, basestring):
         raise InvalidParameter("name of collection space must be\
                         an instance of basestring")

      try:
         rc = sdbclient.drop_collection_space(self._client, cs_name)
         pysequoiadb._raise_if_error("Failed to drop collection space", rc)
      except SequoiaDBError:
         raise

   def list_collection_spaces(self):
      """List all collection space of current database, include temporary
         collection space.

      Return values:
         a cursor object of collection spaces.
      Exceptions:
         pysequoiadb.error.SequoiaDBError
      """
      try:
         result = cursor()
         rc = sdbclient.list_collection_spaces(self._client, result._cursor)
         pysequoiadb._raise_if_error("Failed to list collection spaces", rc)
      except SequoiaDBError:
         del result
         result = None
         raise

      return result

   def list_collections(self):
      """List all collections in current database.

      Return values:
         a cursor object of collection.
      Exceptions:
         pysequoiadb.error.SequoiaDBError
      """
      try:
         result = cursor()
         rc = sdbclient.list_collections(self._client, result._cursor)
         pysequoiadb._raise_if_error("Failed to list collections", rc)
      except SequoiaDBError:
         del result
         result = None
         raise

      return result

   def list_replica_groups(self):
      """List all replica groups of current database.

      Return values:
         a cursor object of replication groups.
      Exceptions:
         pysequoiadb.error.SequoiaDBError
      """
      try:
         result = cursor()
         rc = sdbclient.list_replica_groups(self._client, result._cursor)
         pysequoiadb._raise_if_error("Failed to list replica groups", rc)
      except SequoiaDBError:
         del result
         result = None
         raise

      return result

   def get_replica_group_by_name(self, group_name):
      """Get the specified replica group of specified group name.

      Parameters:
         Name         Type     Info:
         group_name   str      The name of replica group.
      Return values:
         the replicagroup object of query.
      Exceptions:
         pysequoiadb.error.InvalidParameter
         pysequoiadb.error.SequoiaDBError
      """
      if not isinstance(group_name, basestring):
         raise InvalidParameter("group name must be an instance of basestring")

      try:
         result = replicagroup(self._client)
         rc = sdbclient.get_replica_group_by_name(self._client, group_name,
                                                  result._group)
         pysequoiadb._raise_if_error("Failed to get specified group", rc)
      except SequoiaDBError:
         del result
         result = None
         raise

      return result

   def get_replica_group_by_id(self, id):
      """Get the specified replica group of specified group id.

      Parameters:
         Name       Type     Info:
         id         str      The id of replica group.
      Return values:
         the replicagroup object of query.
      Exceptions:
         pysequoiadb.error.InvalidParameter
         pysequoiadb.error.SequoiaDBError
      """
      if not isinstance(id, int):
         raise InvalidParameter("group id must be an instance of int")

      try:
         result = replicagroup(self._client)
         rc = sdbclient.get_replica_group_by_id(self._client, id, result._group)
         pysequoiadb._raise_if_error("Failed to get specified group", rc)
      except SequoiaDBError:
         del result
         result = None
         raise

      return result

   def create_replica_group(self, group_name):
      """Create the specified replica group.

      Parameters:
         Name        Type     Info:
         group_name  str      The name of replica group to be created.
      Return values:
         the replicagroup object created.
      Exceptions:
         pysequoiadb.error.InvalidParameter
         pysequoiadb.error.SequoiaDBError
      """
      if not isinstance(group_name, basestring):
         raise InvalidParameter("group name must be an instance of basestring")

      try:
         replica_group = replicagroup(self._client)
         rc = sdbclient.create_replica_group(self._client, group_name,
                                             replica_group._group)
         pysequoiadb._raise_if_error("Failed to create replica group", rc)
      except SequoiaDBError:
         del replica_group
         replica_group = None
         raise

      return replica_group

   def remove_replica_group(self, group_name):
      """Remove the specified replica group.

      Parameters:
         Name         Type     Info:
         group_name   str      The name of replica group to be removed
      Exceptions:
         pysequoiadb.error.InvalidParameter
         pysequoiadb.error.SequoiaDBError
      """
      if not isinstance(group_name, basestring):
         raise InvalidParameter("group name must be an instance of basestring")

      try:
         rc = sdbclient.remove_replica_group(self._client, group_name)
         pysequoiadb._raise_if_error("Failed to remove replica group", rc)
      except SequoiaDBError:
         raise

   def create_replica_cata_group(self, host, service, path, configure):
      """Create a catalog replica group.

      Parameters:
         Name         Type     Info:
         host         str      The hostname for the catalog replica group.
         service      str      The servicename for the catalog replica group.
         path         str      The path for the catalog replica group.
         configure    dict     The configurations for the catalog replica group.
      Exceptions:
         pysequoiadb.error.InvalidParameter
         pysequoiadb.error.SequoiaDBError
      """
      if not isinstance(host, basestring):
         raise InvalidParameter("host must be an instance of basestring")
      if not isinstance(service, basestring):
         raise InvalidParameter("service name must be an instance of basestring")
      if not isinstance(path, basestring):
         raise InvalidParameter("path must be an instance of basestring")
      if not isinstance(configure, dict):
         raise InvalidParameter("configure must be an instance of dict")

      bson_configure = bson.BSON.encode(configure)

      try:
         rc = sdbclient.create_replica_cata_group(self._client, host, service,
                                                  path, bson_configure)
         pysequoiadb._raise_if_error("Failed to create replica cate group", rc)
      except SequoiaDBError:
         raise

   def exec_update(self, sql):
      """Executing SQL command for updating.

      Parameters:
         Name         Type     Info:
         sql          str      The SQL command.
      Exceptions:
         pysequoiadb.error.InvalidParameter
         pysequoiadb.error.SequoiaDBError
      """
      if not isinstance(sql, basestring):
         raise InvalidParameter("update sql must be an instance of basestring")

      try:
         rc = sdbclient.exec_update(self._client, sql)
         pysequoiadb._raise_if_error("Failed to execute update sql", rc)
      except SequoiaDBError:
         raise

   def exec_sql(self, sql):
      """Executing SQL command.

      Parameters:
         Name         Type     Info:
         sql          str      The SQL command.
      Return values:
         a cursor object of matching documents.
      Exceptions:
         pysequoiadb.error.InvalidParameter
         pysequoiadb.error.SequoiaDBError
      """
      if not isinstance(sql, basestring):
         raise InvalidParameter("sql must be an instance of basestring")

      try:
         result = cursor()
         rc = sdbclient.exec_sql(self._client, sql, result._cursor)
         pysequoiadb._raise_if_error("Failed to execute sql command", rc)
      except SequoiaDBError:
         del result
         result = None
         raise

      return result

   def transaction_begin(self):
      """Transaction begin.

      Exceptions:
         pysequoiadb.error.SequoiaDBError
      """
      try:
         rc = sdbclient.transaction_begin(self._client)
         pysequoiadb._raise_if_error("Transaction error", rc)
      except SequoiaDBError:
         raise

   def transaction_commit(self):
      """Transaction commit.

      Exceptions:
         pysequoiadb.error.SequoiaDBError
      """
      try:
         rc = sdbclient.transaction_commit(self._client)
         pysequoiadb._raise_if_error("Transaction commit error", rc)
      except SequoiaDBError:
         raise

   def transaction_rollback(self):
      """Transaction rollback

      Exceptions:
         pysequoiadb.error.SequoiaDBError
      """
      try:
         rc = sdbclient.transaction_rollback(self._client)
         pysequoiadb._raise_if_error("Transaction rollback error", rc)
      except SequoiaDBError:
         raise

   def flush_configure(self, options):
      """Flush the options to configure file.
      Parameters:
         Name      Type  Info:
         options   dict  The configure infomation, pass {"Global":true} or
                               {"Global":false} In cluster environment, passing
                               {"Global":true} will flush data's and catalog's 
                               configuration file, while passing {"Global":false} will 
                               flush coord's configuration file. In stand-alone
                               environment, both them have the same behaviour.
      Exceptions:
         pysequoiadb.error.InvalidParameter
         pysequoiadb.error.SequoiaDBError
      """
      if not isinstance(options, dict):
         raise InvalidParameter("options must be an instance of dict")

      bson_options = bson.BSON.encode(options)
      try:
         rc = sdbclient.flush_configure(self._client, bson_options)
         pysequoiadb._raise_if_error("Failed to flush confige", rc)
      except SequoiaDBError:
         raise

   def create_procedure(self, code):
      """Create a store procedures

      Parameters:
         Name         Type     Info:
         code         str      The JS code of store procedures.
      Exceptions:
         pysequoiadb.error.InvalidParameter
         pysequoiadb.error.SequoiaDBError
      """
      if not isinstance(code, basestring):
         raise InvalidParameter("code must be an instance of basestring")
      try:
         rc = sdbclient.create_JS_procedure(self._client, code)
         pysequoiadb._raise_if_error("Failed to crate procedure", rc)
      except SequoiaDBError:
         raise

   def remove_procedure(self, name):
      """Remove a store procedures.
     
      Parameters:
         Name         Type     Info:
         name         str      The name of store procedure.
      Exceptions:
         pysequoiadb.error.InvalidParameter
         pysequoiadb.error.SequoiaDBError
      """
      if not isinstance(name, basestring):
         raise InvalidParameter("procedure name must be an instance of basestring")

      try:
         rc = sdbclient.remove_procedure(self._client, name)
         pysequoiadb._raise_if_error("Failed to remove procedure", rc)
      except SequoiaDBError:
         raise

   def list_procedures(self, condition):
      """List store procedures.

      Parameters:
         Name         Type     Info:
         condition    dict     The condition of list.
      Return values:
         an cursor object of result
      Exceptions:
         pysequoiadb.error.InvalidParameter
         pysequoiadb.error.SequoiaDBError
      """
      if not isinstance(condition, dict):
         raise InvalidParameter("condition must be an instance of dict")

      bson_condition = bson.BSON.encode(condition)
      try:
         result = cursor()
         rc = sdbclient.list_procedures(self._client, result._cursor,
                                        bson_condition)
         pysequoiadb._raise_if_error("Failed to list procedures", rc)
      except SequoiaDBError:
         del result
         result = None
         raise

      return result

   def eval_procedure(self, name):
      """Eval a func.
      
      Parameters:
         Name         Type     Info:
         name         str      The name of store procedure.
      Return values:
         cursor object of current eval.
      Exceptions:
         pysequoiadb.error.InvalidParameter
         pysequoiadb.error.SequoiaDBError
      """
      if not isinstance(name, basestring):
         raise InvalidParameter("code must be an instance of basestring")

      try:
         result = cursor()
         rc = sdbclient.eval_JS(self._client, result._cursor, name)
         pysequoiadb._raise_if_error("Failed to eval procedure", rc)
      except SequoiaDBError:
         del result
         result = None
         raise

      return result

   def backup_offline(self, options = None):
      """Backup the whole database or specifed replica group.

      Parameters:
         Name      Type  Info:
         options   dict  Contains a series of backup configuration
                               infomations. Backup the whole cluster if None. 
                               The "options" contains 5 options as below. 
                               All the elements in options are optional. 
                               eg:
                               { "GroupName":["rgName1", "rgName2"], 
                                 "Path":"/opt/sequoiadb/backup",
                                 "Name":"backupName", "Description":description,
                                 "EnsureInc":true, "OverWrite":true }
                               See Info as below.
      Exceptions:
         pysequoiadb.error.InvalidParameter
         pysequoiadb.error.SequoiaDBError
      Info:
         GroupName   :  The replica groups which to be backuped.
         Path        :  The backup path, if not assign, use the backup path assigned in configuration file.
         Name        :  The name for the backup.
         Description :  The description for the backup.
         EnsureInc   :  Whether excute increment synchronization,
                              default to be false.
         OverWrite   :  Whether overwrite the old backup file,
                              default to be false.
      """
      bson_options = None
      if options is not None:
         if not isinstance(options, dict):
            raise InvalidParameter("options must be an instance of dict")
         bson_options = bson.BSON.encode(options)

      try:
         rc = sdbclient.backup_offline(self._client, bson_options)
         pysequoiadb._raise_if_error("Failed to backup offline", rc)
      except SequoiaDBError:
         raise

   def list_backup(self, options, **kwargs):
      """List the backups.

      Parameters:
         Name        Type     Info:
         options     dict     Contains configuration infomations for remove
                                     backups, list all the backups in the
                                     default backup path if None. 
                                     The "options" contains 3 options as below. 
                                     All the elements in options are optional. 
                                     eg:
                                     { "GroupName":["rgame1", "rgName2"], 
                                       "Path":"/opt/sequoiadb/backup",
                                       "Name":"backupName" }
                                     See Info as below.
         **kwargs             Useful option arw below
         - condition dict     The matching rule, return all the documents
                                    if None.
         - selector  dict     The selective rule, return the whole document
                                    if None.
         - order_by  dict     The ordered rule, never sort if None.
      Return values:
         a cursor object of backup list
      Exceptions:
         pysequoiadb.error.InvalidParameter
         pysequoiadb.error.SequoiaDBError
      Info:
         GroupName   :  Assign the backups of specifed replica groups to be list.
         Path        :  Assign the backups in specifed path to be list,
                              if not assign, use the backup path asigned in the
                              configuration file.
         Name        :  Assign the backups with specifed name to be list.
      """

      bson_condition = None
      bson_selector = None
      bson_order_by = None

      if not isinstance(options, dict):
         raise InvalidParameter("options in kwargs must be an instance of dict")

      bson_options = bson.BSON.encode(options)
      if "condition" in kwargs:
         if not isinstance(kwargs.get("condition"), dict):
            raise InvalidParameter("condition in kwargs must be an instance of dict")
         bson_condition = bson.BSON.encode(kwargs.get("condition"))
      if "selector" in kwargs:
         if not isinstance(kwargs.get("selector"), dict):
            raise InvalidParameter("selector in kwargs must be an instance of dict")
         bson_selector = bson.BSON.encode(kwargs.get("selector"))
      if "order_by" in kwargs:
         if not isinstance(kwargs.get("order_by"), dict):
            raise InvalidParameter("order_by in kwargs must be an instance of dict")
         bson_order_by = bson.BSON.encode(kwargs.get("order_by"))

      try:
         result = cursor()
         rc = sdbclient.list_backup(self._client, result._cursor, bson_options,
                                   bson_condition, bson_selector, bson_order_by)
         pysequoiadb._raise_if_error("Failed to list backup", rc)
      except SequoiaDBError:
         del result
         result = None
         raise

      return result

   def remove_backup(self, options):
      """Remove the backups

      Parameters:
         Name      Type  Info:
         options   dict  Contains configuration infomations for remove
                               backups, remove all the backups in the default
                               backup path if null. The "options" contains 3
                               options as below. All the elements in options are
                               optional.
                               eg:
                               { "GroupName":["rgName1", "rgName2"],
                                 "Path":"/opt/sequoiadb/backup",
                                 "Name":"backupName" }
                               See Info as below.
      Return values:
         an cursor object of result
      Exceptions:
         pysequoiadb.error.InvalidParameter
         pysequoiadb.error.SequoiaDBError
      Info:
         GroupName   : Assign the backups of specifed replica groups to be
                             remove.
         Path        : Assign the backups in specifed path to be remove, if not
                             assign, use the backup path asigned in the configuration
                             file.
         Name        : Assign the backups with specifed name to be remove.
      """
      bson_options = None
      if not isinstance(options, dict):
         raise InvalidParameter("options must be an instance of dict")
      bson_options = bson.BSON.encode(options)

      try:
         rc = sdbclient.remove_backup(self._client, bson_options)
         pysequoiadb._raise_if_error("Failed to remove backup", rc)
      except SequoiaDBError:
         raise

   def list_task(self, **kwargs):
      """List the tasks.

      Parameters:
         Name           Type     Info:
         **kwargs                Useful options are below
         - condition    dict     The matching rule, return all the documents
                                       if None.
         - selector     dict     The selective rule, return the whole
                                       document if None.
         - order_by     dict     The ordered rule, never sort if None.
                                       bson.SON may need if it is order-sensitive.
                                       eg.
                                       bson.SON([("name",-1), ("age":1)]) it will
                                       be ordered descending by 'name' first, and
                                       be ordered ascending by 'age'
         - hint         dict     The hint, automatically match the optimal
                                       hint if None.
      Return values:
         a cursor object of task list
      Exceptions:
         pysequoiadb.error.InvalidParameter
         pysequoiadb.error.SequoiaDBError
      """
      bson_condition = None
      bson_selector = None
      bson_order_by = None
      bson_hint = None

      if "condition" in kwargs:
         if not isinstance(kwargs.get("condition", dict)):
            raise InvalidParameter("consition in kwargs must be an instance of dict")
         bson_condition = bson.BSON.encode(kwargs.get("condition"))
      if "selector" in kwargs:
         if not isinstance(kwargs.get("selector", dict)):
            raise InvalidParameter("selector in kwargs must be an instance of dict")
         bson_selector = bson.BSON.encode(kwargs.get("selector"))
      if "order_by" in kwargs:
         if not isinstance(kwargs.get("order_by", dict)):
            raise InvalidParameter("order_by in kwargs must be an instance of dict")
         bson_order_by = bson.BSON.encode(kwargs.get("order_by"))
      if "hint" in kwargs:
         if not isinstance(kwargs.get("hint", dict)):
            raise InvalidParameter("hint in kwargs must be an instance of dict")
         bson_hint = bson.BSON.encode(kwargs.get("hint"))

      try:
         result = cursor()
         rc = sdbclient.list_task(self._client, result._cursor, bson_condition,
                                  bson_selector, bson_order_by, bson_hint)
         pysequoiadb._raise_if_error("Failed to list tasks", rc)
      except SequoiaDBError:
         del result
         result = None
         raise

      return result

   def wait_task(self, task_ids, num):
      """Wait the tasks to finish.

      Parameters:
         Name         Type     Info:
         task_ids     list     The list of task id.
         num          int      The number of task id.
      Exceptions:
         pysequoiadb.error.InvalidParameter
         pysequoiadb.error.SequoiaDBError
      """
      if not isinstance(task_ids, list):
         raise InvalidParameter("task id must be an instance of list")
      if not isinstance(num, int):
         raise InvalidParameter("size of tasks must be an instance of int")

      try:
         rc = sdbclient.wait_task(self._client, task_ids, num)
         pysequoiadb._raise_if_error("Failed to wait task", rc)
      except SequoiaDBError:
         raise

   def cancel_task(self, task_id, is_async):
      """Cancel the specified task.

      Parameters:
         Name         Type     Info:
         task_id      long     The task id to be canceled.
         is_async     bool     The operation "cancel task" is async or not,
                                     "True" for async, "False" for sync.
      Exceptions:
         pysequoiadb.error.InvalidParameter
         pysequoiadb.error.SequoiaDBError
      """
      if not isinstance(task_id, long):
         raise InvalidParameter("task id must be an instance of list")

      async = 0
      if isinstance(is_async, bool):
         if is_async:
            async = 1
      else:
         raise InvalidParameter("size of tasks must be an instance of int")

      try:
         rc = sdbclient.cancel_task(self._client, task_id, async)
         pysequoiadb._raise_if_error("Failed to cancel task", rc)
      except SequoiaDBError:
         raise

   def set_session_attri(self, options = None):
      """Set the attributes of the session.

      Parameters:
         Name         Type     Info:
         options      dict     The configuration options for session.
      Exceptions:
         pysequoiadb.error.InvalidParameter
         pysequoiadb.error.SequoiaDBError
      """
      bson_options = None
      if options is not None:
         if not isinstance(options, dict):
            raise InvalidParameter("options must be an instance of dict")
         bson_options = bson.BSON.encode(options)

      try:
         rc = sdbclient.set_session_attri(self._client, bson_options)
         pysequoiadb._raise_if_error("Failed to set session attribute", rc)
      except SequoiaDBError:
         raise

   def close_all_cursors(self):
      """Close all the cursors in current thread, we can't use those cursors to 
         get data again.

      Exceptions:
         pysequoiadb.error.SequoiaDBError
      """
      try:
         rc = sdbclient.close_all_cursors(self._client)
         pysequoiadb._raise_if_error("Failed to close all cursors", rc)
      except SequoiaDBError:
         raise

   def is_valid(self):
      """Judge whether the connection is valid.

      Return values:
         bool 
      Exceptions:
         pysequoiadb.error.SequoiaDBError
      """
      try:
         rc, valid = sdbclient.is_valid(self._client)
         pysequoiadb._raise_if_error("connection is invalid", rc)
      except SequoiaDBError:
         valid = False
         raise

      return valid
