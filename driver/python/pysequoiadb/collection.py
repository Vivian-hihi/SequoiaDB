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

import sdbcollection
import bson
from pysequoiadb import ( static_object,
                          default_host,
                          default_port,
                          default_user,
                          default_psw,
                          driver_version )
from pysequoiadb import cursor
from pysequoiadb import error

class collection(object):
    """Collection for SequoiaDB"""

    def __init__(self):
        self._cl = sdbcollection.create_cl()

    def __del__(self):
        if self._cl is not None:
            sdbcollection.release_cl(self._cl)
            self._cl = None

    def get_count(self, condition = static_object):
        rc, count = sdbcollection.get_count(self._cl, condition)
        if rc:
            pass

    def split(self, source_group_name, target_group_name,
                    split_condition, split_end_contiditon = static_object):
        rc = sdbcollection.split(self._cl, source_group_name, target_group_name,
                                  split_condition, split_end_contiditon)
        if rc:
            pass

    def split(self, source_group_name, target_group_name, precent):
        rc = sdbcollection.split(self._cl, source_group_name, target_group_name, precent)
        if rc:
            pass

    def split_async(self, source_group_name, target_group_name,
                                   split_condition,
                                   split_end_conditon = static_object):
        rc, task_id = sdbcollection.split_async_by_condition(self._cl,
                                                     source_group_name,
                                                     target_group_name,
                                                     split_condition,
                                                     split_end_conditon)
        if rc:
            pass
        return task_id

    def split_async(self, source_group_name, target_group_name, percent):
        rc, task_id = sdbcollection.splite_async_by_precent(self._cl, source_group_name,
                                                             target_group_name,
                                                             percent)
        if rc:
            pass
        return task_id

    def bulk_insert(self, flags, *obj):
        rc =sdbcollection.bulk_insert(self._cl, flags, obj)
        if rc:
            pass

    def insert(self, obj, oid = None):
        rc = sdbcollection.insert(self._cl, obj, oid)
        if rc:
            pass

    def update(self, rule, condition = static_object, hint = static_object):
        rc = sdbcollection.update(self._cl, rule, condition, hint)
        if rc:
            pass

    def upsert(self, rule, condition = static_object, hint = static_object):
        rc = sdbcollection.upsert(self._cl, rule, condition, hint)
        if rc:
            pass

    def delete(self, condition = static_object, hint = static_object):
        rc = sdbcollection.delete(self._cl, condition, hint)
        if rc:
            pass

    def query(self, condition = static_object, selected = static_object,
                            order_by = static_object, hint = static_object,
                            num_to_skip = 0, num_to_return = -1):
        result = cursor()
        rc = sdbcollection.query(self._cl, result._cursor, condition, selected,
                                   order_by, hint, num_to_skip, num_to_return)
        if rc:
            pass
        return result

    def create_index(self, idx_name, is_unique, is_enforced):
        rc = sdbcollection.create_index(self._cl, idx_name, is_unique, is_enforced)
        if rc:
            pass

    def get_indexes(self, idx_name):
        result = cursor()
        rc = sdbcollection.get_indexes(self._cl, result._cursor, idx_name)
        if rc:
            pass
        return result

    def drop_index(self, idx_name):
        rc = sdbcollection.drop_index(self._cl, idx_name)
        if rc:
            pass

    def get_collection_name(self):
        _, cl_name = sdbcollection.get_collection_name(self._cl)
        if _ is not 0:
            pass
        return cl_name

    def get_cs_name(self):
        _, cs_name = sdbcollection.get_cs_name(self._cl)
        if _ is not 0:
            pass
        return cs_name

    def get_full_name(self):
        _, full_name = sdbcollection.gey_full_name(self._cl)
        if _ is not 0:
            pass
        return full_name

    def aggregate(self, obj):
        result = cursor()
        rc = sdbcollection.aggregate(self._cl, result._cursor, obj)
        if rc:
            pass
        return result

    def get_query_meta(self, condition = static_object,
                                     order_by = static_object,
                                     hint = static_object,
                                     num_to_skip = 0, num_to_return = -1):
        result = cursor()
        rc = sdbcollection.get_query_meta(self._cl, result._cursor, condition, order_by, hint,
                                           num_to_skip, num_to_return)
        if rc:
            pass
        return result

    def attach_collection(self, cl_full_name, options):
        rc = sdbcollection.attach_collection(self._cl, cl_full_name, options)
        if rc:
            pass

    def detach_collection(self, sub_cl_full_name):
        rc = sdbcollection.detach_collection(self._cl, sub_cl_full_name)
        if rc:
            pass
