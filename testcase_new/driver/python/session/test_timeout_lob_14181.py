# @decription: set time out, crud lob
# @testlink:   seqDB-14181
# @author:     liuxiaoxuan 2018-01-25

import unittest
from lib import testlib
from lib import sdbconfig
from commlib import *
from pysequoiadb.error import SDBBaseError
from bson.objectid import ObjectId
from pysequoiadb.lob import LOB_WRITE

class TestSessiontimeout14181(testlib.SdbTestBase):
   def setUp(self):
      # check standalone
      if testlib.is_standalone():
         self.skipTest('run mode is standalone')
      if 2 > testlib.get_data_group_num():
         self.skipTest('less than 2 groups')
      testlib.drop_cs(self.db, self.cs_name, ignore_not_exist = True)

   def test_session_timeout_14181(self):

      # create cs cl
      self.cs = self.db.create_collection_space(self.cs_name)
      self.cl = self.cs.create_collection(self.cl_name)
      
      # create lob
      lob = self.cl.create_lob(ObjectId('5a699c8081d089d50600006d'))
      
      # write lob
      length = 256 * 1024
      lob_data = random_str(length)
      lob.write(lob_data, len(lob_data))
      lob.close()
      
      # get lob set session timeout 60s 
      opt = {'Timeout' : 60000};
      self.db.set_session_attri(options = opt) 
      lob = self.cl.open_lob(ObjectId('5a699c8081d089d50600006d'), LOB_WRITE)
      
      # set session attr timeout 1ms
      opt = {'Timeout' : 1};
      self.db.set_session_attri(options = opt)
   
      # write lob
      try:
         length = 256 * 1024
         lob_data = random_str(length)
         lob.write(lob_data, len(lob_data))
         print('Need Error -13')
      except SDBBaseError as e:
         if -13 != e.code:
            print('check write lob timeout fail')  
      finally:
         if(lob != None):
            lob.close()         
            
      # get lob set session timeout 60s 
      opt = {'Timeout' : 60000};
      self.db.set_session_attri(options = opt) 
      lob = self.cl.open_lob(ObjectId('5a699c8081d089d50600006d'), LOB_WRITE)      
            
      # set session attr timeout 1ms
      opt = {'Timeout' : 1};
      self.db.set_session_attri(options = opt)
            
      # read lob
      try:
         length = 256 * 1024
         lob.read(length)
         print('Need Error -13')
      except SDBBaseError as e:
         if -13 != e.code:
            print('check read lob timeout fail')           
            
      # remove lob
      try:
         self.cl.remove_lob(ObjectId('5a699c8081d089d50600006d'))
         print('Need Error -13')
      except SDBBaseError as e:
         if -13 != e.code:
            print('check remove lob timeout fail')       

   def tearDown(self):
      # set session timeout 60s at last
      opt = {'Timeout' : 60000};
      self.db.set_session_attri(options = opt) 
      try:
         testlib.drop_cs(self.db, self.cs_name)   
         self.db.disconnect()
      except SDBBaseError as e:
         if -34 != e.code:
            print(e.detail)
            self.fail("tear_down_fail")    