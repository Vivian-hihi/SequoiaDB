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
from pysequoiadb import cursor
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
      rc, count = sdbcl.get_count(self._cl, condition)
      return rc, count

   def split(self, source_group_name, target_group_name,
               split_condition, split_end_condition = static_object):
      rc = sdbcl.split(self._cl, source_group_name, target_group_name,
                          split_condition, split_end_condition)
      return rc

   def split(self, source_group_name, target_group_name, precent):
      rc = sdbcl.split(self._cl, source_group_name, target_group_name, precent)
      return rc

   def split_async(self, source_group_name, target_group_name,
                           split_condition,
                           split_end_condition = static_object):
      rc, task_id = sdbcl.split_async_by_condition(self._cl,
                                        source_group_name,
                                        target_group_name,
                                        split_condition,
                                        split_end_condition)
      return task_id

   def split_async(self, source_group_name, target_group_name, precent):
      rc, task_id = sdbcl.splite_async_by_precent(self._cl, source_group_name,
                                              target_group_name,
                                              precent)
      return task_id

   def bulk_insert(self, flags, *obj):
      rc =sdbcl.bulk_insert(self._cl, flags, obj)
      return rc

   def insert(self, obj, oid = None):
      rc = sdbcl.insert(self._cl, obj, oid)
      return rc

   def update(self, rule, condition = static_object, hint = static_object):
      rc = sdbcl.update(self._cl, rule, condition, hint)
      return rc

   def upsert(self, rule, condition = static_object, hint = static_object):
      rc = sdbcl.upsert(self._cl, rule, condition, hint)
      return rc

   def delete(self, condition = static_object, hint = static_object):
      rc = sdbcl.delete(self._cl, condition, hint)
      return rc

   def query(self, condition = static_object, selected = static_object,
                     order_by = static_object, hint = static_object,
                     num_to_skip = 0, num_to_return = -1):
      result = cursor()
      rc = sdbcl.query(self._cl, result._cursor, condition, selected,
                           order_by, hint, num_to_skip, num_to_return)
      return rc, result

   def create_index(self, idx_name, is_unique, is_enforced):
      rc = sdbcl.create_index(self._cl, idx_name, is_unique, is_enforced)
      return rc

   def get_indexes(self, idx_name):
      result = cursor()
      rc = sdbcl.get_indexes(self._cl, result._cursor, idx_name)
      return rc, result

   def drop_index(self, idx_name):
      rc = sdbcl.drop_index(self._cl, idx_name)
      return rc

   def get_collection_name(self):
      _, cl_name = sdbcl.get_collection_name(self._cl)
      if _ is not 0:
         pass
      return cl_name

   def get_cs_name(self):
      _, cs_name = sdbcl.get_cs_name(self._cl)
      if _ is not 0:
         pass
      return cs_name

   def get_full_name(self):
      _, full_name = sdbcl.gey_full_name(self._cl)
      if _ is not 0:
         pass
      return full_name

   def aggregate(self, obj):
      result = cursor()
      rc = sdbcl.aggregate(self._cl, result._cursor, obj)
      return rc, result

   def get_query_meta(self, condition = static_object,
                            order_by = static_object,
                            hint = static_object,
                            num_to_skip = 0, num_to_return = -1):
      result = cursor()
      rc = sdbcl.get_query_meta(self._cl, result._cursor, condition, order_by, hint,
                                 num_to_skip, num_to_return)
      return rc, result

   def attach_collection(self, cl_full_name, options):
      rc = sdbcl.attach_collection(self._cl, cl_full_name, options)
      return rc

   def detach_collection(self, sub_cl_full_name):
      rc = sdbcl.detach_collection(self._cl, sub_cl_full_name)
      return rc
