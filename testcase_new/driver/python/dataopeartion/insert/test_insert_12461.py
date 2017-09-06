# @decription: insert record include oid
# @testlink:   seqDB-12461
# @interface:  bulk_insert(flag,record) ,flag set 0/1/2,record set list/tuple
# @author:     zhaoyu 2017-8-30

import unittest
from dataopeartion.insert.commlib import *
from lib import sdbconfig
from lib import testlib
from pysequoiadb.error import (SDBBaseError)

class TestCS12461(unittest.TestCase):
   def setUp(self):
      testlib.print_setup_msg(self)
      self.db = testlib.default_db()
      self.run_tearDown = False
      
   def testCS12461(self):
      #create cs and cl
      self.cs_name = "cs_12461"
      self.cl_name = "cl_12461"
      try:
         self.db.drop_collection_space(self.cs_name)
      except SDBBaseError as e:
         if(-34 != e.code):
            print(e.detail)
            self.fail("drop_cs_fail")
      self.cs = self.db.create_collection_space( self.cs_name )
      self.cl = self.cs.create_collection( self.cl_name )
      
      #insert list data
      record = [{"a":1}, {"a":2}]
      self.cl.bulk_insert( 0, record )
      
      #query data and check
      check_Result( self.cl, {}, record, False )
      
      #delete data
      self.cl.delete()
      
      #insert tuple data
      record = ({"a":1}, {"a":2})
      self.cl.bulk_insert( 1, record )
      
      #query data and check
      check_Result( self.cl, {}, record, False )
      
      #delete data
      self.cl.delete()
      
      #check flag set 0
      record = [{"a":1,"_id":1}, {"a":2,"_id":1}, {"a":3,"_id":2}]
      try:
         self.cl.bulk_insert( 0, record )
         self.fail("need_an_error")
      except SDBBaseError as e:
         if(-38 != e.code):
            print(e.detail)
            self.fail("check_error_code_fail")
      
      #query data and check
      expect_record = [{"a":1,"_id":1}]
      check_Result( self.cl, {}, expect_record, True )
      
      #delete data
      self.cl.delete()
      
      #check flag set 1
      record = [{"a":1,"_id":1}, {"a":2,"_id":1}, {"a":3,"_id":2}]
      self.cl.bulk_insert( 1, record )
      
      #query data and check
      expect_record = [{"a":1,"_id":1}, {"a":3,"_id":2}]
      check_Result( self.cl, {}, expect_record, True )
      
      #delete data
      self.cl.delete()
      
      #check flag set 2
      try:
         self.cl.bulk_insert( 2, record )
         self.fail("need_an_error")
      except SDBBaseError as e:
         if(-38 != e.code):
            print(e.detail)
            self.fail("check_error_code_fail")
      
      #query data and check
      expect_record = [{"a":1,"_id":1}]
      check_Result( self.cl, {}, expect_record, True )

      self.run_tearDown = True
      
   def tearDown(self):
      if self.run_tearDown and (not sdbconfig.sdb_config.break_on_failure):
         try:
            self.db.drop_collection_space(self.cs_name)
            self.db.disconnect()
         except SDBBaseError as e:
            if(-34 != e.code):
               print(e.detail)
               self.fail("tear_down_fail")
      testlib.print_teardown_msg(self)
            
     
      
   
        