#! /usr/bin/python

import pysequoiadb
from pysequoiadb import client
from pysequoiadb import const
from pysequoiadb.error import SequoiaDBError

from bson.objectid import ObjectId

if __name__ == "__main__":

   try:
      # connect to local db, using default args value.
      # host= 'localhost', port= 11810, user= '', password= ''
      db = client()
   
      cs_name = "gymnasium"
      cs = db.create_collection_space(cs_name)

      cl_name = "sports"
      cl = cs.create_collection(cl_name)

      # insert single record
      basketball = {"Item":"basketball", "id":0}
      oid = cl.insert(basketball)

      # insert records
      records = []
      for idx in range(2, 10):
         sport = {"sport id":idx}
         records.append(sport)
      cl.bulk_insert(1, records)

      # drop collection
      cs.drop_collection( cl_name )
      del cl
      # drop collection space
      db.drop_collection_space(cs_name)
      del cs

      db.disconnect()
      del db

   except (InvalidParameter, SequoiaDBError), e:
      pysequoiadb.cout(e)