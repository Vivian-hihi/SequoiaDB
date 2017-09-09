# @decription: create cs and drop cs
# @testlink:   seqDB-12443
# @interface:  create_collection_space(cs_name,options):options set int
#              list_collection_spaces()
#              drop_collection_space(cs_name)
# @author:     zhaoyu 2017-8-24

import unittest
from pysequoiadb.error import (SDBBaseError, SDBEndOfCursor)
from lib import sdbconfig
from lib import testlib

class TestMeteData12443(unittest.TestCase):
   def setUp(self):
      testlib.print_setup_msg(self)
      self.db = testlib.default_db()
   
   def test_metedata_12443(self):
      self.list = [["cs_12443_1", 0, 65536, 262144],
                   ["cs_12443_2", 4096, 4096, 262144],
                   ["cs_12443_3", 8192, 8192, 262144],
                   ["cs_12443_4", 16384, 16384, 262144],
                   ["cs_12443_5", 32768, 32768, 262144],
                   ["cs_12443_6", 65536, 65536, 262144],
                   #["cs_12443_7", 131072],
                   #["cs_12443_8", 262144],
                   #["cs_12443_9", 524288]
                  ]
      for index in range(len(self.list)):
         self.cs_name = self.list[index][0]
         try:
            self.db.drop_collection_space(self.cs_name)
         except SDBBaseError as e:
            if(-34 != e.code):
               print(e.detail)
               self.fail("drop_cs_fail")
         page_size = self.list[index][1]
         expect_page_size = self.list[index][2]
         expect_lob_page_size = self.list[index][3]
         expect_cs_options = {"PageSize":expect_page_size, "LobPageSize":expect_lob_page_size}
         self.createCSAndCheck(self.cs_name, page_size, expect_cs_options)
      
   def tearDown(self):
      try:
         for index in range(len(self.list)):
            self.db.drop_collection_space(self.cs_name)
         self.db.disconnect()
      except SDBBaseError as e:
         if(-34 != e.code):
            print(e.detail)
            self.fail("tear_down_fail")
      testlib.print_teardown_msg(self)
        
   def createCSAndCheck(self, cs_name, page_size, expect_cs_options):
      #create cs and cl
      cl_name = "cl"
      self.cs = self.db.create_collection_space(cs_name, page_size);
      self.cs.create_collection( cl_name )
      
      #check cs exists or not
      self.check_list_collection_spaces(cs_name, True)
      
      #check cs options
      self.check_cs_snapshot_5(self.cs_name, expect_cs_options)
      
      #drop cs and check exists or not
      self.db.drop_collection_space(cs_name)
      self.check_list_collection_spaces(cs_name, False)
      
   def check_cs_snapshot_5(self, cs_name, options):
      cursor = self.db.get_snapshot( 5, condition = {"Name":cs_name})
      while True:
         try:
            record = cursor.next()
            self.assertDictContainsSubset(options, record)
         except SDBEndOfCursor:
            break
      cursor.close()
            
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
      cursor.close()
      if expect_cs_name in cs_names:
         flag = True
         self.assertEqual(flag, cs_exists)
