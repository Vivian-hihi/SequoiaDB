# @decription create and drop autoincrements
# @testlink   seqDB-16632
# @interface  create_autoincrement	( self, options )  drop_autoincrement( self, names )
# @author     yinzhen 2018-12-12

from lib import testlib
from lib import sdbconfig
from pysequoiadb.error import (SDBBaseError, SDBEndOfCursor)


class TestCreateDropAutoIncrements16632(testlib.SdbTestBase):
   def setUp(self):
      # create cs cl   
      testlib.drop_cs(self.db, self.cs_name, ignore_not_exist=True)      
      self.cs = self.db.create_collection_space(self.cs_name)
      self.cl = self.cs.create_collection(self.cl_name)

   def test_create_drop_autoincrements_16632(self):
      #create autoincrement with no paramter or None
      try:
         self.cl.create_autoincrement({})
      except SDBBaseError as e:
         self.assertEqual(-6, e.code, "===> step 1 check create autoincrement with {} fail")
         
      try:
         self.cl.create_autoincrement(None)
      except SDBBaseError as e:
         self.assertEqual(-6, e.code, "===> step 2 check create autoincrement with None fail")
      
      #create autoincrement with invalid paramter
      try:
         self.cl.create_autoincrement({"FirstField":"a", "Increment":"10"})
      except SDBBaseError as e:
         self.assertEqual(-6, e.code, "===> step 3 check create autoincrement with invalid paramter fail")
      
      #create autoincrement with effective paramter
      self.cl.create_autoincrement([{"Field":"a"},{"Field":"b"},{"Field":"c"}])
      self.cl.create_autoincrement(({"Field":"d"},{"Field":"e"},{"Field":"f"}))
      
      #insert
      self.insert_records() 
      except_records = self.get_expect_records()
      cursor = self.cl.query()
      actual_records = testlib.get_all_records_noid(cursor)
      msg = "===> step 4 " + str(except_records) + "expect is not equal to actResult" + str(actual_records)
      self.assertListEqual(except_records, actual_records, msg)
      
      #drop_autoincrement
      try:
         self.cl.drop_autoincrement("test")
      except SDBBaseError as e:
         self.assertEqual(-333, e.code, "===> step 5 drop_autoincrement not exist fail")
         
      try:
         self.cl.drop_autoincrement("")
      except SDBBaseError as e:
         self.assertEqual(-6, e.code, "===> step 6 drop_autoincrement no str fail")
         
      try:
         self.cl.drop_autoincrement(None)
      except SDBBaseError as e:
         self.assertEqual(-6, e.code, "===> step 7 drop_autoincrement None fail")
         
      #check sequence
      sequence_name = self.get_sequence_name("a")
      if (sequence_name == "ERROR"):
         self.assertFail("===> step 8 get_sequence_name fail")
      
      self.assertTrue(self.check_sequence_name_exist(sequence_name), "===> step 9 check sequence_name exist fail")
      self.cl.drop_autoincrement("a")
      self.assertFalse(self.check_sequence_name_exist(sequence_name), "===> step 10 check sequence_name not exist fail")
      
   def tearDown(self):
      self.db.drop_collection_space(self.cs_name)   
      self.db.disconnect()
      
   def insert_records(self):
      doc = []
      for i in range(0, 10):
         doc.append({"name":"a"+str(i)})
      try:
         flags = 0
         self.cl.bulk_insert(flags, doc)
      except SDBBaseError as e:
         self.fail('insert fail: ' + e.detail) 

   def get_expect_records(self):
      doc = []
      for i in range(0, 10):
         doc.append({"name":"a"+str(i), "a":i+1, "b":i+1, "c":i+1, "d":i+1, "e":i+1, "f":i+1})
      return doc
      
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
      