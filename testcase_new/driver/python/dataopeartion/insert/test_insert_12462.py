# @decription: insert record include oid
# @testlink:   seqDB-12462
# @interface:  insert(record) set oid
# @author:     zhaoyu 2017-8-30

import unittest
import datetime
from pysequoiadb import client
from pysequoiadb import collectionspace
from pysequoiadb.error import (SDBTypeError, SDBBaseError, SDBEndOfCursor)
from insert.commlib import *
from lib.config import *

class TestCS12462(unittest.TestCase):
   def setUp(self):
      print(datetime.datetime.now())
      config = Config()
      self.db = client( config.host_name, config.service )
      
      self.cs_name = "cs_12462"
      self.cl_name = "cl_12462"
      self.cs = self.db.create_collection_space( self.cs_name )
      self.cl = self.cs.create_collection( self.cl_name )
      
   def testCS12462(self):
      #insert data include oid
      record = {"a":1,"_id":1}
      self.cl.insert( record )
      
      #query data and check
      check_Result( self.cl, {"_id":1}, [record], True )
      
      #insert error
      try:
         self.cl.insert( record )
         self.fail("need_an_error")
      except SDBBaseError as e:
         if(-38 != e.code):
            print(e.detail)
            self.fail("check_error_code_fail")
        
   def tearDown(self):
      try:
         print(datetime.datetime.now())
         self.db.drop_collection_space(self.cs_name)
         self.db.disconnect()
      except SDBBaseError as e:
         if(-34 != e.code):
            print(e.detail)
            self.fail("tear_down_fail")
                
      
   
        