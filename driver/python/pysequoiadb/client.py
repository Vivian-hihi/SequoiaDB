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

"""Module of client for python driver for SequoiaDB
"""

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
                          default_psw,
                          driver_version )
from pysequoiadb.collectionspace import collectionspace
from pysequoiadb.collection import collection
from pysequoiadb.cursor import cursor
from pysequoiadb.replicagroup import replicagroup
from pysequoiadb.common import const


class client(object):
   """SequoiaDB Client Driver
   
   The client support interfaces to connect to SequoiaDB.
   In order to connect to SequoiaDB, you need use the class first.
   And you should make sure the instance of it released when you don't use it
   any more.
   """
   def __init__(self, host = default_host, port = default_port):

      try:
         self._client = sdbclient.create_client()
         # try to connect with default user and password 
         rc = sdbclient.init_connect(self._client, host, port)
         pysequoiadb.check_error(rc)
         if const.SDB_OK != rc:
            what = "connect to %s:%d failed\n" % host, port
            pysequoiadb.printInfo(what)
            sdbclient.disconnect(self._client)
      except SystemError:
         pysequoiadb.check_error(const.SDM_OOM)
     
   def __del__(self):

      if self._client is not None:
         rc = sdbclient.release_client(self._client)
         pysequoiadb.check_error(rc)
         self._client = None

   def __getitem__(self, name):

      return self.__getattr__(name)

   def __getattr__(self, name):

      cs = collectionspace()
      rc = sdbclient.get_collection_space(self._client, name, cs._cs)

      pysequoiadb.check_error(rc)
      if const.SDB_OK != rc:
         cs = None

      return cs

   def __dir__(self):

      return [self.__members__, self.__methods__]

   def connect_by_host(self, host = default_host, port = default_port,
                             user = default_user, psw  = default_psw):

      rc = sdbclient.connect_by_host(self._client, host, port, user, psw)
      pysequoiadb.check_error(rc)

      return rc

   def connect_by_service(self, host, service_name, user = default_user,
                                                    psw  = default_psw):

      rc = sdbclient.connect_by_service(self._client, host, service_name,
                                                      user, psw)
      pysequoiadb.check_error(rc)

      return rc

   def connect_by_address(self, addr, addr_size, user = default_user,
                                                 psw = default_psw):

      rc = sdbclient.connect_by_address(self._client, addr, addr_size,
                                                      user, psw)
      pysequoiadb.check_error(rc)

      return rc

   def disconnect(self):

      rc = sdbclient.disconnect(self._client)
      pysequoiadb.check_error(rc)

      return rc

   def create_user(self, name, psw):

      rc = sdbclient.create_user(self._client, name, psw)
      pysequoiadb.check_error(rc)

      return rc

   def remove_user(self, user, psw):

      rc = sdbclient.remove_user(self._client, user, psw)
      pysequoiadb.check_error(rc)

      return rc

   def get_snapshot(self, snap_type, condition = static_object,
                                     selector  = static_object,
                                     order_by  = static_object):

      bson_condition = None
      bson_selector = None
      bson_order_by = None

      if condition is not None:
         bson_condition = bson.BSON.encode(condition)
      if selector is not None:
         bson_selector = bson.BSON.encode(selector)
      if condition is not None:
         bson_order_by = bson.BSON.encode(order_by)

      result = cursor()
      rc = sdbclient.get_snapshot(self._client, result._cursor, snap_type,
                                  bson_condition, bson_selector, bson_order_by)
      pysequoiadb.check_error(rc)
      if const.SDB_OK != rc:
         result = None

      return rc, result

   def reset_snapshot(self, condition = static_object):

      bson_condition = None
      if condition is not None:
         bson_condition = bson.BSON.encode(condition)

      rc = sdbclient.reset_snapshot(self._client, bson_condition)
      pysequoiadb.check_error(rc)

      return rc 

   def get_list(self, list_type, condition = static_object,
                                 selector  = static_object,
                                 order_by  = static_object):

      bson_condition = None
      bson_selector = None
      bson_order_by = None

      if condition is not None:
         bson_condition = bson.BSON.encode(condition)
      if selector is not None:
         bson_selector = bson.BSON.encode(selector)
      if condition is not None:
         bson_order_by = bson.BSON.encode(order_by)

      result = cursor()
      rc = sdbclient.get_list(self._client, result._cursor, list_type,
                              bson_condition, bson_selector, bson_order_by)
      pysequoiadb.check_error(rc)

      if const.SDB_OK != rc:
         result = None

      return rc, result

   def get_collection(self, cl_full_name):
      
      cl = collection()
      rc = sdbclient.get_collection(self._client, cl_full_name, cl._cl)
      pysequoiadb.check_error(rc)

      if const.SDB_OK != rc:
         cl = None

      return rc, cl

   def get_collection_space(self, cs_name):

      cs = collectionspace()
      rc = sdbclient.get_collection_space(self._client, cs_name, cs._cs)
      pysequoiadb.check_error(rc)

      if const.SDB_OK != rc:
         cs = None

      return rc, cs

   def create_collection_space(self, cs_name, page_size = 0):

      cs = collectionspace()
      rc = sdbclient.create_collection_space(self._client, cs_name,
                                             page_size, cs._cs)
      pysequoiadb.check_error(rc)

      if const.SDB_OK != rc:
         cs = None

      return rc, cs

   def drop_collection_space(self, cs_name):

      rc = sdbclient.drop_collection_space(self._client, cs_name)
      pysequoiadb.check_error(rc)

      return rc

   def list_collection_spaces(self):

      result = cursor()
      rc = sdbclient.list_collection_spaces(self._client, result._cursor)
      pysequoiadb.check_error(rc)

      if const.SDB_OK != rc:
         result = None

      return rc, result

   def list_collections(self):

      result = cursor()
      rc = sdbclient.list_collections(self._client, result._cursor)
      pysequoiadb.check_error(rc)

      if const.SDB_OK != rc:
         result = None

      return rc, result

   def list_replica_groups(self):

      result = cursor()
      rc = sdbclient.list_replica_groups(self._client, result._cursor)
      pysequoiadb.check_error(rc)

      if const.SDB_OK != rc:
         result = None

      return rc, result

   def get_replica_group_by_name(self, group_name):

      result = replicagroup(self._client)
      rc = sdbclient.get_replica_group_by_name(self._client, group_name,
                                                             result._group)
      pysequoiadb.check_error(rc)

      if const.SDB_OK != rc:
         result = None

      return rc, result

   def get_replica_group_by_id(self, id):

      result = replicagroup(self._client)
      rc = sdbclient.get_replica_group_by_id(self._client, id, result._group)
      pysequoiadb.check_error(rc)

      if const.SDB_OK != rc:
         result = None

      return rc, result

   def creat_replica_group(self, group_name):

      replica_group = replicagroup(self._client)
      rc = sdbclient.create_replica_group(self._client, group_name,
                                                        replica_group._group)
      pysequoiadb.check_error(rc)

      if const.SDB_OK != rc:
         replica_group = None

      return rc, replica_group

   def remove_replica_group(self, group_name):

      rc = sdbclient.remove_replica_group(self._client, group_name)
      pysequoiadb.check_error(rc)

      return rc

   def create_replica_cata_group(self, host, service_name, dbpath, configure):

      bson_configure = bson.BSON.encode(configure)

      rc = sdbclient.create_replica_cata_group(self._client, host, service_name,
                                               dbpath, bson_configure)
      pysequoiadb.check_error(rc)

      return rc

   def exec_update(self, sql):

      rc = sdbclient.exec_update(self._client, sql)
      pysequoiadb.check_error(rc)

      return rc

   def exec_sql(self, sql):

      result = cursor()
      rc = sdbclient.exec_sql(self._client, sql, result._cursor)
      pysequoiadb.check_error(rc)

      if const.SDB_OK != rc:
         result = None

      return rc, result

   def transaction_begin(self):

      rc = sdbclient.transaction_begin(self._client)
      pysequoiadb.check_error(rc)

      return rc

   def transaction_commit(self):

      rc = sdbclient.transaction_commit(self._client)
      pysequoiadb.check_error(rc)

      return rc

   def transaction_rollback(self):

      rc = sdbclient.transaction_rollback(self._client)
      pysequoiadb.check_error(rc)

      return rc

   def flush_configure(self, options):

      bson_options = bson.BSON.encode(options)
      rc = sdbclient.flush_configure(self._client, bson_options)
      pysequoiadb.check_error(rc)

      return rc

   def create_js_procedure(self, code):

      rc = sdbclient.create_JS_procedure(self._client, code)
      pysequoiadb.check_error(rc)

      return rc

   def remove_procedure(self, sp_name):

      rc = sdbclient.remove_procedure(self._client, sp_name)
      pysequoiadb.check_error(rc)

      return rc

   def list_procedures(self, condition):

      bson_condition = bson.BSON.encode(condition)
      result = cursor()
      rc = sdbclient.list_procedures(self._client, result._cursor,
                                                   bson_condition)
      pysequoiadb.check_error(rc)

      return rc, result

   def eval_js(self, code):

      result = cursor()
      rc = sdbclient.eval_JS(self._client, result._cursor, code)
      pysequoiadb.check_error(rc)

      return rc, result

   def backup_offline(self, options):

      rc = sdbclient.backup_offline(self._client, options)
      pysequoiadb.check_error(rc)

      return rc

   def list_backup(self, options, condition = static_object,
                                  selector  = static_object,
                                  order_by  = static_object):

      bson_condition = None
      bson_selector = None
      bson_order_by = None

      bson_options = bson.BSON.encode(options)
      if condition is not None:
         bson_condition = bson.BSON.encode(condition)
      if selector is not None:
         bson_selector = bson.BSON.encode(selector)
      if condition is not None:
         bson_order_by = bson.BSON.encode(order_by)

      result = cursor()
      rc = sdbclient.list_backup(self._client, result._cursor, bson_options,
                                 bson_condition, bson_selector, bson_order_by)
      pysequoiadb.check_error(rc)

      if const.SDB_OK != rc:
         result = None

      return rc, result

   def list_task(self, condition = static_object, selector  = static_object,
                       order_by  = static_object, hint      = static_object):
      
      bson_condition = None
      bson_selector = None
      bson_order_by = None
      bson_hint = None

      if condition is not None:
         bson_condition = bson.BSON.encode(condition)
      if selector is not None:
         bson_selector = bson.BSON.encode(selector)
      if condition is not None:
         bson_order_by = bson.BSON.encode(order_by)
      if hint is not None:
         bson_hint = bson.BSON.encode(hint)

      result = cursor()
      rc = sdbclient.list_task(self._client, result._cursor, bson_condition,
                               bson_selector, bson_order_by, bson_hint)
      pysequoiadb.check_error(rc)

      if const.SDB_OK != rc:
         result = None
      
      return rc, result

   def wait_task(self, task_ids, num):

      rc = sdbclient.wait_task(self._client, task_ids, num)
      pysequoiadb.check_error(rc)
      
      return rc

   def cancel_task(self, task_id, is_async):
      
      rc = sdbclient.cancel_task(self._client, task_id, is_async)
      pysequoiadb.check_error(rc)

      return rc

   def set_session_attri(self, options = static_object):
      
      bson_options = None
      if options is not None:
         bson_options = bson.BSON.encode(options)

      rc = sdbclient.set_session_attri(self._client, bson_options)
      pysequoiadb.check_error(rc)

      return rc

   def close_all_cursors(self):
      
      rc = sdbclient.close_all_cursors(self._client)
      pysequoiadb.check_error(rc)

      return rc

   def is_valid(self):

      rc, valid = sdbclient.is_valid(self._client)
      pysequoiadb.check_error(rc)
      if const.SDB_OK != rc:
         valid = False

      return valid
