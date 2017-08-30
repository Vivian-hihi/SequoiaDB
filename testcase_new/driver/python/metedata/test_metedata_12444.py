# @decription: create cs and drop cs
# @testlink:   seqDB-12444
# @interface:  create_collection_space(cs_name,options):options set dict,key set PageSize/LobPageSize
#              list_collection_spaces()
#              drop_collection_space(cs_name)
# @author:     zhaoyu 2017-8-24

import unittest
import datetime
from pysequoiadb import client
from pysequoiadb import collectionspace
from pysequoiadb.error import (SDBTypeError, SDBBaseError, SDBEndOfCursor)
from lib.config import *

class TestCS12444(unittest.TestCase):
   def setUp(self):
      print(datetime.datetime.now())
      config = Config()
      self.db = client( config.host_name, config.service )
      
   def testCS12444(self):
      self.list = [["cs_12444_1", 0, 524288, 65536, 524288],
                   ["cs_12444_2", 4096, 262144, 4096, 262144],
                   ["cs_12444_3", 8192, 131072, 8192, 131072],
                   ["cs_12444_4", 16384, 32768, 16384, 32768],
                   ["cs_12444_5", 32768, 65536, 32768, 65536],
                   ["cs_12444_6", 65536, 16384, 65536, 16384],
                   ["cs_12444_7", 0, 8192, 65536, 8192],
                   ["cs_12444_8", 0, 4096, 65536, 4096],
                   ["cs_12444_9", 0, 0, 65536, 262144]
                   ]
                  
      for index in range(len(self.list)):
         self.cs_name = self.list[index][0]
         page_size = self.list[index][1]
         lob_page_size = self.list[index][2]
         cs_options = {"PageSize":page_size, "LobPageSize":lob_page_size}
         expect_page_size = self.list[index][3]
         expect_lob_page_size = self.list[index][4]
         expect_cs_options = {"PageSize":expect_page_size, "LobPageSize":expect_lob_page_size}
         self.createCSAndCheck(self.cs_name, cs_options, expect_cs_options) 
   
   def tearDown(self):
      try:
         print(datetime.datetime.now())
         for index in range(len(self.list)):
            self.db.drop_collection_space(self.cs_name)
         self.db.disconnect()
      except SDBBaseError as e:
         if(-34 != e.code):
            print(e.detail)
            self.fail("tear_down_fail") 
           
   def createCSAndCheck(self, cs_name, cs_options, expect_cs_options):
      #create cs and cl
      cl_name = "cl"
      self.cs = self.db.create_collection_space(cs_name, cs_options);
      self.cs.create_collection( cl_name )
      
      #check cs exists or not
      self.check_list_collection_spaces(cs_name, True)
      
      #check cs options
      self.check_cs_snapshot_5(self.cs_name, expect_cs_options)
      
      #drop cs and check exists or not      
      self.db.drop_collection_space(self.cs_name)
      self.check_list_collection_spaces(cs_name, False)
      
   def check_cs_snapshot_5(self, cs_name, options):
      cursor = self.db.get_snapshot( 5, condition = {"Name":cs_name})
      while True:
         try:
            record = cursor.next()
            actual_cs_name = record['Name']
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
            
   def check_list_collection_spaces(self, expect_cs_name, cs_exists):
      cursor = self.db.list_collection_spaces()
      flag = False
      cs_names = []
      while True:
         try:
            record = cursor.next()
            cs_names.append(record['Name'])
         except SDBEndOfCursor:
            break
      if expect_cs_name in cs_names:
         flag = True
         self.assertEqual(flag, cs_exists)
