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
