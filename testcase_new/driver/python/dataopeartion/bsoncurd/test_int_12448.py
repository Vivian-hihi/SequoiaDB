# @decription: insert int data
# @testlink:   seqDB-12448
# @interface:  insert(record)
#              update(rule, kwargs)
#              delete(kwargs)
# @author:     zhaoyu 2017-8-31

import unittest
import datetime
from pysequoiadb import client
from pysequoiadb import collectionspace
from pysequoiadb.error import (SDBTypeError, SDBBaseError, SDBEndOfCursor)
from dataopeartion.bsoncurd.commlib import *
from lib.config import *

class TestCS12448(unittest.TestCase):
   def setUp(self):
      print(datetime.datetime.now())
      config = Config()
      self.db = client( config.host_name, config.service )
      
   def testCS12448(self):
      #create cs and cl
      self.cs_name = "cs_12448"
      self.cl_name = "cl_12448"
      self.cs = self.db.create_collection_space( self.cs_name )
      self.cl = self.cs.create_collection( self.cl_name )
      
      #insert int data
      data1 = -2147483648
      data2 = 2147483647
      record = [{"a":data1}, {"a":data2}]
      self.cl.bulk_insert( 0, record )
      
      #query data and check
      expect_type = [{"a":"int32"},{"a":"int32"}]
      check_Result( self.cl, {}, {"a":{"$type":2}}, record, expect_type, False )
      
      #update data
      data1_after_update = 1
      data2_after_update = 0
      self.cl.update({"$set":{"a":data1_after_update}},condition = {"a":data1})
      self.cl.update({"$set":{"a":data2_after_update}},condition = {"a":data2})
      
      #query data and check
      record_after_update = [{"a":data1_after_update},{"a":data2_after_update}]
      check_Result( self.cl, {}, {"a":{"$type":2}}, record_after_update, expect_type, False )
      
      #update data
      self.cl.update({"$set":{"a":data1}},condition = {"a":data1_after_update})
      self.cl.update({"$set":{"a":data2}},condition = {"a":data2_after_update})
      
      #query data and check
      check_Result( self.cl, {}, {"a":{"$type":2}}, record, expect_type, False )
      
      #delete data
      self.cl.delete()
      
      #query data and check
      check_Result( self.cl, {}, {}, {}, {}, False )
      
      #insert int data 
      self.cl.bulk_insert( 0, record )
      
      #query data and check
      check_Result( self.cl, {}, {"a":{"$type":2}}, record, expect_type, False )
      
      #delete data
      self.cl.delete(condition = {"a":data1})
      self.cl.delete(condition = {"a":{"$et":data2}})
      
      #query data and check
      check_Result( self.cl, {}, {}, {}, {}, False )
      
      #insert out of range int
      data3 = -2147483649
      data4 = 2147483648
      record = [{"a":data3},{"a":data4}]
      self.cl.bulk_insert( 0, record )
      
      #query data and check
      expect_type = [{"a":"int64"},{"a":"int64"}]
      check_Result( self.cl, {}, {"a":{"$type":2}}, record, expect_type, False )
      
   def tearDown(self):
      try:
         print(datetime.datetime.now())
         self.db.drop_collection_space(self.cs_name)
         self.db.disconnect()
      except SDBBaseError as e:
         if(-34 != e.code):
            print(e.detail)
            self.fail("tear_down_fail")