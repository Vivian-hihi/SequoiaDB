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
      
      # create a cs
      cs_name = "gymnasium"
      cs = db.create_collection_space(cs_name)

      #create a cl
      cl_name = "sports"
      cl = cs.create_collection(cl_name)

      #create an index
      index = {'Item':1, 'Rank':-1}
      index_name = 'idx'
      cl.create_index(index, index_name, False, False)

      # get all indexes
      cr = cl.get_indexes()

      # print indexes
      state, record = cr.next()
      while ( const.SDB_DMS_EOC != state ):
         pysequoiadb._print(record)
         state, record = cr.next()

      # release all
      cs.drop_collection(cl_name)
      del cl

      db.drop_collection_space(cs_name)
      del cs

      db.disconnect()
      del db

   except (InvalidParameter, SequoiaDBError), e:
      pysequoiadb.cout(e)