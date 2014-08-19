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

      # insert records
      records = []
      for idx in xrange(0, 10):
         name = 'SequoiaDB' + str(idx)
         sport = {"Rank":idx, "Name":name}
         records.append(sport)

      cl.bulk_insert(1, [{'idx':i} for i in xrange(10)]) #records

      full_name = cl.get_full_name()
      sql1 = "select * from %s" % full_name
      sql2 = "insert into %s ( Rank, Name ) values( 10000, 'SequoiaDB' )"\
             % full_name

      # execute sql1
      cr = db.exec_sql(sql1)
      pysequoiadb._print("The result are below after execute sql:%s" % sql1)
      rc, record = cr.next()
      while const.SDB_DMS_EOC != rc:
         pysequoiadb._print(record)
         rc, record = cr.next()

      pysequoiadb._print('\n')

      # execute sql2
      db.exec_update(sql2)
      pysequoiadb._print("The result are below after execute sql:%s" % sql2)
      rcr = cl.query()
  
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
      