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

import sdbclient

import bson
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
   def __init__(self, host = default_host, port = default_port,
                      user = default_user, psw  = default_psw):
      # if NULL returned, a SystemError will be raised
      self._client = sdbclient.create_client()
      rc = sdbclient.init_connect(self._client, host, port)
      if rc:
         sdbclient.disconnect(self._client)
     
   def __del__(self):
      if self._client is not None:
         sdbclient.release_client(self._client)
      self._client = None

   def __getitem__(self, item_name):
      cs = collectionspace()
      rc = sdbclient.get_collection_space(self._client, item_name, cs._cs)
      if const.SDB_OK != rc:
         cs = None
      return rc, cs

   def connect(self, host = default_host, port = default_port,
                     user = default_user, psw  = default_psw):
      rc = sdbclient.connect_by_host(self._client, host, port, user, psw)
      if rc:
         pass

   def connect(self, host, service_name, user = default_user,
                                         psw  = default_psw):
      rc = sdbclient.connect_by_service(self._client, host, service_name,
                                                      user, psw)
      return rc

   def connect(self, addr, addr_size, user = default_user, psw = default_psw):
      rc = sdbclient.connect_by_address(self._client, addr, addr_size,
                                                      user, psw)
      return rc

   def disconnect(self):
      rc = sdbclient.disconnect(self._client)
      return rc

   def is_connected(self):
      rc = sdbclient.is_connected(self._client)
      return rc

   def create_user(self, name, psw):
      rc = sdbclient.create_user(self._client, name, psw)
      return rc

   def remove_user(self, user, psw):
      rc = sdbclient.remove_user(self._client, user, psw)
      return rc

   def get_snapshot(self, snap_type, condition = static_object,
                                     selector  = static_object,
                                     order_by  = static_object):
      if condition is not None:
         bson_condition = bson.BSON.encode(condition)
      if selector is not None:
         bson_selector = bson.BSON.encode(selector)
      if condition is not None:
         bson_order_by = bson.BSON.encode(order_by)

      result = cursor()
      rc = sdbclient.get_snapshot(self._client, result._cursor, snap_type,
                                  bson_condition, bson_selector, bson_order_by)
      if const.SDB_OK != rc:
         result = None
      return rc, result

   def reset_snapshot(self, condition = static_object):
      if condition is not None:
         bson_condition = bson.BSON.encode(condition)
      rc = sdbclient.reset_snapshot(self._client, bson_condition)
      return rc 

   def get_list(self, list_type, condition = static_object,
                                 selector  = static_object,
                                 order_by  = static_object):
      if condition is not None:
         bson_condition = bson.BSON.encode(condition)
      if selector is not None:
         bson_selector = bson.BSON.encode(selector)
      if condition is not None:
         bson_order_by = bson.BSON.encode(order_by)
      result = cursor()
      rc = sdbclient.get_list(self._client, result._cursor, list_type,
                              bson_condition, bson_selector, bson_order_by)
      if const.SDB_OK != rc:
         result = None
      return rc, result

   def get_collection_space(self, cs_name):
      cs = collectionspace()
      rc = sdbclient.get_collection_space(self._client, cs_name, 
                                 cs._cs)
      if const.SDB_OK != rc:
         cs = None
      return rc, cs

   def create_collection_space(self, cs_name, page_size = 0):
      cs = collectionspace()
      rc = sdbclient.create_collection_space(self._client, cs_name,
                                             page_size, cs._cs)
      if const.SDB_OK != rc:
         cs = None
      return rc, cs

   def drop_collection_space(self, cs_name):
      rc = sdbclient.drop_collection_space(self._client, cs_name)
      return rc

   def list_collection_spaces(self):
      result = cursor()
      rc = sdbclient.list_collection_spaces(self._client, result._cursor)
      if const.SDB_OK != rc:
         result = None
      return rc, result

   def list_replica_groups(self):
      result = cursor()
      rc = sdbclient.list_replica_groups(self._client, result._cursor)
      if const.SDB_OK != rc:
         result = None
      return rc, result

   def get_replica_group_by_name(self, group_name):
      result = replicagroup(self._client)
      rc = sdbclient.get_replica_group_by_name(self._client, group_name,
                                                             result._group)
      if const.SDB_OK != rc:
         result = None
      return rc, result

   def get_replica_group_by_id(self, id):
      result = replicagroup(self._client)
      rc = sdbclient.get_replica_group_by_id(self._client, id, result._group)
      if const.SDB_OK != rc:
         result = None
      return rc, result

   def creat_replica_group(self, group_name):
      replica_group = replicagroup(self._client)
      rc = sdbclient.create_replica_group(self._client, group_name,
                                                        replica_group._group)
      return rc, replica_group


   def remove_replica_group(self, group_name):
      rc = sdbclient.remove_replica_group(self._client, group_name)
      return rc

   def create_replica_cata_group(self, host, service_name, dbpath, configure):
      bson_configure = bson.BSON.encode(configure)
      rc = sdbclient.create_replica_cata_group(self._client, host, service_name,
                                               dbpath, bson_configure)
      return rc

   def exec_update(self, sql):
      rc = sdbclient.exec_update(self._client, sql)
      return rc

   def exec_sql(self, sql):
      result = cursor()
      rc = sdbclient.exec_sql(self._client, sql, result._cursor)
      if const.SDB_OK != rc:
         result = None
      return rc, result

   def transaction_begin(self):
      rc = sdbclient.transaction_begin(self._client)
      return rc

   def transaction_commit(self):
      rc = sdbclient.transaction_commit(self._client)
      return rc

   def transaction_rollback(self):
      rc = sdbclient.transaction_rollback(self._client)
      return rc

   def flush_configure(self, options):
      if options is not None:
         bson_options = bson.BSON.encode(options)

      rc = sdbclient.flush_configure(self._client, bson_options)
      return rc

#   def crt_js_procedure(self, code)
#   def rm_procedure(self, sp_name)
#   def list_procedures(self, cursor, condition)
#   def eval_js(self, cursor, code, type, err_msg)

   def backup_offline(self, options):
      rc = sdbclient.backup_offline(self._client, options)
      return rc

   def list_backup(self, options, condition = static_object,
                                  selector  = static_object,
                                  order_by  = static_object):
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
      if const.SDB_OK != rc:
         result = None
      return rc, result

   def list_task(self, condition = static_object, selector  = static_object,
                       order_by  = static_object, hint      = static_object):
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
      if const.SDB_OK != rc:
         result = None
      return rc, result

   def wait_task(self, task_ids, num):
      rc = sdbclient.wait_task(self._client, task_ids, num)
      return rc

   def cancel_task(self, task_id, is_async):
      rc = sdbclient.cancel_task(self._client, task_id, is_async)
      return rc

   def set_session_attri(self, options = static_object):
      if options is not None:
         bson_options = bson.BSON.encode(options)

      rc = sdbclient.set_session_attri(self._client, bson_options)
      return rc

   def close_all_cursors(self):
      rc = sdbclient.close_all_cursors(self._client)
      return rc

   def is_valid(self):
      rc, valid = sdbclient.is_valid(self._client)
      if const.SDB_OK != rc:
         valid = False
      return valid
