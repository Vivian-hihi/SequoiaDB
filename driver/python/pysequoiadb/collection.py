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

import sdbcl
import bson
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
      self._cl = sdbcl.create_cl()

   def __del__(self):
      if self._cl is not None:
         sdbcl.release_cl(self._cl)
      self._cl = None

   def get_count(self, condition = static_object):
      if condition is not None:
         bson_condition = bson.BSON.encode(condition)

      rc, count = sdbcl.get_count(self._cl, bson_condition)
      if const.SDB_OK != rc:
         count = 0
      return rc, count

   def split(self, source_group_name, target_group_name, split_condition,
                                          split_end_condition = static_object):
      if split_condition is not None:
         bson_split_condition = bson.BSON.encode(split_condition)
      if split_end_condition is not None:
         bson_end_condition = bson.BSON.encode(split_end_condition)

      rc = sdbcl.split(self._cl, source_group_name, target_group_name,
                                 bson_split_condition, bson_end_condition)
      return rc

   def split(self, source_group_name, target_group_name, precent):
      rc = sdbcl.split(self._cl, source_group_name, target_group_name, precent)
      return rc

   def split_async(self, source_group_name, target_group_name,
                           split_condition,
                           split_end_condition = static_object):

      bson_split_condition = bson.BSON.encode(split_condition)
      if split_end_condition is not None:
         bson_end_condition = bson.BSON.encode(split_end_condition)

      rc, task_id = sdbcl.split_async_by_condition(self._cl,
                                                   source_group_name,
                                                   target_group_name,
                                                   bson_split_condition,
                                                   bson_end_condition)
      if const.SDB_OK != rc:
         task_id = 0
      return rc, task_id

   def split_async(self, source_group_name, target_group_name, precent):
      rc, task_id = sdbcl.splite_async_by_precent(self._cl, source_group_name,
                                                  target_group_name, precent)
      if const.SDB_OK != rc:
         task_id = 0
      return rc, task_id

   def bulk_insert(self, flags, list):
      container = []
      for elem in list :
         bson = bson.BSON.encode( elem )
         container.append( bson )
      rc =sdbcl.bulk_insert(self._cl, flags, container)
      return rc

   def insert(self, obj, oid = None):
      bson_obj = bson.BSON.encode(obj)
      rc = sdbcl.insert(self._cl, bson_obj, oid)
      return rc

   def update(self, rule, condition = static_object, hint = static_object):
      bson_rule = bson.BSON.encode(rule)
      if condition is not None:
         bson_condition = bson.BSON.encode(condition)
      if hint is not None:
         bson_hint = bson.BSON.encode(hint)

      rc = sdbcl.update(self._cl, bson_rule, bson_condition, bson_hint)
      return rc

   def upsert(self, rule, condition = static_object, hint = static_object):
      bson_rule = bson.BSON.encode(rule)
      if condition is not None:
         bson_condition = bson.BSON.encode(condition)
      if hint is not None:
         bson_hint = bson.BSON.encode(hint)
      rc = sdbcl.upsert(self._cl, bson_rule, bson_condition, bson_hint)
      return rc

   def delete(self, condition = static_object, hint = static_object):
      bson_rule = bson.BSON.encode(rule)
      if condition is not None:
         bson_condition = bson.BSON.encode(condition)
      if hint is not None:
         bson_hint = bson.BSON.encode(hint)
      rc = sdbcl.delete(self._cl, bson_condition, bson_hint)
      return rc

   def query(self, condition   = static_object, selected = static_object,
                   order_by    = static_object, hint     = static_object,
                   num_to_skip = 0, num_to_return = -1):
      bson_condition = None
      if condition is not None:
         bson_condition = bson.BSON.encode(condition)
      bson_selected = None
      if selected is not None:
         bson_selected = bson.BSON.encode(selected)
      bson_order_by = None
      if order_by is not None:
         bson_order_by = bson.BSON.encode(order_by)
      bson_hint = None
      if hint is not None:
         bson_hint = bson.BSON.encode(hint)
      result = cursor()
      rc = sdbcl.query(self._cl, result._cursor, bson_condition, bson_selected,
                                                 bson_order_by, bson_hint,
                                                 num_to_skip, num_to_return)
      if const.SDB_OK != rc:
         result = None
      return rc, result

   def create_index(self, idx_name, is_unique, is_enforced):
      rc = sdbcl.create_index(self._cl, idx_name, is_unique, is_enforced)
      return rc

   def get_indexes(self, idx_name):
      result = cursor()
      rc = sdbcl.get_indexes(self._cl, result._cursor, idx_name)
      if const.SDB_OK != rc:
         result = None
      return rc, result

   def drop_index(self, idx_name):
      rc = sdbcl.drop_index(self._cl, idx_name)
      return rc

   def get_collection_name(self):
      _, cl_name = sdbcl.get_collection_name(self._cl)
      return cl_name

   def get_cs_name(self):
      _, cs_name = sdbcl.get_cs_name(self._cl)
      return cs_name

   def get_full_name(self):
      _, full_name = sdbcl.gey_full_name(self._cl)
      return full_name

   def aggregate(self, list):
      container = []
      for elem in list :
         bson = bson.BSON.encode( elem )
         container.append( bson )

      result = cursor()
      rc = sdbcl.aggregate(self._cl, result._cursor, container)
      if const.SDB_OK != rc:
         result = None
      return rc, result

   def get_query_meta(self, condition = static_object,
                            order_by  = static_object,
                            hint      = static_object,
                            num_to_skip = 0, num_to_return = -1):
      if condition is not None:
         bson_condition = bson.BSON.encode(condition)
      if order_by is not None:
         bson_order_by = bson.BSON.encode(order_by)
      if hint is not None:
         bson_hint = bson.BSON.encode(hint)
      result = cursor()
      rc = sdbcl.get_query_meta(self._cl, result._cursor, condition,
                                order_by, hint, num_to_skip, num_to_return)
      if const.SDB_OK != rc:
         result = None
      return rc, result

   def attach_collection(self, cl_full_name, options):
      if options is not None:
         bson_options = bson.BSON.encode(options)
      rc = sdbcl.attach_collection(self._cl, cl_full_name, bson_options)
      return rc

   def detach_collection(self, sub_cl_full_name):
      rc = sdbcl.detach_collection(self._cl, sub_cl_full_name)
      return rc
