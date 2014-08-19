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

import pysequoiadb
from pysequoiadb import common
from pysequoiadb.common import const

class SDBBaseError(Exception):
   """Base Exception of Python Driver for SequoiaDB
   """

class SequoiaDBError(SDBBaseError):
   """Exception of SequoiaDB
   """
   DEFAULT = "  Error: "
   def __init__(self, errmsg, code = 0):

      if errmsg is None:
         self.__errmsg = self.DEFAULT
      else:
         self.__errmsg = errmsg

      if code != 0:
         self.__code = code
         self.__details = common.get_info(code)
      SDBBaseError.__init__(self, errmsg)

   def __repr__(self):
      """make the error info.
      """
      return self.__errmsg

   def __detail(self):
      """make error info with code
      """
      return ("%s. Error code: %d, detail: %s" %
              ( self.__errmsg, self.__code, self.__details) )

   def __str__(self):
      """return the error info.
      """
      if self.__code == 0:
         return self.__repr__()
      return self.__detail()

   @property
   def code(self):
      """The error code returned by the server, if any.
      """
      return self.__code

   @property
   def detail(self):
      """return the detail error message
      """
      if self.__code == 0:
         return self.__str__()
      return self.__detail()

class InvalidParameter(TypeError):
   """Invalid parameter
   """
