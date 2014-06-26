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
   def __init__(self, host = default_host, port = default_port,
                      user = default_user, psw  = default_psw):
      """initialize when product a object.
 
         it will try to connect to SequoiaDB using host and port given,
         localhost and 11810 are the default value of host and port,
         user and password are "". 
      """
      self._host = host
      self._port = port
      self._user = user
      self._psw  = '*' * len(psw)
      try:
         self._client = sdbclient.create_client()
         # try to connect with default user and password 
         rc = sdbclient.init_connect(self._client, host, port, user, psw)
         if const.SDB_OK != rc:
            what = " Error: connect to host: %s, port: %d" % (host, port) + \
                   " user: %s, psw : %s failed." % (user, psw)
            pysequoiadb.printInfo(what)
            sdbclient.disconnect(self._client)
      except SystemError:
         pysequoiadb.check_error(const.SDM_OOM)

   def __del__(self):
      """release resource when del called.

      """
      self._host = default_host
      self._port = default_port
      self._user = default_user
      self._psw  = default_psw
      if self._client is not None:
         rc = sdbclient.release_client(self._client)
         pysequoiadb.check_error(rc)
         self._client = None

   def __str__(self):

      return "Client "

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
   
   @classmethod
   def connect_by_host(self, host = default_host, port = default_port,
                             user = default_user, psw  = default_psw):
      """connect to database, using default host ip, port, user, password.

         if there is no arguments specified, default value insteaded. 
      """
      self._host = host
      self._port = port
      self._user = user
      self._psw  = '*' * len(psw)

      rc = sdbclient.connect_by_host(self._client, host, port, user, psw)
      pysequoiadb.check_error(rc)

      return rc

   @classmethod
   def connect_by_service(self, host, service_name, user = default_user,
                                                    psw  = default_psw):
      """connect to database, using host ip and service name, user and password.

         if there is no arguments specified, default value insteaded. 
      """
      self._host = host
      self._service = service_name
      self._user = user
      self._psw  = '*' * len(psw)
      rc = sdbclient.connect_by_service(self._client, host, service_name,
                                                      user, psw)
      pysequoiadb.check_error(rc)

      return rc

   @classmethod
   def disconnect(self):
      """release current connection.
       
      """
      rc = sdbclient.disconnect(self._client)
      pysequoiadb.check_error(rc)

      return rc

   @classmethod
   def create_user(self, name, psw):
      """create a new user, with user name and password.

      """
      rc = sdbclient.create_user(self._client, name, psw)
      pysequoiadb.check_error(rc)

      return rc

   @classmethod
   def remove_user(self, user, psw):
      """delete a user.
      """
      rc = sdbclient.remove_user(self._client, user, psw)
      pysequoiadb.check_error(rc)

      return rc

   @classmethod
   def get_snapshot(self, snap_type, condition = static_object,
                                     selector  = static_object,
                                     order_by  = static_object):
      """get a snapshot of current database.

         if success, a cursor object will be returned with error code,
         or cursor object is None.
      """
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
         del result
         result = None

      return rc, result

   @classmethod
   def reset_snapshot(self, condition = static_object):
      """reset the snapshot.
      """
      bson_condition = None
      if condition is not None:
         bson_condition = bson.BSON.encode(condition)

      rc = sdbclient.reset_snapshot(self._client, bson_condition)
      pysequoiadb.check_error(rc)

      return rc

   @classmethod
   def get_list(self, list_type, condition = static_object,
                                 selector  = static_object,
                                 order_by  = static_object):
      """list the content of database, according to the given list_type.

         if success, a cursor object will be returned with error code,
         or cursor object is None.
      """
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
         del result
         result = None

      return rc, result

   @classmethod
   def get_collection(self, cl_full_name):
      """get the collection by the full name specified.

         if success, a collection object will be returned with error code,
         or collection object is None.
      """
      cl = collection()
      rc = sdbclient.get_collection(self._client, cl_full_name, cl._cl)
      pysequoiadb.check_error(rc)

      if const.SDB_OK != rc:
         del cl
         cl = None

      return rc, cl

   @classmethod
   def get_collection_space(self, cs_name):
      """get the collection space by the name specified.

         if success, a collection space object will be returned with error code,
         or collection object is None.
      """
      cs = collectionspace()
      rc = sdbclient.get_collection_space(self._client, cs_name, cs._cs)
      pysequoiadb.check_error(rc)

      if const.SDB_OK != rc:
         del cs
         cs = None

      return rc, cs

   @classmethod
   def create_collection_space(self, cs_name, page_size = 0):
      """create the collection space by the name specified.

         page_size must be times of 4096(4k). 0 means create collection space
         using default page size.
         if success, a collection space object will be returned with error code,
         or collection object is None.
      """
      cs = collectionspace()
      rc = sdbclient.create_collection_space(self._client, cs_name,
                                             page_size, cs._cs)
      pysequoiadb.check_error(rc)

      if const.SDB_OK != rc:
         del cs
         cs = None

      return rc, cs

   @classmethod
   def drop_collection_space(self, cs_name):
      """drop the collection space
      """

      rc = sdbclient.drop_collection_space(self._client, cs_name)
      pysequoiadb.check_error(rc)

      return rc

   @classmethod
   def list_collection_spaces(self):
      """list all the collection spaces in current database.

         if success, a cursor object will be returned with error code,
         or cursor object is None.
      """
      result = cursor()
      rc = sdbclient.list_collection_spaces(self._client, result._cursor)
      pysequoiadb.check_error(rc)

      if const.SDB_OK != rc:
         del result
         result = None

      return rc, result

   @classmethod
   def list_collections(self):
      """list all the collection in current database.

         if success, a cursor object will be returned with error code,
         or cursor object is None.
      """
      result = cursor()
      rc = sdbclient.list_collections(self._client, result._cursor)
      pysequoiadb.check_error(rc)

      if const.SDB_OK != rc:
         del result
         result = None

      return rc, result

   @classmethod
   def list_replica_groups(self):
      """list all the replica groups in current database.

         if success, a cursor object will be returned with error code,
         or cursor object is None.
      """
      result = cursor()
      rc = sdbclient.list_replica_groups(self._client, result._cursor)
      pysequoiadb.check_error(rc)

      if const.SDB_OK != rc:
         del result
         result = None

      return rc, result

   @classmethod
   def get_replica_group_by_name(self, group_name):
      """get the replica group using specified group name.

         if success, a cursor object will be returned with error code,
         or cursor object is None.
      """
      result = replicagroup(self._client)
      rc = sdbclient.get_replica_group_by_name(self._client, group_name,
                                                             result._group)
      pysequoiadb.check_error(rc)

      if const.SDB_OK != rc:
         del result
         result = None

      return rc, result

   @classmethod
   def get_replica_group_by_id(self, id):
      """get the replica group using specified group id.

         if success, a cursor object will be returned with error code,
         or cursor object is None.
      """
      result = replicagroup(self._client)
      rc = sdbclient.get_replica_group_by_id(self._client, id, result._group)
      pysequoiadb.check_error(rc)

      if const.SDB_OK != rc:
         del result
         result = None

      return rc, result

   @classmethod
   def creat_replica_group(self, group_name):
      """create the replica group using specified group name.

         if success, a cursor object will be returned with error code,
         or cursor object is None.
      """
      replica_group = replicagroup(self._client)
      rc = sdbclient.create_replica_group(self._client, group_name,
                                                        replica_group._group)
      pysequoiadb.check_error(rc)

      if const.SDB_OK != rc:
         del replica_group
         replica_group = None

      return rc, replica_group

   @classmethod
   def remove_replica_group(self, group_name):
      """remove the replica group using specified group name.

      """
      rc = sdbclient.remove_replica_group(self._client, group_name)
      pysequoiadb.check_error(rc)

      return rc

   @classmethod
   def create_replica_cata_group(self, host, service_name, dbpath, configure):
      """create the replica group using specified group name.

         if success, SDB_OK will be returned.
      """
      bson_configure = bson.BSON.encode(configure)

      rc = sdbclient.create_replica_cata_group(self._client, host, service_name,
                                               dbpath, bson_configure)
      pysequoiadb.check_error(rc)

      return rc

   @classmethod
   def exec_update(self, sql):
      """execute a update sql.

      """
      rc = sdbclient.exec_update(self._client, sql)
      pysequoiadb.check_error(rc)

      return rc

   @classmethod
   def exec_sql(self, sql):
      """execute a sql.

      """
      result = cursor()
      rc = sdbclient.exec_sql(self._client, sql, result._cursor)
      pysequoiadb.check_error(rc)

      if const.SDB_OK != rc:
         del result
         result = None

      return rc, result

   @classmethod
   def transaction_begin(self):
      """begin transaction.

      """
      rc = sdbclient.transaction_begin(self._client)
      pysequoiadb.check_error(rc)

      return rc

   @classmethod
   def transaction_commit(self):
      """commit transaction.

      """
      rc = sdbclient.transaction_commit(self._client)
      pysequoiadb.check_error(rc)

      return rc

   @classmethod
   def transaction_rollback(self):
      """rollback

      """

      rc = sdbclient.transaction_rollback(self._client)
      pysequoiadb.check_error(rc)

      return rc

   @classmethod
   def flush_configure(self, options):
      """flush the configure

      """
      bson_options = bson.BSON.encode(options)
      rc = sdbclient.flush_configure(self._client, bson_options)
      pysequoiadb.check_error(rc)

      return rc

   @classmethod
   def create_js_procedure(self, code):
      """create procedure usring js

      """
      rc = sdbclient.create_JS_procedure(self._client, code)
      pysequoiadb.check_error(rc)

      return rc

   @classmethod
   def remove_procedure(self, sp_name):
      """remove procedure.
     
         procedure name should be specified.
      """
      rc = sdbclient.remove_procedure(self._client, sp_name)
      pysequoiadb.check_error(rc)

      return rc

   @classmethod
   def list_procedures(self, condition):
      """list all procedures in current database.

         if success, a cursor object will be returned with error code,
         or cursor object is None.
      """
      bson_condition = bson.BSON.encode(condition)
      result = cursor()
      rc = sdbclient.list_procedures(self._client, result._cursor,
                                                   bson_condition)
      pysequoiadb.check_error(rc)

      return rc, result

   @classmethod
   def eval_js(self, code):

      result = cursor()
      rc = sdbclient.eval_JS(self._client, result._cursor, code)
      pysequoiadb.check_error(rc)

      return rc, result

   @classmethod
   def backup_offline(self, options):
      """backup offline.

      """
      rc = sdbclient.backup_offline(self._client, options)
      pysequoiadb.check_error(rc)

      return rc

   @classmethod
   def list_backup(self, options, condition = static_object,
                                  selector  = static_object,
                                  order_by  = static_object):
      """list all backup in current database.

         if success, a cursor object will be returned with error code,
         or cursor object is None.
      """
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
         del result
         result = None

      return rc, result

   @classmethod
   def list_task(self, condition = static_object, selector  = static_object,
                       order_by  = static_object, hint      = static_object):
      """list all tasks in current database.

         if success, a cursor object will be returned with error code,
         or cursor object is None.
      """
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
         del result
         result = None

      return rc, result

   @classmethod
   def wait_task(self, task_ids, num):
      """wait task.

      """
      rc = sdbclient.wait_task(self._client, task_ids, num)
      pysequoiadb.check_error(rc)

      return rc

   @classmethod
   def cancel_task(self, task_id, is_async):
      """cancel task.

      """
      rc = sdbclient.cancel_task(self._client, task_id, is_async)
      pysequoiadb.check_error(rc)

      return rc

   @classmethod
   def set_session_attri(self, options = static_object):
      """set session attribution.

      """
      bson_options = None
      if options is not None:
         bson_options = bson.BSON.encode(options)

      rc = sdbclient.set_session_attri(self._client, bson_options)
      pysequoiadb.check_error(rc)

      return rc

   @classmethod
   def close_all_cursors(self):
      """close all cursors.

      """
      rc = sdbclient.close_all_cursors(self._client)
      pysequoiadb.check_error(rc)

      return rc

   @classmethod
   def is_valid(self):
      """check current client is valid or not.

      """
      rc, valid = sdbclient.is_valid(self._client)
      pysequoiadb.check_error(rc)
      if const.SDB_OK != rc:
         valid = False

      return valid
