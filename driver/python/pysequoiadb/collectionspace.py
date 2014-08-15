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

"""Module of collectionspace for python driver of SequoiaDB
"""

try:
   import sdbcs
except ImportError:
   try:
      import libsdbcs
   except ImportError:
      raise Exception("cannot find C module file: sdbcs")

import bson
import pysequoiadb
from pysequoiadb import ( static_object,
                          default_host,
                          default_svcname,
                          default_user,
                          default_psw )
from pysequoiadb.collection import collection
from pysequoiadb.cursor import cursor
from pysequoiadb import error
from pysequoiadb.common import const
from pysequoiadb.error import SequoiaDBError

class collectionspace(object):
   """CollectionSpace for SequoiaDB
   
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
      """invoked when a new object is producted.

      """
      #'cs' is short for collection space
      try:
         self._cs = sdbcs.create_cs()
      except SystemError:
         pysequoiadb.check_error(const.SDM_OOM)
         raise SequoiaDBError

   def __del__(self):
      """delete a object existed.

      """
      if self._cs is not None:
         rc = sdbcs.release_cs(self._cs)
         pysequoiadb.check_error(rc)
         self._cs = None

   def __repr__(self):

      return "Collection Space %s" %(self.get_collection_space_name())

   def __getattr__(self, name):
      """support client.cs to access to collection.

         eg.
         cc = client()
         cs = cc.test
         cl = cs.test_cl  # access to collection named 'test_cl'

         and we should pass '__members__' and '__methods__',
         becasue dir(cc) will invoke __getattr__("__members__") and
         __getattr__("__methods__").

         if success, a collection object will be returned, or None.
      """
      if '__members__' == name or '__methods__' == name:
         pass
      else:
         cl = collection()
         rc = sdbcs.get_collection(self._cs, name, cl._cl)
         pysequoiadb.check_error(rc)
         if const.SDB_OK != rc:
            del cl;
            cl = None
         return cl

   def __getitem__(self, name):
      """support [] to access to collection.
      
         eg.
         cc = client()
         cs = cc['test']
         cl = cs['test_cl']# access to collection named 'test_cl'.
      """
      return self.__getattr__(name)

   def get_collection(self, cl_name):
      """Get the named collection.
         
      Parameters:
         Name         Type     Info:
         cl_name      str      The full name of the collection..
      Return values:
         Success: SDB_OK  and  a collection object of query
         Fail   : Others  and  None
      """
      if not isinstance(cl_name, basestring):
         raise TypeError("collection must be an instance of basestring")

      cl = collection()
      rc = sdbcs.get_collection(self._cs, cl_name, cl._cl)
      pysequoiadb.check_error(rc)

      if const.SDB_OK != rc:
         del cl
         cl = None

      return rc, cl

   def create_collection(self, cl_name, options = static_object):
      """create a collection using name and options.

      Parameters:
         Name      Type     Info:
         cl_name   str      The collection name.
         options   str      The options for creating collection, including
                                  "ShardingKey", "ReplSize", "IsMainCL" and
                                  "Compressed" informations, no options, if None.
      Return values:
         Success: SDB_OK  and  a collection object created
         Fail   : Others  and  None

      """
      if not isinstance(cl_name, basestring):
         raise TypeError("collection must be an instance of basestring")

      bson_options = None
      if options is not None:
         bson_options = bson.BSON.encode(options)

      cl = collection()
      if options is None:
         rc = sdbcs.create_collection(self._cs, cl_name, cl._cl)
      else:
         rc = sdbcs.create_collection_use_opt(self._cs, cl_name,
                                              bson_options, cl._cl)
      pysequoiadb.check_error(rc)

      if const.SDB_OK != rc:
         del cl
         cl = None

      return rc, cl

   def drop_collection(self, cl_name):
      """Drop the specified collection in current collection space.

      Parameters:
         Name      Type     Info:
         cl_name   str      The collection name.
      Return values:
         Success: SDB_OK
         Fail   : Others
      """
      if not isinstance(cl_name, basestring):
         raise TypeError("collection must be an instance of basestring")

      rc = sdbcs.drop_collection(self._cs, cl_name)
      pysequoiadb.check_error(rc)

      return rc

   def get_collection_space_name(self):
      """Get the current collection space name.

      Parameters:
         Name      Type     Info:
         N/A
      Return values:
         The name of current collection space.
      """
      _, cs_name = sdbcs.get_collection_space_name(self._cs)
      pysequoiadb.check_error(_)

      return cs_name
