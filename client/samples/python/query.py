#! /usr/bin/python

import pysequoiadb
from pysequoiadb import client
from pysequoiadb import const
from pysequoiadb.error import (SDBTypeError,
                               SDBBaseError)

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

      # insert single record for query()
      basketball = {"Item":"basketball", "id":0}
      oid = cl.insert(basketball)

      cond = {"id":{'$et':0}}
      pysequoiadb._print("query one record, using condition=%s" % cond)

      cr = cl.query(condition=cond)
      rc, record = cr.next()
      while const.SDB_DMS_EOC != rc:
         pysequoiadb._print(record)
         rc, record = cr.next()

      # bulk_insert
      records = []
      for idx in range(2, 10):
         sport = {"sport id":idx}
         records.append(sport)
      cl.bulk_insert(1, records)

      pysequoiadb._print("query all records:")
      cr = cl.query()
      rc, record = cr.next()
      while const.SDB_DMS_EOC != rc:
         pysequoiadb._print(record)
         rc, record = cr.next()

      cs.drop_collection( cl_name )
      del cl

      db.drop_collection_space(cs_name)
      del cs

      db.disconnect()
      del db

   except (SDBTypeError, SDBBaseError), e:
      pysequoiadb._print(e)
