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
      # host= '192.168.20.48', port= 11810, user= '', password= ''
      db = client("192.168.20.48", 50000)
      cs_name = "gymnasium"
      cs = db.create_collection_space(cs_name)

      cl_name = "sports"
      cl = cs.create_collection(cl_name, {"ReplSize":0})

      # insert single record
      for i in range(0, 10):
         basketball = {"Item":"basketball", "id":i}
         cl.insert(basketball)

      cr = cl.query_and_update({"$set":{"id":1000}}, {"id": 1}, return_new=True)
      while True:
         try:
            record = cr.next()
            pysequoiadb._print(record)
            if (1000 == record['id']):
               print 'find and update ok'
         except SDBEndOfCursor :
            break
         except SDBBaseError :
            raise

      cr = cl.query_and_remove(condition={"id":1000})
      while True:
         try:
            record = cr.next()
            pysequoiadb._print(record)
            if (1000 == record['id']):
               print 'find and remove failed'

         except SDBEndOfCursor :
            break
         except SDBBaseError :
            raise

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
