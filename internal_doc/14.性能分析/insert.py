#! /usr/bin/python

import pysequoiadb
from pysequoiadb import client
from pysequoiadb import const
from pysequoiadb.error import (SDBTypeError,
                               SDBBaseError,
                               SDBEndOfCursor)
import time
import random

from bson.objectid import ObjectId

if __name__ == "__main__":

   try:
      db = client("localhost", 11810)

      cs_name = "foo"
      cs = db.get_collection_space(cs_name)

      cl_name = "bar"
      cl = cs.get_collection(cl_name)

      inner_obj = {"x":"abcdefghijklmnopqrstuvwxyz#1234567890", "y":"1234567890", "z":"abcdefghijklmnopqrstuvwxyz#abcdefghijklmnopqrstuvwxyz"}

      # insert records
      print("start...")
      batches = 10 
      record_per_batch = 100000
      start_time = time.time()
      for batch in range(0, batches):
         records = []
         for idx in range(0, record_per_batch):
            obj = {"i":batch * record_per_batch + idx, "obj":inner_obj, "int":random.randint(0, idx)}
            records.append(obj)
         begin_time = time.time();
         cl.bulk_insert(1, records)
         end_time = time.time();
         print("batch " + str(batch) + ": " + str(end_time - begin_time) + " seconds.")
      finish_time = time.time()
      print("total: " + str(finish_time - start_time) + " seconds.")

      # drop collection
      del cl
      # drop collection space
      del cs

      db.disconnect()
      del db

      print("Success")

   except (SDBTypeError, SDBBaseError), e:
      pysequoiadb._print(e)
