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
   try:
      import libsdbcl
   except ImportError:
      raise Exception("cannot find C module file: sdbcl")

import bson
from bson.objectid import ObjectId
import pysequoiadb
from pysequoiadb import EMPTY_BSON

from pysequoiadb.cursor import cursor
from pysequoiadb import error
from pysequoiadb.common import const
from pysequoiadb.error import (SequoiaDBError, InvalidParameter)

class collection(object):
   """Collection for SequoiaDB
   
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

   def __init__(self):
      """create a new collection.

      Exceptions:
         pysequoiadb.error.SequoiaDBError
      """
      try:
         self._cl = sdbcl.create_cl()
      except SystemError:
         raise SequoiaDBError("Failed to alloc collection", const.SDB_OOM)

   def __del__(self):
      """delete a object existed.
      
      Exceptions:
         pysequoiadb.error.SequoiaDBError
      """
      if self._cl is not None:
         try:
            rc = sdbcl.release_cl(self._cl)
            pysequoiadb._raise_if_error("Failed to release collection", rc)
         except SequoiaDBError:
            raise

         self._cl = None

   def __repr__(self):

      return "Collection: %s" % (self.get_full_name())

   def get_count(self, condition = EMPTY_BSON):
      """Get the count of matching documents in current collection.

      Parameters:
         Name         Type     Info:
         condition    dict     The matching rule, return the count of all
                                     documents if None.
      Return values:
         count of result
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
         rc, count = sdbcl.get_count(self._cl, bson_condition)
         pysequoiadb._raise_if_error("Failed to get count of record", rc)
      except SequoiaDBError:
         count = 0
         raise

      return count

   def split_by_condition(self, source_group_name, target_group_name,
                                split_condition,
                                split_end_condition = EMPTY_BSON):
      """Split the specified collection from source replica group to target
         replica group by range.

      Parameters:
         Name                  Type     Info:
         source_group_name     str      The source replica group name.
         target_group_name     str      The target replica group name.
         split_condition       dict     The matching rule, return the count
                                              of all documents if None.
         split_end_condition   dict     The split end condition or None.
                                              eg:
                                              If we create a collection with the 
                                              option { ShardingKey:{"age":1},
                                              ShardingType:"Hash",Partition:2^10 },
                                              we can fill {age:30} as the
                                              splitCondition, and fill {age:60} 
                                              as the splitEndCondition. when
                                              split, the target replica group
                                              will get the records whose age's
                                              hash value are in [30,60).
                                              If splitEndCondition is null, they
                                              are in [30,max).
      Exceptions:
         pysequoiadb.error.InvalidParameter
         pysequoiadb.error.SequoiaDBError
      """
      if not isinstance(source_group_name, basestring):
         raise InvalidParameter("source group name must be an instance of basestring")
      if not isinstance(target_group_name, basestring):
         raise InvalidParameter("target group name must be an instance of basestring")

      bson_split_condition = None
      if split_condition is not None:
         if not isinstance(split_condition, dict):
            raise InvalidParameter("split condition must be an instance of dict")
         bson_split_condition = bson.BSON.encode(split_condition)

      bson_end_condition = None
      if split_end_condition is not None:
         if not isinstance(split_end_condition, dict):
            raise InvalidParameter("split end condition must be an instance of dict")
         bson_end_condition = bson.BSON.encode(split_end_condition)

      try:
         rc = sdbcl.split_by_condition(self._cl, source_group_name,
                                                 target_group_name,
                                                 bson_split_condition,
                                                 bson_end_condition)
         pysequoiadb._raise_if_error("Failed to split", rc)
      except SequoiaDBError:
         raise

   def split_by_percent(self, source_group_name, target_group_name, percent):
      """Split the specified collection from source replica group to target
         replica group by percent.
      
      Parameters:
         Name               Type     Info:
         source_group_name  str      The source replica group name.
         target_group_name  str      The target replica group name.
         percent	          float    The split percent, Range:(0,100]
      Exceptions:
         pysequoiadb.error.InvalidParameter
         pysequoiadb.error.SequoiaDBError
      """
      if not isinstance(source_group_name, basestring):
         raise InvalidParameter("source group name must be an instance of basestring")
      if not isinstance(target_group_name, basestring):
         raise InvalidParameter("target group name must be an instance of basestring")
      if not isinstance(percent, float):
         raise InvalidParameter("precent must be an instance of float")

      try:
         rc = sdbcl.split_by_percent(self._cl, source_group_name,
                                               target_group_name, percent)
         pysequoiadb._raise_if_error("Failed to split by precent", rc)
      except SequoiaDBError:
         raise

   def split_async_by_condition(self, source_group_name, target_group_name,
                         split_condition, split_end_condition = EMPTY_BSON):
      """Split the specified collection from source replica group to target
         replica group by range.
      
      Parameters:
         Name                  Type  Info:
         source_group_name     str   The source replica group name.
         target_group_name     str   The target replica group name.
         split_condition       dict  The matching rule, return the count of
                                           all documents if None.
         split_end_condition   dict  The split end condition or None.
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
         task id
      Exceptions:
         pysequoiadb.error.InvalidParameter
         pysequoiadb.error.SequoiaDBError
      """
      if not isinstance(source_group_name, basestring):
         raise InvalidParameter("source group name must be an instance of basestring")
      if not isinstance(target_group_name, basestring):
         raise InvalidParameter("target group name must be an instance of basestring")

      bson_split_condition = None
      if split_condition is not None:
         if not isinstance(split_condition, dict):
            raise InvalidParameter("split condition must be an instance of dict")
         bson_split_condition = bson.BSON.encode(split_condition)

      bson_end_condition = None
      if split_end_condition is not None:
         if not isinstance(split_end_condition, dict):
            raise InvalidParameter("split end condition must be an instance of dict")
         bson_end_condition = bson.BSON.encode(split_end_condition)

      try:
         rc, task_id = sdbcl.split_async_by_condition(self._cl,
                                                      source_group_name,
                                                      target_group_name,
                                                      bson_split_condition,
                                                      bson_end_condition)
         pysequoiadb._raise_if_error("Failed to split async", rc)

      except SequoiaDBError:
         task_id = 0
         raise

      return task_id

   def split_async_by_percent(self, source_group_name, target_group_name,
                                                       percent):
      """Split the specified collection from source replica group to target
         replica group by percent.
      
      Parameters:
         Name               Type     Info:
         source_group_name  str      The source replica group name.
         target_group_name  str      The target replica group name.
         percent	          float    The split percent, Range:(0,100]
      Return values:
         task id
      Exceptions:
         pysequoiadb.error.InvalidParameter
         pysequoiadb.error.SequoiaDBError
      """
      if not isinstance(source_group_name, basestring):
         raise InvalidParameter("source group name must be an instance of basestring")
      if not isinstance(target_group_name, basestring):
         raise InvalidParameter("target group name must be an instance of basestring")
      if not isinstance(percent, float):
         raise InvalidParameter("percent must be an instance of float")

      try:
         rc, task_id = sdbcl.splite_async_by_percent(self._cl,
                                                     source_group_name,
                                                     target_group_name,
                                                     percent)
         pysequoiadb._raise_if_error("Failed to split async", rc)
      except SequoiaDBError:
         task_id = 0
         raise

      return task_id

   def bulk_insert(self, flags, records):
      """Insert a bulk of record into current collection.
      
      Parameters:
         Name        Type       Info:
         flags       int        0 or 1, see Info as below.
         records     list/tuple The list of inserted records.
      Exceptions:
         pysequoiadb.error.InvalidParameter
         pysequoiadb.error.SequoiaDBError
      Info:
         flags : 0 or 1. 
         0 : stop insertting record when an error occurred
         1 : continue insertting records even though error occurred
      """
      if not isinstance(flags, int):
         raise InvalidParameter("flags must be an instance of int")

      container = []
      for elem in records :
         if not isinstance(elem, dict):
            raise InvalidParameter("record must be an instance of dict")
         record = bson.BSON.encode( elem )
         container.append( record )

      try:
         rc = sdbcl.bulk_insert(self._cl, flags, container)
         pysequoiadb._raise_if_error("Failed to insert records", rc)
      except SequoiaDBError:
         raise

   def insert(self, record):
      """Insert a record into current collection.

      Parameters:
         Name      Type    Info:
         records   dict    The inserted record.
      Return values:
         ObjectId of record inserted
      Exceptions:
         pysequoiadb.error.InvalidParameter
         pysequoiadb.error.SequoiaDBError
      """
      if not isinstance(record, dict):
         raise InvalidParameter("record must be an instance of dict")

      bson_record = bson.BSON.encode(record)
      try:
         rc, id_str = sdbcl.insert(self._cl, bson_record)
         pysequoiadb._raise_if_error("Failed to insert record", rc)
      except SequoiaDBError:
         raise

      oid = bson.ObjectId(id_str)
      return oid

   def update(self, rule, **kwargs):
      """Update the matching documents in current collection.

      Parameters:
         Name        Type     Info:
         rule        dict     The updating rule.
         **kwargs             Useful option are below
         - condition dict     The matching rule, update all the documents
                                    if not provided.
         - hint      dict     The hint, automatically match the optimal hint
                                    if not provided
      Exceptions:
         pysequoiadb.error.InvalidParameter
         pysequoiadb.error.SequoiaDBError
      Note:
         It won't work to update the "ShardingKey" field, but the other fields
               take effect.
      """
      if not isinstance(rule, dict):
         raise InvalidParameter("rule must be an instance of dict")

      bson_rule = bson.BSON.encode(rule)
      bson_condition = None
      bson_hint = None

      if "condition" in kwargs:
         if not isinstance(kwargs.get("condition"), dict):
            raise InvalidParameter("condition in kwargs must be an instance of dict")
         bson_condition = bson.BSON.encode(kwargs.get("condition"))
      if "hint" in kwargs:
         if not isinstance(kwargs.get("hint"), dict):
            raise InvalidParameter("hint in kwargs must be an instance of dict")
         bson_hint = bson.BSON.encode(kwargs.get("hint"))

      try:
         rc = sdbcl.update(self._cl, bson_rule, bson_condition, bson_hint)
         pysequoiadb._raise_if_error("Failed to update", rc)
      except SequoiaDBError:
         raise

   def upsert(self, rule, **kwargs):
      """Update the matching documents in current collection, insert if
         no matching.

      Parameters:
         Name        Type  Info:
         rule        dict  The updating rule.
         **kwargs          Useful options are below
         - condition dict  The matching rule, update all the documents
                                 if not provided.
         - hint      dict  The hint, automatically match the optimal hint
                                 if not provided
      Exceptions:
         pysequoiadb.error.InvalidParameter
         pysequoiadb.error.SequoiaDBError
      Note:
         It won't work to update the "ShardingKey" field, but the other fields
               take effect.
      """
      if not isinstance(rule, dict):
         raise InvalidParameter("rule must be an instance of dict")
      bson_rule = bson.BSON.encode(rule)

      bson_condition = None
      bson_hint = None

      if "condition" in kwargs:
         if not isinstance(kwargs.get("condition"), dict):
            raise InvalidParameter("condition must be an instance of dict")
         bson_condition = bson.BSON.encode(kwargs.get("condition"))
      if "hint" in kwargs:
         if not isinstance(kwargs.get("hint"), dict):
            raise InvalidParameter("hint must be an instance of dict")
         bson_hint = bson.BSON.encode(kwargs.get("hint"))

      try:
         rc = sdbcl.upsert(self._cl, bson_rule, bson_condition, bson_hint)
         pysequoiadb._raise_if_error("Failed to update", rc)
      except SequoiaDBError:
         raise

   def delete(self, **kwargs):
      """Delete the matching documents in current collection.

      Parameters:
         Name        Type  Info:
         **kwargs          Useful options are below
         - condition dict  The matching rule, delete all the documents
                                 if not provided.
         - hint      dict  The hint, automatically match the optimal hint
                                 if not provided
      Exceptions:
         pysequoiadb.error.InvalidParameter
         pysequoiadb.error.SequoiaDBError
      """
      bson_condition = None
      bson_hint = None

      if "condition" in kwargs:
         if not isinstance(kwargs.get("condition"), dict):
            raise InvalidParameter("condition must be an instance of dict")
         bson_condition = bson.BSON.encode(kwargs.get("condition"))
      if "hint" in kwargs:
         if not isinstance(kwargs.get("hint"), dict):
            raise InvalidParameter("hint must be an instance of dict")
         bson_hint = bson.BSON.encode(kwargs.get("hint"))

      try:
         rc = sdbcl.delete(self._cl, bson_condition, bson_hint)
         pysequoiadb._raise_if_error("Failed to delete", rc)
      except SequoiaDBError:
         raise

   def query(self, **kwargs):
      """Get the matching documents in current collection.

      Parameters:
         Name              Type     Info:
         **kwargs                   Useful options are below
         - condition       dict     The matching rule, update all the
                                          documents if not provided.
         - selected        dict     The selective rule, return the whole
                                          document if not provided.
         - order_by        dict     The ordered rule, result set is unordered
                                          if not provided.
         - hint            dict     The hint, automatically match the optimal
                                          hint if not provided.
         - num_to_skip     long     Skip the first numToSkip documents,
                                          default is 0L.
         - num_to_return   long     Only return numToReturn documents,
                                          default is -1L for returning
                                          all results.
      Return values:
         a cursor object of query
      Exceptions:
         pysequoiadb.error.InvalidParameter
         pysequoiadb.error.SequoiaDBError
      """

      bson_condition = None
      bson_selected = None
      bson_order_by = None
      bson_hint = None
      
      num_to_skip = 0L
      num_to_return = -1L

      if "condition" in kwargs:
         if not isinstance(kwargs.get("condition"), dict):
            raise InvalidParameter("condition must be an instance of dict")
         bson_condition = bson.BSON.encode(kwargs.get("condition"))
      if "selector" in kwargs:
         if not isinstance(kwargs.get("selector"), dict):
            raise InvalidParameter("selector must be an instance of dict")
         bson_hint = bson.BSON.encode(kwargs.get("selector"))
      if "order_by" in kwargs:
         if not isinstance(kwargs.get("order_by"), dict):
            raise InvalidParameter("order_by must be an instance of dict")
         bson_hint = bson.BSON.encode(kwargs.get("order_by"))
      if "hint" in kwargs:
         if not isinstance(kwargs.get("hint"), dict):
            raise InvalidParameter("hint must be an instance of dict")
         bson_hint = bson.BSON.encode(kwargs.get("hint"))
      if "num_to_skip" in kwargs:
         if not isinstance(kwargs.get("num_to_skip"), long):
            raise InvalidParameter("num_to_skip must be an instance of long")
         else:
            num_to_return = kwargs.get("num_to_skip")
      if "num_to_return" in kwargs:
         if not isinstance(kwargs.get("num_to_return"), long):
            raise InvalidParameter("num_to_return must be an instance of long")
         else:
            num_to_return = kwargs.get("num_to_return")

      try:
         result = cursor()
         rc = sdbcl.query(self._cl, result._cursor,
                          bson_condition, bson_selected,
                          bson_order_by, bson_hint,
                          num_to_skip, num_to_return)
         pysequoiadb._raise_if_error("Failed to query", rc)
      except SequoiaDBError:
         del result
         result = None
         raise

      return result

   def create_index(self, index_def, idx_name, is_unique, is_enforced):
      """Create the index in current collection.

      Parameters:
         Name         Type  Info:
         index_def    dict  The dict object of index element.
                                  e.g. {name:1, age:-1}
         idx_name     str   The index name.
         is_unique    bool  Whether the index elements are unique or not.
         is_enforced  bool  Whether the index is enforced unique This
                                  element is meaningful when isUnique is set to
                                  true.
      Exceptions:
         pysequoiadb.error.InvalidParameter
         pysequoiadb.error.SequoiaDBError
      """
      if not isinstance(index_def, dict):
         raise InvalidParameter("index definition must be an instance of dict")
      if not isinstance(idx_name, basestring):
         raise InvalidParameter("index name must be an instance of basestring")
      if not isinstance(is_unique, bool):
         raise InvalidParameter("is_unique must be an instance of bool")
      if not isinstance(is_enforced, bool):
         raise InvalidParameter("is_enforced must be an instance of bool")

      unique = 0
      enforce = 0
      bson_index_def = bson.BSON.encode(index_def)

      if is_unique:
         unique = 1
      if is_enforced:
         enforced = 1

      try:
         rc = sdbcl.create_index(self._cl, bson_index_def, idx_name,
                                           is_unique, is_enforced)
         pysequoiadb._raise_if_error("Failed to create index", rc)
      except SequoiaDBError:
         raise

   def get_indexes(self, idx_name = None):
      """Get all of or one of the indexes in current collection.

      Parameters:
         Name         Type  Info:
         idx_name     str   The index name, returns all of the indexes
                                  if this parameter is None.
      Return values:
         a cursor object of result
      Exceptions:
         pysequoiadb.error.InvalidParameter
         pysequoiadb.error.SequoiaDBError
      """
      if idx_name is not None and not isinstance(idx_name, basestring):
         raise InvalidParameter("index name must be an instance of basestring")
      if idx_name is None:
         idx_name = ""

      try:
         result = cursor()
         rc = sdbcl.get_index(self._cl, result._cursor, idx_name)
         pysequoiadb._raise_if_error("Failed to get indexes", rc)
      except SequoiaDBError:
         del result
         result = None
         raise

      return result

   def drop_index(self, idx_name):
      """The index name.

      Parameters:
         Name         Type  Info:
         idx_name     str   The index name.
      Exceptions:
         pysequoiadb.error.InvalidParameter
         pysequoiadb.error.SequoiaDBError
      """
      if not isinstance(idx_name, basestring):
         raise InvalidParameter("index name must be an instance of basestring")

      try:
         rc = sdbcl.drop_index(self._cl, idx_name)
         pysequoiadb._raise_if_error("Failed to drop index", rc)
      except SequoiaDBError:
         raise

   def get_collection_name(self):
      """Get the name of specified collection in current collection space.

      Return values:
         The name of specified collection
      Exceptions:
         pysequoiadb.error.SequoiaDBError
      """
      try:
         rc, cl_name = sdbcl.get_collection_name(self._cl)
         pysequoiadb._raise_if_error("Failed to get collection name", rc)
      except SequoiaDBError:
         raise

      return cl_name

   def get_cs_name(self):
      """Get the name of current collection space.

      Return values:
         The name of current collection space
      Exceptions:
         pysequoiadb.error.SequoiaDBError
      """
      try:
         rc, cs_name = sdbcl.get_collection_space_name(self._cl)
         pysequoiadb._raise_if_error("Failed to get collection space name", rc)
      except SequoiaDBError:
         raise

      return cs_name

   def get_full_name(self):
      """Get the full name of specified collection in current collection space.

      Return values:
         The full name of current collection
      Exceptions:
         pysequoiadb.error.SequoiaDBError
      """
      try:
         rc, full_name = sdbcl.get_full_name(self._cl)
         pysequoiadb._raise_if_error("Failed to get full name", rc)
      except SequoiaDBError:
         raise

      return full_name

   def aggregate(self, aggregate_options):
      """Execute aggregate operation in specified collection.

      Parameters:
         Name               Type       Info:
         aggregate_options  list/tuple The array of dict objects.
                                             bson.SON may need if the element is
                                             order-sensitive.
                                             eg.
                                             {'$sort':bson.SON([("name",-1), ("age":1)])}
                                             it will be ordered descending by 'name'
                                             first, and be ordered ascending by 'age'
      Return values:
         a cursor object of result
      Exceptions:
         pysequoiadb.error.SequoiaDBError
      """
      if not isinstace(aggregate_options, list):
         raise InvalidParameter("aggregate options must be an instance of list")

      container = []
      for option in aggregate_options :
         if not isinstace(option, dict):
            raise InvalidParameter("options must be an instance of dict")
         bson_option = bson.BSON.encode( option )
         container.append( bson_option )

      try:
         result = cursor()
         rc = sdbcl.aggregate(self._cl, result._cursor, container)
         pysequoiadb._raise_if_error("Failed to aggregate", rc)
      except SequoiaDBError:
         del result
         result = None
         raise

      return result

   def get_query_meta(self, **kwargs):
      """Get the index blocks' or data blocks' infomations for concurrent query.

      Parameters:
         Name              Type     Info:
         **kwargs                   Useful options are below
         - condition       dict     The matching rule, return the whole range
                                          of index blocks if not provided.
                                          eg:{"age":{"$gt":25},"age":{"$lt":75}}.
         - order_by        dict     The ordered rule, result set is unordered
                                          if not provided.bson.SON may need if it is
                                          order-sensitive.
         - hint            dict     One of the indexs in current collection,
                                          using default index to query if not
                                          provided.
                                          eg:{"":"ageIndex"}.
         - num_to_skip     long     Skip the first num_to_skip documents,
                                          default is 0L.
         - num_to_return   long     Only return num_to_return documents,
                                          default is -1L for returning all results.
      Return values:
         a cursor object of query
      Exceptions:
         pysequoiadb.error.InvalidParameter
         pysequoiadb.error.SequoiaDBError
      """
      num_to_skip = 0L
      num_to_return = -1L

      if "num_to_skip" in kwargs:
         if not isinstance(kwargs.get("num_to_skip"), long):
            raise InvalidParameter("number to skip must be an instance of long")
         else:
            num_to_skip = kwargs.get("num_to_skip")
      if "num_to_return" in kwargs:
         if not isinstance(kwargs.get("num_to_return"), long):
            raise InvalidParameter("number to return must be an instance of long")
         else:
            num_to_return = kwargs.get("num_to_return")

      bson_condition = None
      bson_order_by = None
      bson_hint = None
      if "condition" in kwargs:
         if not isinstance(kwargs.get("condition"), dict):
            raise InvalidParameter("condition must be an instance of dict")
         bson_condition = bson.BSON.encode(kwargs.get("condition"))
      if "order_by" in kwargs:
         if not isinstance(kwargs.get("order_by"), dict):
            raise InvalidParameter("order_by must be an instance of dict")
         bson_hint = bson.BSON.encode(kwargs.get("order_by"))
      if "hint" in kwargs:
         if not isinstance(kwargs.get("hint"), dict):
            raise InvalidParameter("hint must be an instance of dict")
         bson_hint = bson.BSON.encode(kwargs.get("hint"))

      try:
         result = cursor()
         rc = sdbcl.get_query_meta(self._cl, result._cursor, condition,
                                order_by, hint, num_to_skip, num_to_return)
         pysequoiadb._raise_if_error("Failed to query meta", rc)
      except SequoiaDBError:
         del result
         result = None
         raise

      return result

   def attach_collection(self, cl_full_name, options):
      """Attach the specified collection.

      Parameters:
         Name            Type  Info:
         subcl_full_name str   The name fo the subcollection.
         options         dict  he low boudary and up boudary
                                     eg: {"LowBound":{a:1},"UpBound":{a:100}}
      Exceptions:
         pysequoiadb.error.InvalidParameter
         pysequoiadb.error.SequoiaDBError
      """
      if not isinstance(cl_full_name, basestring):
         raise InvalidParameter("full name of subcollection must be \
                          an instance of basestring")
      if not isinstance(options, dict):
         raise InvalidParameter("options must be an instance of basestring")

      bson_options = None
      if options is not None:
         bson_options = bson.BSON.encode(options)

      try:
         rc = sdbcl.attach_collection(self._cl, cl_full_name, bson_options)
         pysequoiadb._raise_if_error("Failed to attach collection", rc)
      except SequoiaDBError:
         raise

   def detach_collection(self, sub_cl_full_name):
      """Dettach the specified collection.

      Parameters:
         Name            Type  Info:
         subcl_full_name str   The name fo the subcollection.
      Exceptions:
         pysequoiadb.error.InvalidParameter
         pysequoiadb.error.SequoiaDBError
      """
      if not isinstance(sub_cl_full_name, basestring):
         raise InvalidParameter("name of subcollection must be an instance of basestring")

      try:
         rc = sdbcl.detach_collection(self._cl, sub_cl_full_name)
         pysequoiadb._raise_if_error("Failed to detach collection", rc)
      except SequoiaDBError:
         raise
