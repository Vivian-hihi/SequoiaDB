
import pysequoiadb
from pysequoiadb import common
from pysequoiadb.common import const
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

   def __str__(self):
      return repr( self.errmsg + str(self.__code) + '----->' + self.details() )

   @property
   def code(self):
      """The error code returned by the server, if any.
      """
      return self.__code

   @property
   def details(self):
      return self.__details


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
