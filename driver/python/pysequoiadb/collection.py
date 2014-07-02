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

"""Module of collection for python driver of SequoiaDB
"""

try:
   import sdbcl
except ImportError:
   raise Exception("cannot find C module file: sdbcl")

import bson
from bson.objectid import ObjectId
import pysequoiadb
from pysequoiadb import ( static_object,
                          default_host,
                          default_port,
                          default_user,
                          default_psw )

from pysequoiadb.cursor import cursor
from pysequoiadb import error
from pysequoiadb.common import const
from pysequoiadb.error import SequoiaDBError

class collection(object):
   """Collection for SequoiaDB"""

   def __init__(self):
      """create a new collection.

      """
      try:
         self._cl = sdbcl.create_cl()
      except SystemError:
         pysequoiadb.check_error(const.SDM_OOM)
         raise SequoiaDBError

   def __del__(self):
      """delete a object existed.
      
      """
      if self._cl is not None:
         rc = sdbcl.release_cl(self._cl)
         pysequoiadb.check_error(rc)
         self._cl = None

   def __repr__(self):

      return "Collection:%s" %(self.get_full_name())

   def get_count(self, condition = static_object):
      """Get the count of matching documents in current collection.

      Parameters:
              Name         Type     Info:
         [in] condition    dict     The matching rule, return the count of all
                                    documents if None.
      Return values:
         Success: SDB_OK   and   count of result
         Fail   : Others   and   0
      """
      bson_condition = None
      if condition is not None:
         bson_condition = bson.BSON.encode(condition)

      rc, count = sdbcl.get_count(self._cl, bson_condition)
      pysequoiadb.check_error(rc)

      if const.SDB_OK != rc:
         count = 0

      return rc, count

   def split_by_condition(self, source_group_name, target_group_name,
                                split_condition,
                                split_end_condition = static_object):
      """Split the specified collection from source replica group to target
         replica group by range.

      Parameters:
              Name                  Type  Info:
         [in] source_group_name     str   The source replica group name.
         [in] target_group_name     str   The target replica group name.
         [in] split_condition       dict  The matching rule, return the count of
                                          all documents if None.
         [in] split_end_condition   dict  The split end condition or None.
                                          eg:
                                          If we create a collection with the 
                                          option { ShardingKey:{"age":1},
                                          ShardingType:"Hash",Partition:2^10 },
                                          we can fill {age:30} as the
                                          splitCondition, and fill {age:60} 
                                          as the splitEndCondition. when split,
                                          the target replica group will get the
                                          records whose age's hash value are in
                                          [30,60). If splitEndCondition is null,
                                          they are in [30,max).
      Return values:
         Success: SDB_OK
         Fail   : Others
      """
      if not isinstance(source_group_name, basestring):
         raise TypeError("source group name must be an instance of basestring")
      if not isinstance(target_group_name, basestring):
         raise TypeError("target group name must be an instance of basestring")

      bson_split_condition = bson.BSON.encode(split_condition)
      bson_end_condition = None

      if split_end_condition is not None:
         bson_end_condition = bson.BSON.encode(split_end_condition)

      rc = sdbcl.split_by_condition(self._cl, source_group_name,
                                              target_group_name,
                                              bson_split_condition,
                                              bson_end_condition)
      pysequoiadb.check_error(rc)

      return rc

   def split_by_precent(self, source_group_name, target_group_name, precent):
      """Split the specified collection from source replica group to target
         replica group by percent.
      
      Parameters:
              Name               Type     Info:
         [in] source_group_name  str      The source replica group name.
         [in] target_group_name  str      The target replica group name.
         [in]percent	            float    The split percent, Range:(0,100]
      Return values:
         Success: SDB_OK
         Fail   : Others
      """
      if not isinstance(source_group_name, basestring):
         raise TypeError("source group name must be an instance of basestring")
      if not isinstance(target_group_name, basestring):
         raise TypeError("target group name must be an instance of basestring")
      if not isinstance(precent, float):
         raise TypeError("precent must be an instance of float")

      rc = sdbcl.split_by_precent(self._cl, source_group_name,
                                            target_group_name, precent)
      pysequoiadb.check_error(rc)

      return rc

   def split_async_by_condition(self, source_group_name, target_group_name,
                         split_condition, split_end_condition = static_object):
      """Split the specified collection from source replica group to target
         replica group by range.
      
      Parameters:
              Name                  Type  Info:
         [in] source_group_name     str   The source replica group name.
         [in] target_group_name     str   The target replica group name.
         [in] split_condition       dict  The matching rule, return the count of
                                          all documents if None.
         [in] split_end_condition   dict  The split end condition or None.
                                          eg:
                                          If we create a collection with the 
                                          option { ShardingKey:{"age":1},
                                          ShardingType:"Hash",Partition:2^10 },
                                          we can fill {age:30} as the
                                          splitCondition, and fill {age:60} 
                                          as the splitEndCondition. when split,
                                          the target replica group will get the
                                          records whose age's hash value are in
                                          [30,60). If splitEndCondition is null,
                                          they are in [30,max).
      Return values:
         Success: SDB_OK  and  the task id
         Fail   : Others  and  0
      """
      if not isinstance(source_group_name, basestring):
         raise TypeError("source group name must be an instance of basestring")
      if not isinstance(target_group_name, basestring):
         raise TypeError("target group name must be an instance of basestring")

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

   def split_async_by_precent(self, source_group_name, target_group_name,
                                                       precent):
      """Split the specified collection from source replica group to target
         replica group by percent.
      
      Parameters:
              Name               Type     Info:
         [in] source_group_name  str      The source replica group name.
         [in] target_group_name  str      The target replica group name.
         [in]percent	            float    The split percent, Range:(0,100]
      Return values:
         Success: SDB_OK  and  the task id
         Fail   : Others  and  0
      """
      if not isinstance(source_group_name, basestring):
         raise TypeError("source group name must be an instance of basestring")
      if not isinstance(target_group_name, basestring):
         raise TypeError("target group name must be an instance of basestring")
      if not isinstance(precent, float):
         raise TypeError("precent must be an instance of float")

      rc, task_id = sdbcl.splite_async_by_precent(self._cl, source_group_name,
                                                  target_group_name, precent)
      pysequoiadb.check_error(rc)

      if const.SDB_OK != rc:
         task_id = 0

      return rc, task_id

   def bulk_insert(self, flags, records):
      """Insert a bulk of record into current collection.
      
      Parameters:
              Name      Type     Info:
         [in] flags     int      0 or 1, see Info as below.
         [in] records   list     The list of inserted records.
      Return values:
         Success: SDB_OK
         Fail   : Others
      Info:
         flags : 0 or 1. 
         0 : stop insertting record when an error occurred
         1 : continue insertting records even though error occurred
      """
      if not isinstance(flags, int):
         raise TypeError("flags must be an instance of int")
      if not isinstance(records, list):
         raise TypeError("records must be an instance of list")

      container = []
      for elem in records :
         record = bson.BSON.encode( elem )
         container.append( record )

      rc = sdbcl.bulk_insert(self._cl, flags, container)
      pysequoiadb.check_error(rc)

      return rc

   def insert(self, record, oid = None):
      """Insert a record into current collection.

      Parameters:
              Name      Type           Info:
         [in] records   dict           The inserted record.
        [out] oid       bson.ObjectId  The object id of inserted bson object in
                                       current collection.
      Return values:
         Success: SDB_OK
         Fail   : Others
      """
      if not isinstance(record, dict):
         raise TypeError("record must be an instance of dict")
      if oid is not None and not isinstance(oid, ObjectId):
         raise TypeError("oid must be an instance of bson.ObjectId")

      bson_record = bson.BSON.encode(record)
      rc = sdbcl.insert(self._cl, bson_record, oid)
      pysequoiadb.check_error(rc)

      return rc

   def update(self, rule, condition = static_object, hint = static_object):
      """Update the matching documents in current collection.

      Parameters:
              Name      Type  Info:
         [in] rule      dict  The updating rule.
         [in] condition dict  The matching rule, update all the documents
                              if not provided.
         [in] hint      dict  The hint, automatically match the optimal hint
                              if not provided
      Return values:
         Success: SDB_OK
         Fail   : Others
      Note:
         It won't work to update the "ShardingKey" field, but the other fields
         take effect.
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

   def upsert(self, rule, condition = static_object, hint = static_object):
      """Update the matching documents in current collection, insert if
         no matching.

      Parameters:
              Name      Type  Info:
         [in] rule      dict  The updating rule.
         [in] condition dict  The matching rule, update all the documents
                              if not provided.
         [in] hint      dict  The hint, automatically match the optimal hint
                              if not provided
      Return values:
         Success: SDB_OK
         Fail   : Others
      Note:
         It won't work to update the "ShardingKey" field, but the other fields
         take effect.
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

   def delete(self, condition = static_object, hint = static_object):
      """Delete the matching documents in current collection.

      Parameters:
              Name      Type  Info:
         [in] condition dict  The matching rule, delete all the documents
                              if not provided.
         [in] hint      dict  The hint, automatically match the optimal hint
                              if not provided
      Return values:
         Success: SDB_OK
         Fail   : Others
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

   def query(self, condition   = static_object, selected = static_object,
                   order_by    = static_object, hint     = static_object,
                   num_to_skip = 0L, num_to_return = -1L):
      """Get the matching documents in current collection.

      Parameters:
              Name            Type  Info:
         [in] condition       dict  The matching rule, update all the documents
                                    if not provided.
         [in] selected        dict  The selective rule, return the whole
                                    document if not provided.
         [in] order_by        dict  The ordered rule, result set is unordered
                                    if not provided.
         [in] hint            dict  The hint, automatically match the optimal
                                    hint if not provided.
         [in] num_to_skip     long  Skip the first numToSkip documents,
                                    default is 0L.
         [in] num_to_return   long  Only return numToReturn documents, default
                                    is -1L for returning all results.
      Return values:
         Success: SDB_OK   and   a cursor object of query
         Fail   : Others   and   None
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

   def create_index(self, index_def, idx_name, is_unique, is_enforced):
      """Create the index in current collection.
         more info: http://www.sequoiadb.com/document/1.8/index.html

      Parameters:
              Name         Type  Info:
         [in] index_def    dict  The dict object of index element.
                                 e.g. {name:1, age:-1}
         [in] idx_name     str   The index name.
         [in] is_unique    bool  Whether the index elements are unique or not.
         [in] is_enforced  bool  Whether the index is enforced unique This
                                 element is meaningful when isUnique is set to
                                 true.
      Return values:
         Success: SDB_OK
         Fail   : Others
      """
      if not isinstance(idx_name, basestring):
         raise TypeError("index name must be an instance of basestring")
      if not isinstance(is_unique, bool):
         raise TypeError("is_unique must be an instance of bool")
      if not isinstance(is_enforced, bool):
         raise TypeError("is_enforced must be an instance of bool")

      unique = 0
      enforce = 0
      bson_index_def = None;
      bson_index_def = bson.BSONOBJ.encode(index_def)
      if is_unique:
         unique = 1

      if is_enforced:
         enforced = 1

      rc = sdbcl.create_index(self._cl, bson_index_def, idx_name,
                                        is_unique, is_enforced)
      pysequoiadb.check_error(rc)

      return rc

   def get_indexes(self, idx_name):
      """Get all of or one of the indexes in current collection.

      Parameters:
              Name         Type  Info:
         [in] idx_name     str   The index name, returns all of the indexes
                                 if this parameter is ""(empty str).
      Return values:
         Success: SDB_OK  and  a cursor object of result
         Fail   : Others  and  None
      """
      if not isinstance(idx_name, basestring):
         raise TypeError("index name must be an instance of basestring")

      result = cursor()
      rc = sdbcl.get_indexes(self._cl, result._cursor, idx_name)
      pysequoiadb.check_error(rc)

      if const.SDB_OK != rc:
         del result
         result = None

      return rc, result

   def drop_index(self, idx_name):
      """The index name.

      Parameters:
              Name         Type  Info:
         [in] idx_name     str   The index name.
      Return values:
         Success: SDB_OK
         Fail   : Others
      """
      if not isinstance(idx_name, basestring):
         raise TypeError("index name must be an instance of basestring")

      rc = sdbcl.drop_index(self._cl, idx_name)
      pysequoiadb.check_error(rc)

      return rc

   def get_collection_name(self):
      """Get the name of specified collection in current collection space.

      Parameters:
              Name         Type  Info:
         N/A
      Return values:
         The name of specified collection
      """
      _, cl_name = sdbcl.get_collection_name(self._cl)
      pysequoiadb.check_error(_)

      return cl_name

   def get_cs_name(self):
      """Get the name of current collection space.

      Parameters:
              Name         Type  Info:
         N/A
      Return values:
         The name of current collection space
      """
      _, cs_name = sdbcl.get_collection_space_name(self._cl)
      pysequoiadb.check_error(_)

      return cs_name

   def get_full_name(self):
      """Get the full name of specified collection in current collection space.

         +--------+--------+------------+
         |   db   |   cs   |     cl     |
         +--------+--------+------------+
         | localdb. sports . basketball |
         +--------+--------+------------+
         |        |        |            |
         +--------+--------+------------+
      Parameters:
              Name         Type  Info:
         N/A
      Return values:
         The full name of current collection
      """
      _, full_name = sdbcl.get_full_name(self._cl)
      pysequoiadb.check_error(_)

      return full_name

   def aggregate(self, aggregate_options):
      """Execute aggregate operation in specified collection.

      Parameters:
              Name               Type  Info:
         [in] aggregate_options  list  The array of dict objects.
      Return values:
         Success: SDB_OK  and  a cursor object of result
         Fail   : Others  and  None
      """
      container = []
      for option in aggregate_options :
         bson_option = bson.BSON.encode( option )
         container.append( bson_option )

      result = cursor()
      rc = sdbcl.aggregate(self._cl, result._cursor, container)
      pysequoiadb.check_error(rc)

      if const.SDB_OK != rc:
         del result
         result = None

      return rc, result

   def get_query_meta(self, condition = static_object,
                            order_by  = static_object,
                            hint      = static_object,
                            num_to_skip = 0L, num_to_return = -1L):
      """Get the index blocks' or data blocks' infomations for concurrent query.

      Parameters:
              Name            Type  Info:
         [in] condition       dict  The matching rule, return the whole range of
                                    index blocks if not provided.
                                    eg:{"age":{"$gt":25},"age":{"$lt":75}}.
         [in] order_by        dict  The ordered rule, result set is unordered
                                    if not provided.
         [in] hint            dict  One of the indexs in current collection,
                                    using default index to query if not provided
                                    eg:{"":"ageIndex"}.
         [in] num_to_skip     long  Skip the first num_to_skip documents,
                                    default is 0L.
         [in] num_to_return   long  Only return num_to_return documents, default
                                    is -1L for returning all results.
      Return values:
         Success: SDB_OK   and   a cursor object of query
         Fail   : Others   and   None
      """
      if not isinstance(num_to_skip, long):
         raise TypeError("number to skip must be an instance of long")
      if not isinstance(num_to_return, long):
         raise TypeError("number to return must be an instance of long")

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

   def attach_collection(self, cl_full_name, options):
      """Attach the specified collection.

      Parameters:
              Name            Type  Info:
         [in] subcl_full_name str   The name fo the subcollection.
         [in] options         dict  he low boudary and up boudary
                                    eg: {"LowBound":{a:1},"UpBound":{a:100}}
      Return values:
         Success: SDB_OK
         Fail   : Others
      """
      if not isinstance(cl_full_name, basestring):
         raise TypeError("full name of subcollection must be an instance of basestring")

      bson_options = None
      if options is not None:
         bson_options = bson.BSON.encode(options)

      rc = sdbcl.attach_collection(self._cl, cl_full_name, bson_options)
      pysequoiadb.check_error(rc)

      return rc

   def detach_collection(self, sub_cl_full_name):
      """Dettach the specified collection.

      Parameters:
              Name            Type  Info:
         [in] subcl_full_name str   The name fo the subcollection.
      Return values:
         Success: SDB_OK
         Fail   : Others
      """
      if not isinstance(sub_cl_full_name, basestring):
         raise TypeError("name of subcollection must be an instance of basestring")

      rc = sdbcl.detach_collection(self._cl, sub_cl_full_name)
      pysequoiadb.check_error(rc)

      return rc
