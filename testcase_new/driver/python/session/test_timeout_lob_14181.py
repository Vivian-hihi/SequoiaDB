# @decription: set time out, crud lob
# @testlink:   seqDB-14181
# @author:     liuxiaoxuan 2018-01-25

import unittest
from lib import testlib
from lib import sdbconfig
from session.commlib import *
from pysequoiadb.error import SDBBaseError
from bson.objectid import ObjectId
from pysequoiadb.lob import LOB_WRITE
from pysequoiadb.lob import LOB_READ

class TestSessiontimeout14181(testlib.SdbTestBase):
   def setUp(self):
      # skip standalone
      if testlib.is_standalone():
         self.skipTest('run mode is standalone')
      testlib.drop_cs(self.db, self.cs_name, ignore_not_exist = True)

   def test_session_timeout_14181(self):

      # create cs cl
      self.cs = self.db.create_collection_space(self.cs_name)
      self.cl = self.cs.create_collection(self.cl_name, {'ReplSize' : 0})
      
      # create lob
      lob = self.cl.create_lob(ObjectId('5a699c8081d089d50600006d'))
      
      # write lob
      length = 20 * 1024 * 1024
      lob_data = random_str(length)
      lob.write(lob_data, len(lob_data))
      lob.close()    
      
      # set session attr timeout 1ms
      newdb1 = testlib.default_db()
      newcl1 = newdb1.get_collection(self.cs_name + '.' + self.cl_name)
      
      opt = {'Timeout' : 1};
      newdb1.set_session_attri(options = opt)
   
      # write lob
      try:
         lob = newcl1.open_lob(ObjectId('5a699c8081d089d50600006d'), LOB_WRITE)
         length = 20 * 1024 * 1024
         lob_data = random_str(length)
         lob.write(lob_data, len(lob_data))
         self.fail('Need Error -13')
      except SDBBaseError as e:
         if -13 != e.code:
            self.fail('check write lob timeout fail: ' + e.detail)  
      finally:
         if(lob != None):
            lob.close()         
            
      # read lob
      try:
         lob = newcl1.open_lob(ObjectId('5a699c8081d089d50600006d'), LOB_READ)
         length = 20 * 1024 * 1024
         lob.read(length)
         self.fail('Need Error -13')
      except SDBBaseError as e:
         if -13 != e.code:
            self.fail('check read lob timeout fail: ' + str(e.detail)) 
      finally:
         if(lob != None):
            lob.close() 				
            
      # remove lob
      try:
         newcl1.remove_lob(ObjectId('5a699c8081d089d50600006d'))
         self.fail('Need Error -13')
      except SDBBaseError as e:
         if -13 != e.code:
            self.fail('check read lob timeout fail: ' + str(e.detail))   
      finally:
         if(lob != None):
            lob.close() 		

      newdb1.disconnect()            

   def tearDown(self): 
      try:
         testlib.drop_cs(self.db, self.cs_name)   
         self.db.disconnect()
      except SDBBaseError as e:
         if -34 != e.code:
            print(e.detail)
            self.fail("tear_down_fail")    