import pysequoiadb
from pysequoiadb import client
from pysequoiadb import collectionspace
from pysequoiadb import collection
from pysequoiadb import cursor
from pysequoiadb import const
from pysequoiadb.error import (SDBTypeError,
                               SDBBaseError,
                               SDBEndOfCursor)
if __name__ == "__main__":

   try:
      # connect to local db, using default args value.
      # host= '192.168.20.43', port= 11810, user= '', password= ''
      db = client("192.168.20.43", 11810)
      cs_name = "pydriver_collection_truncate"
      cs = db.create_collection_space(cs_name)

      cl_name = "truncate_python_driver"
      cl = cs.create_collection(cl_name, {"ReplSize":0})

      # insert single record
      for i in range(0, 10):
         basketball = {"Item":"basketball", "id":i}
         cl.insert(basketball)

      print 'success to insert 10 number'

      # truncate
      print 'truncate begin'
      cl.truncate();

      # verify

      # release all
      cs.drop_collection(cl_name)
      del cl

      db.drop_collection_space(cs_name)
      del cs

      db.disconnect()
      del db

   except (SDBTypeError, SDBBaseError), e:
      pysequoiadb._print(e)
   except SDBBaseError, e:
      pysequoiadb._print(e.detail)
