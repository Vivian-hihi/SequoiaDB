# @decription test sequence get_snapshot and get_list interface
# @testlink   seqDB-16633
# @interface  get_snapshot ( self, snap_type, kwargs ) get_list ( self, list_type, kwargs )
# @author     yinzhen 2018-12-12

from lib import testlib
from lib import sdbconfig
from pysequoiadb.error import (SDBBaseError, SDBEndOfCursor)


class TestSequenceSnapshotList16633(testlib.SdbTestBase):
   def setUp(self):
      # create cs cl   
      testlib.drop_cs(self.db, self.cs_name, ignore_not_exist=True)      
      self.cs = self.db.create_collection_space(self.cs_name)
      self.cl = self.cs.create_collection(self.cl_name)

   def test_sequence_snapshot_list_16633(self):
      #create autoincrement
      self.cl.create_autoincrement({"Field":"test16633", "Increment":2, "StartValue":12, "MinValue":10, "MaxValue":500, "CacheSize":100, "AcquireSize":50, "Cycled" : True, "Generated":"strict"})
         
      #check snapshot
      sequence_name = self.get_sequence_name("test16633")
      if (sequence_name == "ERROR"):
         self.assertFail("===> step 2 get_sequence_name fail, sequence_name : " + sequence_name)
      self.assertTrue(self.check_sequence_name_exist(sequence_name), "===> step 3 check sequence_name exist fail")
      exp_result = {"Name":sequence_name, "Increment":2, "StartValue":12, "MinValue":10, "MaxValue":500, "CacheSize":100, "AcquireSize":50, "Cycled" : True}
      cursor = self.db.get_snapshot(15, condition = {"Name":sequence_name})
      act_result = cursor.next()
      self.assertTrue(self.check_result(exp_result, act_result), "===> step 4 check_result fail exp_result : " + str(exp_result) + " act_result : " + str(act_result))
      
      #check list
      cursor = self.db.get_list(15)
      list_sequence = testlib.get_all_records(cursor)
      self.assertTrue(self.check_sequence_name_in_list(list_sequence, sequence_name),"===> step 5 check_sequence_name_in_list fail list_sequence : " + str(list_sequence) + " sequence_name : " + sequence_name )
            
      
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
      
   def check_sequence_name_in_list(self, list_sequence, sequence_name):
      for item in list_sequence:
         if (item["Name"] != sequence_name):
            continue
         else:
            return True
      return False