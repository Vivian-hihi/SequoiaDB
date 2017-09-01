# @decription: create cs and drop cs;
# @testlink:   seqDB-12442
# @interface:  create_collection_space(cs_name)
#              list_collection_spaces()
#              drop_collection_space(cs_name)
# @author:     zhaoyu 2017-8-24

import unittest
import datetime
from pysequoiadb import client
from pysequoiadb import collectionspace
from pysequoiadb.error import (SDBTypeError, SDBBaseError, SDBEndOfCursor)
from lib.config import *

class TestCS12442(unittest.TestCase):
   def setUp(self):
      print(datetime.datetime.now())
      config = Config()
      self.db = client( config.host_name, config.service )
      
   def testCS12442(self):
      #create cs and cl
      self.cs_name = "cs_12442"
      cl_name = "cl_12442"
      try:
         self.db.drop_collection_space(self.cs_name)
      except SDBBaseError as e:
         if(-34 != e.code):
            print(e.detail)
            self.fail("drop_cs_fail")
      self.cs = self.db.create_collection_space( self.cs_name )
      self.cs.create_collection( cl_name )
      
      #check cs exists or not
      self.check_list_collection_spaces(self.cs_name, True)
      
      #check cs options
      cs_options = {"PageSize":65536, "LobPageSize":262144}
      self.check_cs_snapshot_5(self.cs_name, cs_options)
      
      #drop cs and check exists or not
      self.db.drop_collection_space(self.cs_name)
      self.check_list_collection_spaces(self.cs_name, False)
      
         
   def tearDown(self):
      try:
         print(datetime.datetime.now())
         self.db.drop_collection_space(self.cs_name)
         self.db.disconnect()
      except SDBBaseError as e:
         if(-34 != e.code):
            print(e.detail)
            self.fail("tear_down_fail")

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
   
      