
from pysequoiadb.common import const
class PySequoiaDBError(Exception):
   """Base Exception of Python Driver for SequoiaDB
   
   version 1.8
   """

class ConnectError(PySequoiaDBError):
   """Raised when failed to connect to database
   """

class OperationError(PySequoiaDBError):
   """Raised when fail to do operation(s)
   """
   def get_info(self, code):
      return const.errmaps.get(code,"Unknown")

   def __init__(self, errmsg, code):
      self.errmsg = errmsg
      self.__code = code
      self.__details = self.get_info(code)

   def __str__(self):
      return repr(self.errmsg + self.details)

   @property
   def code(self):
      """The error code returned by the server, if any.
      """
      return self.__code

   @property
   def details(self):
      return self.__details


class InvalidParameter(PySequoiaDBError):
   """Raised when an invalid name is used.
   """

def err_process(result):
   if const.RET_TUPLE_SIZE != len(result):
      raise PySequoiaDBError("internal error")
   if const.SDB_OK != result[const.ERRCODE_INDEX]:
      raise OperationError("sdb error msg", result[const.ERRCODE_INDEX])
   return result[const.RETOBJ_INDEX]

if __name__ == "__main__":
   def test():
      raise OperationError("sdb error msg: ", -2)
   try:
      test()
   except OperationError,data:
      print data