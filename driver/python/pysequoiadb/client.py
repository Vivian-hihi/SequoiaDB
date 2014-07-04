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

"""Module of client for python driver of SequoiaDB
"""
import socket
import random

try:
   import sdbclient
except ImportError:
   raise Exception("cannot find C module file: sdbclient")

import bson
import pysequoiadb
from pysequoiadb import ( static_object,
                          default_host,
                          default_port,
                          default_user,
                          default_psw )
from pysequoiadb.collectionspace import collectionspace
from pysequoiadb.collection import collection
from pysequoiadb.cursor import cursor
from pysequoiadb.replicagroup import replicagroup
from pysequoiadb.common import const
from pysequoiadb.error import SequoiaDBError


class client(object):
   """SequoiaDB Client Driver
   
      The client support interfaces to connect to SequoiaDB.
      In order to connect to SequoiaDB, you need use the class first.
      And you should make sure the instance of it released when you don't use it
      any more.
   """
   def __init__(self, host = default_host, port = default_port,
                      user = default_user, psw  = default_psw):
      """initialize when product a object.
 
         it will try to connect to SequoiaDB using host and port given,
         localhost and 11810 are the default value of host and port,
         user and password are "". 
      """
      if isinstance(host, basestring):
         self.__host = host
      else:
         raise TypeError("host must be an instance of basestring")

      if isinstance(port, int):
         _port = port
      else:
         raise TypeError("port must be an instance of int")

      if isinstance(user, basestring):
         _user = user
      else:
         raise TypeError("user name must be an instance of basestring")

      if isinstance(psw, basestring):
         _psw = psw
      else:
         raise TypeError("password must be an instance of basestring")

      try:
         self._client = sdbclient.create_client()
      except SystemError:
         pysequoiadb.check_error(const.SDM_OOM)
         raise SequoiaDBError

      # try to connect with default user and password 
      rc = sdbclient.init_connect(self._client, self.__host,
                                                 _port, _user, _psw)
      if const.SDB_OK != rc:
         pysequoiadb.cout("Attempt to connect to host:[%s], port:[%d],\
                           user:[%s], password:[%s] failed."\
                           % (self.__host, _port, _user, _psw))
         pysequoiadb.cout("Error: %s", pysequoiadb.getErr(rc))
         sdbclient.disconnect(self._client)

   def __del__(self):
      """release resource when del called.

      """
      self.__host = default_host
      if self._client is not None:
         rc = sdbclient.release_client(self._client)
         pysequoiadb.check_error(rc)
         self._client = None

   def __repr__(self):

      return "Client, connect to: %s" % (self.__host)

   def __getitem__(self, name):
      """support [] to access to collection space.
      
         eg.
         cc = client()
         cs = cc['test'] # access to collection space named 'test'.
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
      """
      if '__members__' == name or '__methods__' == name:
         pass
      else:
         cs = collectionspace()
         rc = sdbclient.get_collection_space(self._client, name, cs._cs)
         pysequoiadb.check_error(rc)
         if const.SDB_OK != rc:
            del cs;
            cs = None

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

   def connect_to_hosts(self, hosts, user = default_user,
                              psw = default_psw, policy = "random"):
      """try to connect to hosts

      Parameters:
              Name    Type    Info:
         [in] hosts   list    The list contains hosts.
                              eg.
                              [ {'host':'localhost', 'port':11810,},
                                {'host':'192.168.10.30', 'port':11810},
                                {'host':'192.168.20.63', 'port':11810}, ]
         [in] user    str     The user name to access to database 
         [in] psw     str     The user password to access to database
         [in] policy  str     The policy of select hosts. it must be string of
                              'random' or 'one_by_one'
      Return values:
         Success: SDB_OK  and  the index the hosts connect to
         Fail   : Others  and  -1
      """

      if not isinstance(hosts, list):
         raise TypeError("hosts must be an instance of list")
      if not isinstance(policy, str):
         raise TypeError("policy must be an instance of str")

      if len(hosts) == 0:
         raise TypeError("hosts must hava at least 1 item")

      local = socket.gethostname()
      localip = self.__get_local_ip()
      _user = user
      _psw  = psw

      count = 0
      for ip in hosts:
         if ( "localhost" in ip.values() or
              local in ip.values() or
              localip in ip.values() ):

            host = ip['host']
            port = ip['port']
            rc = self.connect_by_host(host, port, user, psw)
            if const.SDB_OK == rc:
               pysequoiadb.cout("connect to host:[%s], port:[%d] success."\
                                 % (host, port))
            return rc, count
         count += 1

      size = len(hosts)
      if 0 == cmp("random", policy):
         position = random.randint(0, size - 1)
      elif 0 == cmp("one_by_one", policy):
         position = 0;
      else:
         raise TypeError("policy must be 'random' or 'one_by_one'.")

      count = 0
      while count < size:
         ip = hosts[position]
         host = ip['host']
         port = ip['port']

         rc = self.connect_by_host(host, port, _user, _psw)
         if const.SDB_OK == rc:
            pysequoiadb.cout("connect to host:[%s], port:[%d] success."\
                              % (host, port))
            return rc, position
         position += 1

      return rc, const.INVALIDARG
   
   def connect_by_host(self, host = default_host, port = default_port,
                             user = default_user, psw  = default_psw):
      """connect to specified database

      Parameters:
              Name    Type    Info:
         [in] host    str     The host name or IP address of database server,
                              if None, 'localhost' instead.
         [in] port    int     The port of database server, if None, 11810
                              instead.
         [in] user    str     The user name to access to database, if None,
                              "" instead.
         [in] psw     str     The password to access to database, if None,
                              "" instead.
      Return values:
         Success: SDB_OK
         Fail   : Others
      """
      if isinstance(host, basestring):
         self.__host = host
      else:
         raise TypeError("host must be an instance of basestring")

      if isinstance(port, int):
         _port = port
      else:
         raise TypeError("port must be an instance of int")

      if isinstance(user, basestring):
         _user = user
      else:
         raise TypeError("user name must be an instance of basestring")

      if isinstance(psw, basestring):
         _psw = psw
      else:
         raise TypeError("password must be an instance of basestring")

      rc = sdbclient.connect_by_host(self._client, self.__host,
                                                    _port, _user, _psw)
      pysequoiadb.check_error(rc)

      return rc

   def connect_by_service(self, host, service, user = default_user,
                                               psw  = default_psw):
      """connect to specified database, using host and service name.

      Parameters:
              Name    Type    Info:
         [in] host    str     The host name or IP address of database server.
         [in] service str     The service name of database server.
         [in] user    str     The user name to access to database, if None,
                              "" instead.
         [in] psw     str     The password to access to database, if None,
                              "" instead.
      Return values:
         Success: SDB_OK
         Fail   : Others
      """
      if isinstance(host, basestring):
         self.__host = host
      else:
         raise TypeError("host must be an instance of basestring")

      if isinstance(service, basestring):
         _service = service
      else:
         raise TypeError("port must be an instance of basestring")

      if isinstance(user, basestring):
         _user = user
      else:
         raise TypeError("user name must be an instance of basestring")

      if isinstance(psw, basestring):
         _psw = psw
      else:
         raise TypeError("password must be an instance of basestring")

      rc = sdbclient.connect_by_service(self._client, self.__host,
                                                       _service, _user, _psw)
      pysequoiadb.check_error(rc)

      return rc

   def disconnect(self):
      """disconnect to current server.

      Parameters:
              Name    Type    Info:
         N/A
      Return values:
         Success: SDB_OK
         Fail   : Others
      """
      self.__host = default_host
      rc = sdbclient.disconnect(self._client)
      pysequoiadb.check_error(rc)

      return rc

   def create_user(self, user_name, psw):
      """Add an user in current database.

      Parameters:
              Name         Type     Info:
         [in] user_name    str      The name of user to be created.
         [in] psw          str      The password of user to be created.
      Return values:
         Success: SDB_OK
         Fail   : Others
      """
      if isinstance(user_name, basestring):
         _user_name = user_name
      else:
         raise TypeError("user name must be an instance of basestring")

      if isinstance(psw, basestring):
         _psw = psw
      else:
         raise TypeError("password must be an instance of basestring")

      rc = sdbclient.create_user(self._client, _user_name, _psw)
      pysequoiadb.check_error(rc)

      return rc

   def remove_user(self, user_name, psw):
      """Remove the spacified user from current database.

      Parameters:
              Name         Type     Info:
         [in] user_name    str      The name of user to be removed.
         [in] psw          str      The password of user to be removed.
      Return values:
         Success: SDB_OK
         Fail   : Others
      """
      if isinstance(user_name, basestring):
         _user_name = user_name
      else:
         raise TypeError("user name must be an instance of basestring")

      if isinstance(psw, basestring):
         _psw = psw
      else:
         raise TypeError("password must be an instance of basestring")

      rc = sdbclient.remove_user(self._client, _user_name, _psw)
      pysequoiadb.check_error(rc)

      return rc

   def get_snapshot(self, snap_type, condition = static_object,
                                     selector  = static_object,
                                     order_by  = static_object):
      """Get the snapshots of specified type.

      Parameters:
              Name         Type  Info:
         [in] snap_typr    str   The type of snapshot, see Info as below
         [in] condition    dict  The matching rule, match all the documents
                                 if not provided.
         [in] selector     dict  The selective rule, return the whole
                                 document if not provided.
         [in] order_by     dict  The ordered rule, result set is unordered
                                 if not provided.
      Return values:
         Success: SDB_OK       and  a cursor object of query
         Fail   : Others  and  None
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
         raise TypeError("snap type must be an instance of int")

      bson_condition = None
      bson_selector = None
      bson_order_by = None
      
      if condition is not None:
         bson_condition = bson.BSON.encode(condition)
      if selector is not None:
         bson_selector = bson.BSON.encode(selector)
      if order_by is not None:
         bson_order_by = bson.BSON.encode(order_by)

      result = cursor()
      rc = sdbclient.get_snapshot(self._client, result._cursor, snap_type,
                                  bson_condition, bson_selector, bson_order_by)
      pysequoiadb.check_error(rc)
      if const.SDB_OK != rc:
         del result
         result = None

      return rc, result

   def reset_snapshot(self, condition = static_object):
      """Reset the snapshot.

      Parameters:
              Name         Type     Info:
         [in] condition    dict     The matching rule, usually specifies the
                                    node in sharding environment, in standalone
                                    mode, this option is ignored.
      Return values:
         Success: SDB_OK
         Fail   : Other
      """
      bson_condition = None
      if condition is not None:
         bson_condition = bson.BSON.encode(condition)

      rc = sdbclient.reset_snapshot(self._client, bson_condition)
      pysequoiadb.check_error(rc)

      return rc

   def get_list(self, list_type, condition = static_object,
                                 selector  = static_object,
                                 order_by  = static_object):
      """Get the informations of specified type.

      Parameters:
              Name         Type     Info:
         [in] list_type    int      type of list, see Info as below.
         [in] condition    dict     The matching rule, match all the documents
                                    if None.
         [in] selector     dict     The selective rule, return the whole
                                    documents if None.
         [in] order_by     dict     The ordered rule, never sort if None.
      Return values:
         Success: SDB_OK   and   a cursor object of query
         Fail   : Other    and   None
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
         raise TypeError("list type must be an instance of int")

      bson_condition = None
      bson_selector = None
      bson_order_by = None

      if condition is not None:
         bson_condition = bson.BSON.encode(condition)
      if selector is not None:
         bson_selector = bson.BSON.encode(selector)
      if order_by is not None:
         bson_order_by = bson.BSON.encode(order_by)

      result = cursor()
      rc = sdbclient.get_list(self._client, result._cursor, list_type,
                              bson_condition, bson_selector, bson_order_by)
      pysequoiadb.check_error(rc)

      if const.SDB_OK != rc:
         del result
         result = None

      return rc, result

   def get_collection(self, cl_full_name):
      """Get the specified collection.

      Parameters:
              Name         Type     Info:
         [in] cl_full_name str      The full name of collection
      Return values:
         Success: SDB_OK   and   a collection object of query.
         Fail   : Other    and   None
      """
      if not isinstance(cl_full_name, basestring):
         raise TypeError("full name of collection must be an instance of basestring")

      cl = collection()
      rc = sdbclient.get_collection(self._client, cl_full_name, cl._cl)
      pysequoiadb.check_error(rc)

      if const.SDB_OK != rc:
         del cl
         cl = None

      return rc, cl

   def get_collection_space(self, cs_name):
      """Get the specified collection space.

      Parameters:
              Name         Type     Info:
         [in] cs_name      str      The name of collection space.
      Return values:
         Success: SDB_OK   and   a collection space object of query.
         Fail   : Other    and   None
      """
      if not isinstance(cs_name, basestring):
         raise TypeError("name of collection space must be an instance of basestring")

      cs = collectionspace()
      rc = sdbclient.get_collection_space(self._client, cs_name, cs._cs)
      pysequoiadb.check_error(rc)

      if const.SDB_OK != rc:
         del cs
         cs = None

      return rc, cs

   def create_collection_space(self, cs_name, page_size = 0):
      """Create collection space with specified pagesize.

      Parameters:
              Name         Type     Info:
         [in] cs_name      str      The name of collection space to be created.
         [in] page_size    int      The page size of collection space. See Info
                                    as below.
      Return values:
         Success: SDB_OK   and   the collection space object created.
         Fail   : Other    and   None
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
         raise TypeError("name of collection space must be an instance of basestring")
      if not isinstance(page_size, int):
         raise TypeError("page size must be an instance of int")
      
      cs = collectionspace()
      rc = sdbclient.create_collection_space(self._client, cs_name,
                                             page_size, cs._cs)
      pysequoiadb.check_error(rc)

      if const.SDB_OK != rc:
         del cs
         cs = None

      return rc, cs

   def drop_collection_space(self, cs_name):
      """Remove the specified collection space.

      Parameters:
              Name         Type     Info:
         [in] cs_name      str      The name of collection space to be dropped
      Return values:
         Success: SDB_OK
         Fail   : Other
      """
      if not isinstance(cs_name, basestring):
         raise TypeError("name of collection space must be an instance of basestring")

      rc = sdbclient.drop_collection_space(self._client, cs_name)
      pysequoiadb.check_error(rc)

      return rc

   def list_collection_spaces(self):
      """List all collection space of current database, include temporary
         collection space.

      Parameters:
              Name         Type     Info:
         N/A
      Return values:
         Success: SDB_OK   and   a cursor object of collection space list.
         Fail   : Other    and   None
      """
      result = cursor()
      rc = sdbclient.list_collection_spaces(self._client, result._cursor)
      pysequoiadb.check_error(rc)

      if const.SDB_OK != rc:
         del result
         result = None

      return rc, result

   def list_collections(self):
      """List all collections in current database.

      Parameters:
              Name         Type     Info:
         N/A
      Return values:
         Success: SDB_OK   and   a cursor object of collection list.
         Fail   : Other    and   None
      """
      result = cursor()
      rc = sdbclient.list_collections(self._client, result._cursor)
      pysequoiadb.check_error(rc)

      if const.SDB_OK != rc:
         del result
         result = None

      return rc, result

   def list_replica_groups(self):
      """List all replica groups of current database.

      Parameters:
              Name         Type     Info:
         N/A
      Return values:
         Success: SDB_OK   and   a cursor object of replication groups.
         Fail   : Other    and   None
      """
      result = cursor()
      rc = sdbclient.list_replica_groups(self._client, result._cursor)
      pysequoiadb.check_error(rc)

      if const.SDB_OK != rc:
         del result
         result = None

      return rc, result

   def get_replica_group_by_name(self, group_name):
      """Get the specified replica group of specified group name.

      Parameters:
              Name         Type     Info:
         [in] group_name   str      The name of replica group.
      Return values:
         Success: SDB_OK   and   the replicagroup object of query.
         Fail   : Other    and   None
      """
      if not isinstance(group_name, basestring):
         raise TypeError("group name must be an instance of basestring")

      result = replicagroup(self._client)
      rc = sdbclient.get_replica_group_by_name(self._client, group_name,
                                                             result._group)
      pysequoiadb.check_error(rc)

      if const.SDB_OK != rc:
         del result
         result = None

      return rc, result

   def get_replica_group_by_id(self, id):
      """Get the specified replica group of specified group id.

      Parameters:
              Name         Type     Info:
         [in]   id         str      The id of replica group.
      Return values:
         Success: SDB_OK   and   the replicagroup object of query.
         Fail   : Other    and   None
      """
      if not isinstance(id, int):
         raise TypeError("group id must be an instance of int")

      result = replicagroup(self._client)
      rc = sdbclient.get_replica_group_by_id(self._client, id, result._group)
      pysequoiadb.check_error(rc)

      if const.SDB_OK != rc:
         del result
         result = None

      return rc, result

   def create_replica_group(self, group_name):
      """Create the specified replica group.

      Parameters:
              Name         Type     Info:
         [in] group_name   str      The name of replica group to be created.
      Return values:
         Success: SDB_OK   and   the replicagroup object created.
         Fail   : Other    and   None
      """
      if not isinstance(group_name, basestring):
         raise TypeError("group name must be an instance of basestring")

      replica_group = replicagroup(self._client)
      rc = sdbclient.create_replica_group(self._client, group_name,
                                                        replica_group._group)
      pysequoiadb.check_error(rc)

      if const.SDB_OK != rc:
         del replica_group
         replica_group = None

      return rc, replica_group

   def remove_replica_group(self, group_name):
      """Remove the specified replica group.

      Parameters:
              Name         Type     Info:
         [in] group_name   str      The name of replica group to be removed
      Return values:
         Success: SDB_OK
         Fail   : Other
      """
      if not isinstance(group_name, basestring):
         raise TypeError("group name must be an instance of basestring")

      rc = sdbclient.remove_replica_group(self._client, group_name)
      pysequoiadb.check_error(rc)

      return rc

   def create_replica_cata_group(self, host, service, path, configure):
      """Create a catalog replica group.

      Parameters:
              Name         Type     Info:
         [in] host         str      The hostname for the catalog replica group.
         [in] service      str      The servicename for the catalog replica group.
         [in] path         str      The path for the catalog replica group.
         [in] configure    dict     The configurations for the catalog replica group.
      Return values:
         Success: SDB_OK
         Fail   : Other
      """
      if not isinstance(host, basestring):
         raise TypeError("host must be an instance of basestring")
      if not isinstance(service, basestring):
         raise TypeError("service name must be an instance of basestring")
      if not isinstance(path, basestring):
         raise TypeError("path must be an instance of basestring")

      bson_configure = bson.BSON.encode(configure)

      rc = sdbclient.create_replica_cata_group(self._client, host, service,
                                               path, bson_configure)
      pysequoiadb.check_error(rc)

      return rc

   def exec_update(self, sql):
      """Executing SQL command for updating.

      Parameters:
              Name         Type     Info:
         [in] sql          str      The SQL command.
      Return values:
         Success: SDB_OK
         Fail   : Other
      """
      if not isinstance(sql, basestring):
         raise TypeError("update sql must be an instance of basestring")

      rc = sdbclient.exec_update(self._client, sql)
      pysequoiadb.check_error(rc)

      return rc

   def exec_sql(self, sql):
      """Executing SQL command.

      Parameters:
              Name         Type     Info:
         [in] sql          str      The SQL command.
      Return values:
         Success: SDB_OK   and   a cursor object of matching documents.
         Fail   : Other    and   None
      """
      if not isinstance(sql, basestring):
         raise TypeError("sql must be an instance of basestring")

      result = cursor()
      rc = sdbclient.exec_sql(self._client, sql, result._cursor)
      pysequoiadb.check_error(rc)

      if const.SDB_OK != rc:
         del result
         result = None

      return rc, result

   def transaction_begin(self):
      """Transaction begin.

      Parameters:
              Name         Type     Info:
         N/A
      Return values:
         Success: SDB_OK
         Fail   : Other
      """
      rc = sdbclient.transaction_begin(self._client)
      pysequoiadb.check_error(rc)

      return rc

   def transaction_commit(self):
      """Transaction commit.

      Parameters:
              Name         Type     Info:
         N/A
      Return values:
         Success: SDB_OK
         Fail   : Other
      """
      rc = sdbclient.transaction_commit(self._client)
      pysequoiadb.check_error(rc)

      return rc

   def transaction_rollback(self):
      """Transaction rollback

      Parameters:
              Name         Type     Info:
         N/A
      Return values:
         Success: SDB_OK
         Fail   : Other
      """
      rc = sdbclient.transaction_rollback(self._client)
      pysequoiadb.check_error(rc)

      return rc

   def flush_configure(self, options):
      """Flush the options to configure file.
      Parameters:
              Name      Type  Info:
         [in] options   dict  The configure infomation, pass {"Global":true} or
                        {"Global":false} In cluster environment, passing
                        {"Global":true} will flush data's and catalog's 
                        configuration file, while passing {"Global":false} will 
                        flush coord's configuration file. In stand-alone
                        environment, both them have the same behaviour.
      Return values:
         Success: SDB_OK
         Fail   : Other
      """
      bson_options = bson.BSON.encode(options)
      rc = sdbclient.flush_configure(self._client, bson_options)
      pysequoiadb.check_error(rc)

      return rc

   def create_procedure(self, code):
      """Create a store procedures

      Parameters:
              Name         Type     Info:
         [in] code         str      The JS code of store procedures.
      Return values:
         Success: SDB_OK
         Fail   : Other
      """
      if not isinstance(code, basestring):
         raise TypeError("code must be an instance of basestring")
      rc = sdbclient.create_JS_procedure(self._client, code)
      pysequoiadb.check_error(rc)

      return rc

   def remove_procedure(self, name):
      """Remove a store procedures.
     
      Parameters:
              Name         Type     Info:
         [in] name         str      The name of store procedure.
      Return values:
         Success: SDB_OK
         Fail   : Other
      """
      if not isinstance(name, basestring):
         raise TypeError("procedure name must be an instance of basestring")
      rc = sdbclient.remove_procedure(self._client, name)
      pysequoiadb.check_error(rc)

      return rc

   def list_procedures(self, condition):
      """List store procedures.

      Parameters:
              Name         Type     Info:
         [in] condition    dict     The condition of list.
      Return values:
         Success: SDB_OK  and  an cursor object of result
         Fail   : Other   and  None
      """
      bson_condition = bson.BSON.encode(condition)
      result = cursor()
      rc = sdbclient.list_procedures(self._client, result._cursor,
                                                   bson_condition)
      pysequoiadb.check_error(rc)

      return rc, result

   def eval_procedure(self, name):
      """Eval a func.
      
      Parameters:
              Name         Type     Info:
         [in] name         str      The name of store procedure.
      Return values:
         Success: SDB_OK   and  a cursor object of current eval.
         Fail   : Other    and  None
      """
      if not isinstance(name, basestring):
         raise TypeError("code must be an instance of basestring")

      result = cursor()
      rc = sdbclient.eval_JS(self._client, result._cursor, name)
      pysequoiadb.check_error(rc)
      if const.SDB_OK != rc:
         result = None

      return rc, result

   def backup_offline(self, options = static_object):
      """Backup the whole database or specifed replica group.

      Parameters:
              Name      Type  Info:
         [in] options   dict  Contains a series of backup configuration
                              infomations. Backup the whole cluster if None. 
                              The "options" contains 5 options as below. 
                              All the elements in options are optional. 
                              eg:
                              { "GroupName":["rgName1", "rgName2"], 
                                "Path":"/opt/sequoiadb/backup",
                                "Name":"backupName", "Description":description,
                                "EnsureInc":true, "OverWrite":true }
                              See Info as below.
      Return values:
         Success: SDB_OK
         Fail   : Other
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
         bson_options = bson.BSON.encode(options)

      rc = sdbclient.backup_offline(self._client, bson_options)
      pysequoiadb.check_error(rc)

      return rc

   def list_backup(self, options, condition = static_object,
                                  selector  = static_object,
                                  order_by  = static_object):
      """List the backups.

      Parameters:
              Name      Type     Info:
         [in] options   dict     Contains configuration infomations for remove
                                 backups, list all the backups in the default
                                 backup path if None. 
                                 The "options" contains 3 options as below. 
                                 All the elements in options are optional. 
                                 eg:
                                 { "GroupName":["rgame1", "rgName2"], 
                                   "Path":"/opt/sequoiadb/backup",
                                   "Name":"backupName" }
                                 See Info as below.
         [in] condition dict     The matching rule, return all the documents
                                 if None.
         [in] selector  dict     The selective rule, return the whole document
                                 if None.
         [in] order_by  dict     The ordered rule, never sort if None.
      Return values:
         Success: SDB_OK  and  a cursor object of backup list
         Fail   : Others  and  None
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

      bson_options = bson.BSON.encode(options)
      if condition is not None:
         bson_condition = bson.BSON.encode(condition)
      if selector is not None:
         bson_selector = bson.BSON.encode(selector)
      if order_by is not None:
         bson_order_by = bson.BSON.encode(order_by)

      result = cursor()
      rc = sdbclient.list_backup(self._client, result._cursor, bson_options,
                                 bson_condition, bson_selector, bson_order_by)
      pysequoiadb.check_error(rc)

      if const.SDB_OK != rc:
         del result
         result = None

      return rc, result

   def remove_backup(self, options):
      """Remove the backups

      Parameters:
              Name      Type  Info:
         [in] options   dict  Contains configuration infomations for remove
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
         Success: SDB_OK  and  an cursor object of result
         Fail   : Other   and  None

      Info:
         GroupName   : Assign the backups of specifed replica groups to be
                       remove.
         Path        : Assign the backups in specifed path to be remove, if not
                       assign, use the backup path asigned in the configuration
                       file.
         Name        : Assign the backups with specifed name to be remove.
      """
      bson_condition = None

      bson_options = bson.BSON.encode(options)
      rc = sdbclient.remove_backup(self._client, bson_options)
      pysequoiadb.check_error(rc)

      return rc

   def list_task(self, condition = static_object, selector  = static_object,
                       order_by  = static_object, hint      = static_object):
      """List the tasks.

      Parameters:
              Name         Type     Info:
         [in] condition    dict     The matching rule, return all the documents
                                    if None.
         [in] selector     dict     The selective rule, return the whole
                                    document if None.
         [in] order_by     dict     The ordered rule, never sort if None.
                                    bson.SON may need if it is order-sensitive.
                                    eg.
                                    bson.SON([("name",-1), ("age":1)]) it will
                                    be ordered descending by 'name' first, and
                                    be ordered ascending by 'age'
         [in] hint         dict     The hint, automatically match the optimal
                                    hint if None.
      Return values:
         Success: SDB_OK  and  a cursor object of task list
         Fail   : Others  and  None
      """
      bson_condition = None
      bson_selector = None
      bson_order_by = None
      bson_hint = None

      if condition is not None:
         bson_condition = bson.BSON.encode(condition)
      if selector is not None:
         bson_selector = bson.BSON.encode(selector)
      if order_by is not None:
         bson_order_by = bson.BSON.encode(order_by)
      if hint is not None:
         bson_hint = bson.BSON.encode(hint)

      result = cursor()
      rc = sdbclient.list_task(self._client, result._cursor, bson_condition,
                               bson_selector, bson_order_by, bson_hint)
      pysequoiadb.check_error(rc)

      if const.SDB_OK != rc:
         del result
         result = None

      return rc, result

   def wait_task(self, task_ids, num):
      """Wait the tasks to finish.

      Parameters:
              Name         Type     Info:
         [in] task_ids     list     The list of task id.
         [in] num          int      The number of task id.
      Return values:
         Success: SDB_OK
         Fail   : Others
      """
      if not isinstance(task_ids, list):
         raise TypeError("task id must be an instance of list")
      if not isinstance(num, int):
         raise TypeError("size of tasks must be an instance of int")

      rc = sdbclient.wait_task(self._client, task_ids, num)
      pysequoiadb.check_error(rc)

      return rc

   def cancel_task(self, task_id, is_async):
      """Cancel the specified task.

      Parameters:
              Name         Type     Info:
         [in] task_id      long     The task id to be canceled.
         [in] is_async     int      The operation "cancel task" is async or not,
                                    "true" for async, "false" for sync.
      Return values:
         Success: SDB_OK
         Fail   : Others
      """
      if not isinstance(task_id, long):
         raise TypeError("task id must be an instance of list")

      async = 0
      if isinstance(is_async, bool):
         if is_async:
            async = 1
      else:
         raise TypeError("size of tasks must be an instance of int")

      rc = sdbclient.cancel_task(self._client, task_id, async)
      pysequoiadb.check_error(rc)

      return rc

   def set_session_attri(self, options = static_object):
      """Set the attributes of the session.

      Parameters:
              Name         Type     Info:
         [in] options      dict     The configuration options for session.
      Return values:
         Success: SDB_OK
         Fail   : Others
      """
      bson_options = None
      if options is not None:
         bson_options = bson.BSON.encode(options)

      rc = sdbclient.set_session_attri(self._client, bson_options)
      pysequoiadb.check_error(rc)

      return rc

   def close_all_cursors(self):
      """Close all the cursors in current thread, we can't use those cursors to 
         get data again.

      Parameters:
              Name         Type     Info:
         N/A
      Return values:
         Success: SDB_OK
         Fail   : Others
      """
      rc = sdbclient.close_all_cursors(self._client)
      pysequoiadb.check_error(rc)

      return rc

   def is_valid(self):
      """Judge whether the connection is valid.

      Parameters:
              Name         Type     Info:
         N/A
      Return values:
         Success: True, if the result is valid 
         Fail   : False
      """
      rc, valid = sdbclient.is_valid(self._client)
      pysequoiadb.check_error(rc)
      if const.SDB_OK != rc:
         valid = False

      return valid
