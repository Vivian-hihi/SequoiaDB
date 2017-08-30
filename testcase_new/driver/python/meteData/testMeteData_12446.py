# @decription: create cl set compress and check options
# @testlink:   seqDB-12446
# @interface:  create_collection(cl_name)
#              create_collection(cl_name,options):options set dict,
#                 keys set AutoIndexId/EnsureShardingIndex/Compressed/CompressionType:"snappy"/"lzw"
#              drop_collection(cl_name)
#              list_collections()
# @author:     zhaoyu 2017-8-29

import unittest
import datetime
from pysequoiadb import client
from pysequoiadb import collectionspace
from pysequoiadb.error import (SDBTypeError, SDBBaseError, SDBEndOfCursor)
from meteData.commlib import *
from lib.config import *

class TestCS12446(unittest.TestCase):
   def setUp(self):
      print(datetime.datetime.now())
      config = Config()
      self.db = client( config.host_name, config.service )
      
   def testCS12446(self):
      #create cs
      self.cs_name = "cs_12446"
      cl_names = ["cl_12446_1", "cl_12446_2"]
      self.cs = self.db.create_collection_space( self.cs_name )
      
      #create cl set Compressed
      cl_options_1 = {"ShardingKey":{"a":1},"ShardingType":"range",
                      "AutoIndexId":False,"EnsureShardingIndex":False,
                      "Compressed":True,"CompressionType":"lzw"}
      self.cs.create_collection( cl_names[0], cl_options_1)
      
      cl_options_2 = {"ShardingKey":{"a":1},"ShardingType":"range",
                      "AutoIndexId":False,"EnsureShardingIndex":False,
                      "Compressed":True}
      self.cs.create_collection( cl_names[1], cl_options_2)
      
      #check cl
      except_cl_options_1 = {"Attribute":3, "AttributeDesc":"Compressed | NoIDIndex",
                             "CompressionType":1, "CompressionTypeDesc":"lzw",
                             "ShardingKey":{"a":1}, "EnsureShardingIndex":False,
                             "ShardingType":"range", "AutoIndexId":False}
      self.check_cl_snapshot_8(self.cs_name + "." + cl_names[0], except_cl_options_1)
      
      except_cl_options_2 = {"Attribute":3, "AttributeDesc":"Compressed | NoIDIndex",
                             "CompressionType":0, "CompressionTypeDesc":"snappy",
                             "ShardingKey":{"a":1}, "EnsureShardingIndex":False,
                             "ShardingType":"range", "AutoIndexId":False}
      self.check_cl_snapshot_8(self.cs_name + "." + cl_names[1], except_cl_options_2)
      
   def tearDown(self):
      try:
         print(datetime.datetime.now())
         self.db.drop_collection_space(self.cs_name)
         self.db.disconnect()
      except SDBBaseError as e:
         if(-34 != e.code):
            print(e.detail)
            self.fail("tear_down_fail")
            
   def check_cl_snapshot_8(self, cl_full_name, options):
      cursor = self.db.get_snapshot( 8, condition = {"Name":cl_full_name} )
      while True:
         try:
            record = cursor.next()
            actual_keys = record.keys()
            expect_keys = options.keys()
            for expect_key in expect_keys:
               has_key = False
               for atual_key in actual_keys:
                  if(expect_key == atual_key):
                     self.assertEqual(record[atual_key], options[expect_key])
                     has_key = True
               self.assertTrue(has_key)
         except SDBEndOfCursor:
            break