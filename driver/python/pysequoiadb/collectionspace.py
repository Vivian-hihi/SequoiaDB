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

"""Module of collectionspace for python driver for SequoiaDB
"""

try:
   import sdbcs
except ImportError:
   raise Exception("cannot find C module file: sdbcs")

import bson
import pysequoiadb
from pysequoiadb import ( static_object,
                          default_host,
                          default_port,
                          default_user,
                          default_psw,
                          driver_version )
from pysequoiadb.collection import collection
from pysequoiadb.cursor import cursor
from pysequoiadb import error
from pysequoiadb.common import const

class collectionspace(object):
   """CollectionSpace for SequoiaDB"""
   
   def __init__(self):

      #'cs' is short for collection space
      try:
         self._cs = sdbcs.create_cs()
      except SystemError:
         pysequoiadb.check_error(const.SDM_OOM)

   def __del__(self):

      if self._cs is not None:
         rc = sdbcs.release_cs(self._cs)
         pysequoiadb.check_error(rc)
         self._cs = None

   def __getattr__(self, name):

      if '__members__' == name or '__methods__' == name:
         pass
      else:
         cl = collection()
         rc = sdbcs.get_collection(self._cs, name, cl._cl)
         pysequoiadb.check_error(rc)
         return cl

   def __getitem__(self, name):

      return self.__getattr__(name)

   def get_collection(self, cl_name):

      cl = collection()
      rc = sdbcs.get_collection(self._cs, cl_name, cl._cl)
      pysequoiadb.check_error(rc)

      if const.SDB_OK != rc:
         del cl
         cl = None

      return rc, cl

   def create_collection(self, cl_name, options = static_object):

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

      rc = sdbcs.drop_collection(self._cs, cl_name)
      pysequoiadb.check_error(rc)

      return rc

   def get_collection_space_name(self):

      _, cs_name = sdbcs.get_collection_space_name(self._cs)
      pysequoiadb.check_error(_)

      return cs_name
