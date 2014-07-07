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

#class TypeError()

class SequoiaDBError(Exception):
   """Base Exception of Python Driver for SequoiaDB
   
   version 1.8
   """

class ConnectError(SequoiaDBError):
   """Raised when failed to connect to database
   """

class OperationError(SequoiaDBError):
   """Raised when failed to do operation(s)
   """

   def __init__(self, errmsg, code):

      self.errmsg = errmsg
      self.__code = code
      self.__details = common.get_info(code)

   def __repr__(self, what):
      """make the error info.
      """
      return self.errmsg + '%d -----> ' % self.__code + what

   def __str__(self):
      """return the error info.
      """
      return self.__repr__(self.__details)

   @property
   def code(self):
      """The error code returned by the server, if any.
      """
      return self.__code

class InvalidParameter(SequoiaDBError):
   """Raised when an invalid parameter is used.
   """

if __name__ == "__main__":
   def test():
      raise OperationError(" Error: ", -2)
   try:
      test()
   except OperationError,data:
      print data
