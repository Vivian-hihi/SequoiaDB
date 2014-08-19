#! /usr/bin/python

import pysequoiadb
from pysequoiadb import client
from pysequoiadb import const
from pysequoiadb.error import (InvalidParameter, SequoiaDBError)

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

      pysequoiadb._print("before update")
      cr = cl.query()
      rc, record = cr.next()
      while const.SDB_DMS_EOC != rc:
         pysequoiadb._print(record)
         rc, record = cr.next()

      # update records
      update = {'$set':{"Item":"football", "Rank":1 }}
      condition = {'id':{'$exists':0}}
      cl.update(update, condition)

      pysequoiadb._print("after update")
      cr = cl.query()
      rc, record = cr.next()
      while const.SDB_DMS_EOC != rc:
         pysequoiadb._print(record)
         rc, record = cr.next()

      # drop collection
      cs.drop_collection( cl_name )
      del cl

      # drop collection space
      db.drop_collection_space(cs_name)
      del cs

      db.disconnect()
      del db

   except (InvalidParameter, SequoiaDBError), e:
      pysequoiadb._print(e)