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

"""Module of collection for python driver for SequoiaDB
"""

try:
   import sdbcl
except ImportError:
   raise Exception("cannot find C module file: sdbcl")

import bson
import pysequoiadb
from pysequoiadb import ( static_object,
                          default_host,
                          default_port,
                          default_user,
                          default_psw,
                          driver_version )

from pysequoiadb.cursor import cursor
from pysequoiadb import error
from pysequoiadb.common import const

class collection(object):
   """Collection for SequoiaDB"""

   def __init__(self):
      """create a new collection.

      """
      try:
         self._cl = sdbcl.create_cl()
      except SystemError:
         pysequoiadb.check_error(const.SDM_OOM)

   def __del__(self):
      """delete a object existed.
      
      """
      if self._cl is not None:
         rc = sdbcl.release_cl(self._cl)
         pysequoiadb.check_error(rc)
         self._cl = None

   @classmethod
   def get_count(self, condition = static_object):
      """get the count of documents by condition given.

         Total count will be returned with error code defaultly.
      """
      bson_condition = None
      if condition is not None:
         bson_condition = bson.BSON.encode(condition)

      rc, count = sdbcl.get_count(self._cl, bson_condition)
      pysequoiadb.check_error(rc)

      if const.SDB_OK != rc:
         count = 0

      return rc, count

   @classmethod
   def split_by_condition(self, source_group_name, target_group_name,
                                 split_condition,
                                 split_end_condition = static_object):
      """split the collection to target group, using contidition.
      
      """
      bson_split_condition = bson.BSON.encode(split_condition)
      bson_end_condition = None

      if split_end_condition is not None:
         bson_end_condition = bson.BSON.encode(split_end_condition)

      rc = sdbcl.split(self._cl, source_group_name, target_group_name,
                                 bson_split_condition, bson_end_condition)
      pysequoiadb.check_error(rc)

      return rc

   @classmethod
   def split(self, source_group_name, target_group_name, precent):
      """split the collection to target group, using precent.
      
      """
      rc = sdbcl.split(self._cl, source_group_name, target_group_name, precent)
      pysequoiadb.check_error(rc)

      return rc

   @classmethod
   def split_async_by_condition(self, source_group_name, target_group_name,
                         split_condition, split_end_condition = static_object):
      """split the collection to target group asynchronously, using contidition.
      
      """
      bson_split_condition = bson.BSON.encode(split_condition)
      bson_end_condition = None

      if split_end_condition is not None:
         bson_end_condition = bson.BSON.encode(split_end_condition)

      rc, task_id = sdbcl.split_async_by_condition(self._cl,
                                                   source_group_name,
                                                   target_group_name,
                                                   bson_split_condition,
                                                   bson_end_condition)
      pysequoiadb.check_error(rc)

      if const.SDB_OK != rc:
         task_id = 0

      return rc, task_id

   @classmethod
   def split_async_by_precent(self, source_group_name, target_group_name,
                                                       precent):
      """split the collection to target group asynchronously, using precent.
      
      """
      rc, task_id = sdbcl.splite_async_by_precent(self._cl, source_group_name,
                                                  target_group_name, precent)
      pysequoiadb.check_error(rc)

      if const.SDB_OK != rc:
         task_id = 0

      return rc, task_id

   @classmethod
   def bulk_insert(self, flags, list):
      """insert multi documents.
      
      """
      container = []
      for elem in list :
         bson = bson.BSON.encode( elem )
         container.append( bson )

      rc = sdbcl.bulk_insert(self._cl, flags, container)
      pysequoiadb.check_error(rc)

      return rc

   @classmethod
   def insert(self, obj, oid = None):
      """insert a ducuments once.

      """
      bson_obj = bson.BSON.encode(obj)
      rc = sdbcl.insert(self._cl, bson_obj, oid)
      pysequoiadb.check_error(rc)

      return rc

   @classmethod
   def update(self, rule, condition = static_object, hint = static_object):
      """update a document.

      """
      bson_rule = bson.BSON.encode(rule)
      bson_condition = None
      bson_hint = None

      if condition is not None:
         bson_condition = bson.BSON.encode(condition)
      if hint is not None:
         bson_hint = bson.BSON.encode(hint)

      rc = sdbcl.update(self._cl, bson_rule, bson_condition, bson_hint)
      pysequoiadb.check_error(rc)

      return rc

   @classmethod
   def upsert(self, rule, condition = static_object, hint = static_object):
      """update or insert.

         update if document exists, or insert the document
      """
      bson_rule = bson.BSON.encode(rule)
      bson_condition = None
      bson_hint = None

      if condition is not None:
         bson_condition = bson.BSON.encode(condition)
      if hint is not None:
         bson_hint = bson.BSON.encode(hint)

      rc = sdbcl.upsert(self._cl, bson_rule, bson_condition, bson_hint)
      pysequoiadb.check_error(rc)

      return rc

   @classmethod
   def delete(self, condition = static_object, hint = static_object):
      """delete document(s)

      """
      bson_condition = None
      bson_hint = None

      if condition is not None:
         bson_condition = bson.BSON.encode(condition)
      if hint is not None:
         bson_hint = bson.BSON.encode(hint)

      rc = sdbcl.delete(self._cl, bson_condition, bson_hint)
      pysequoiadb.check_error(rc)

      return rc

   @classmethod
   def query(self, condition   = static_object, selected = static_object,
                   order_by    = static_object, hint     = static_object,
                   num_to_skip = 0L, num_to_return = -1L):
      """query documents.

      """
      bson_condition = None
      bson_selected = None
      bson_order_by = None
      bson_hint = None

      if condition is not None:
         bson_condition = bson.BSON.encode(condition)
      if selected is not None:
         bson_selected = bson.BSON.encode(selected)
      if order_by is not None:
         bson_order_by = bson.BSON.encode(order_by)
      if hint is not None:
         bson_hint = bson.BSON.encode(hint)

      result = cursor()
      rc = sdbcl.query(self._cl, result._cursor, bson_condition, bson_selected,
                                                 bson_order_by, bson_hint,
                                                 num_to_skip, num_to_return)
      pysequoiadb.check_error(rc)

      if const.SDB_OK != rc:
         del result
         result = None

      return rc, result

   @classmethod
   def create_index(self, idx_name, is_unique, is_enforced):
      """create index for current collection.

         more info: http://www.sequoiadb.com/document/1.8/index.html
      """
      rc = sdbcl.create_index(self._cl, idx_name, is_unique, is_enforced)
      pysequoiadb.check_error(rc)

      return rc

   @classmethod
   def get_indexes(self, idx_name):
      """get index of current collection using name.

         if success, a cursor object will be returned with error code,
         or cursor object is None.
      """
      result = cursor()
      rc = sdbcl.get_indexes(self._cl, result._cursor, idx_name)
      pysequoiadb.check_error(rc)

      if const.SDB_OK != rc:
         del result
         result = None

      return rc, result

   @classmethod
   def drop_index(self, idx_name):
      """drop the index of the specified name.

      """
      rc = sdbcl.drop_index(self._cl, idx_name)
      pysequoiadb.check_error(rc)

      return rc

   @classmethod
   def get_collection_name(self):
      """get the name of current collection.

      """
      _, cl_name = sdbcl.get_collection_name(self._cl)
      pysequoiadb.check_error(_)

      return cl_name

   @classmethod
   def get_cs_name(self):
      """get the name of collection space, where the current collection located.

      """
      _, cs_name = sdbcl.get_collection_space_name(self._cl)
      pysequoiadb.check_error(_)

      return cs_name

   @classmethod
   def get_full_name(self):
      """the the full name of current collection.

         +--------+--------+------------+
         |   db   |   cs   |     cl     |
         +--------+--------+------------+
         | localdb. sports . basketball |
         +--------+--------+------------+
         |        |        |            |
         +--------+--------+------------+
      """
      _, full_name = sdbcl.get_full_name(self._cl)
      pysequoiadb.check_error(_)

      return full_name

   @classmethod
   def aggregate(self, list):
      """do aggregate

      """
      container = []
      for elem in list :
         bson = bson.BSON.encode( elem )
         container.append( bson )

      result = cursor()
      rc = sdbcl.aggregate(self._cl, result._cursor, container)
      pysequoiadb.check_error(rc)

      if const.SDB_OK != rc:
         del result
         result = None

      return rc, result

   @classmethod
   def get_query_meta(self, condition = static_object,
                            order_by  = static_object,
                            hint      = static_object,
                            num_to_skip = 0, num_to_return = -1):
      """query meta info of current collection

      """
      bson_condition = None
      bson_order_by = None
      bson_hint = None

      if condition is not None:
         bson_condition = bson.BSON.encode(condition)
      if order_by is not None:
         bson_order_by = bson.BSON.encode(order_by)
      if hint is not None:
         bson_hint = bson.BSON.encode(hint)

      result = cursor()
      rc = sdbcl.get_query_meta(self._cl, result._cursor, condition,
                                order_by, hint, num_to_skip, num_to_return)
      pysequoiadb.check_error(rc)

      if const.SDB_OK != rc:
         del result
         result = None

      return rc, result

   @classmethod
   def attach_collection(self, cl_full_name, options):
      """attach collection

      """
      bson_options = None
      if options is not None:
         bson_options = bson.BSON.encode(options)

      rc = sdbcl.attach_collection(self._cl, cl_full_name, bson_options)
      pysequoiadb.check_error(rc)

      return rc

   @classmethod
   def detach_collection(self, sub_cl_full_name):
      """detach collection
      """
      rc = sdbcl.detach_collection(self._cl, sub_cl_full_name)
      pysequoiadb.check_error(rc)

      return rc
