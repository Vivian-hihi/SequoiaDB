# @decription create collection with autoincrement then alter autoincrement
# @testlink   seqDB-16655
# @interface  create_collection ( self, cl_name, options = None ),alter ( self, options ),set_attributes ( self, options )
# @author     yinzhen 2018-12-12

from lib import testlib
from lib import sdbconfig
from pysequoiadb.error import (SDBBaseError, SDBEndOfCursor)


class TestAlterAutoIncrement16655(testlib.SdbTestBase):
   def setUp(self):
      #skip standlone mode
      if testlib.is_standalone():
         self.skipTest("skip! This testcase do not support standlone")
   
      # create cs cl   
      testlib.drop_cs(self.db, self.cs_name, ignore_not_exist=True)      
      self.cs = self.db.create_collection_space(self.cs_name)

   def test_sequence_snapshot_list_16655(self):
      #create autoincrement
      self.cl = self.cs.create_collection(self.cl_name, {"AutoIncrement":{"Field":"test16655"}})
         
      #check snapshot
      sequence_name = self.get_sequence_name("test16655")
      if (sequence_name == "ERROR"):
         self.assertFail("===> step 1 get_sequence_name fail, sequence_name : " + sequence_name)
      self.assertTrue(self.check_sequence_name_exist(sequence_name), "===> step 2 check sequence_name exist fail sequence_name : " + sequence_name)
      
      #alter autoincrement
      self.cl.alter({"AutoIncrement":{"Field":"test16655", "Increment":2, "StartValue":12, "MinValue":10, "MaxValue":500, "CacheSize":100, "AcquireSize":50}})
      exp_result = {"Name":sequence_name, "Increment":2, "StartValue":12, "MinValue":10, "MaxValue":500, "CacheSize":100, "AcquireSize":50}
      cursor = self.db.get_snapshot(15, condition = {"Name":sequence_name})
      act_result = cursor.next()
      self.assertTrue(self.check_result(exp_result, act_result), "===> step 3 check_result fail exp_result : " + str(exp_result) + " act_result : " + str(act_result))
      
      #set_attributes autoincrement
      self.cl.set_attributes({"AutoIncrement":{"Field":"test16655", "Increment":3, "StartValue":100, "MinValue":1, "MaxValue":50000, "CacheSize":1000, "AcquireSize":800}})
      exp_result = {"Name":sequence_name, "Increment":3, "StartValue":100, "MinValue":1, "MaxValue":50000, "CacheSize":1000, "AcquireSize":800}
      cursor = self.db.get_snapshot(15, condition = {"Name":sequence_name})
      act_result = cursor.next()
      self.assertTrue(self.check_result(exp_result, act_result), "===> step 4 check_result fail exp_result : " + str(exp_result) + " act_result : " + str(act_result))
      
   def tearDown(self):
      self.db.drop_collection_space(self.cs_name)   
      self.db.disconnect()
      
   def get_sequence_name(self, field):
      cursor = self.db.get_snapshot(8, condition = {"Name":self.cs_name + "." + self.cl_name})
      snapshots = testlib.get_all_records(cursor)
      obj = snapshots[0]
      AutoIncrement = obj["AutoIncrement"]
      for item in AutoIncrement:
         if (field == item["Field"]):
            return item["SequenceName"]
      return "ERROR"
      
   def check_sequence_name_exist(self, sequence_name):
      print("sequence_name : " + sequence_name)
      cursor = self.db.get_snapshot(15, condition = {"Name":sequence_name})
      try:
         cursor.next()
         return True 
      except SDBEndOfCursor:
         return False
         
   def check_result(self, exp_result, act_result):
      for item in exp_result:
         if exp_result[item] != act_result[item]:
            return False
      return True