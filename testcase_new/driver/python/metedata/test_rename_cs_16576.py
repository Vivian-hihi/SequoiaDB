# @decription rename collection_space 
# @testlink   seqDB-16576
# @interface  rename_collection_space	( self, old_name, new_name, options = None )		
# @author     yinzhen 2018-12-06

from lib import testlib
from lib import sdbconfig
from pysequoiadb.error import (SDBBaseError, SDBEndOfCursor)


class TestRenameCS16576(testlib.SdbTestBase):
   def setUp(self):
      self.test_cs_name = "test1_16576"
      testlib.drop_cs(self.db, self.test_cs_name, ignore_not_exist=True)
		 
   def test_rename_cs_16576(self):
      #create collection space
      self.cs = self.db.create_collection_space(self.test_cs_name)
      self.assertTrue(self.is_collection_space_exist(self.test_cs_name), "===> step 1 create collection space fail")
      
      # SEQUOIADBMAINSTREAM-3995 filter rename $XX create collection
      self.cs.create_collection("test_cl")
      
      #rename collection space
      try:
         self.db.rename_collection_space(self.test_cs_name, "$test_cs")
      except SDBBaseError as e:
         self.assertEqual(-6, e.code, "===> step 1 check rename collection space fail")
         
      try:
         self.db.rename_collection_space(self.test_cs_name, "test_cs.cs1")
      except SDBBaseError as e:
         self.assertEqual(-6, e.code, "===> step 2 check rename collection space fail")
         
      try:
         self.db.rename_collection_space(self.test_cs_name, "")
      except SDBBaseError as e:
         self.assertEqual(-6, e.code, "===> step 3 check rename collection space fail")
         
      cs_name = ""
      for i in range(200):
         cs_name += "a"
      try:
         self.db.rename_collection_space(self.test_cs_name, cs_name)
      except SDBBaseError as e:
         self.assertEqual(-6, e.code, "===> step 4 check rename collection space fail")
      
      cs_name = ""
      for i in range(127):
         cs_name += "b"      
      self.db.rename_collection_space(self.test_cs_name, cs_name)
      self.assertTrue(self.is_collection_space_exist(cs_name), "===> step 5 rename collection space fail")
      
      cs_name2 = "test_cs_16576@"
      self.db.rename_collection_space(cs_name, cs_name2)
      self.assertTrue(self.is_collection_space_exist(cs_name2), "===> step 6 rename collection space fail")
      
      try:
         self.db.rename_collection_space(cs_name2, "SYStest_cs")
      except SDBBaseError as e:
         self.assertEqual(-6, e.code, "===> step 7 check rename collection space fail")
         
      self.test_cs_name = cs_name2
         
   def tearDown(self):
      self.db.drop_collection_space(self.test_cs_name)
      self.db.disconnect()
      
   def is_collection_space_exist(self, collection_space_name):
      collection_spaces = []
      cursor = self.db.list_collection_spaces()
      while True:
         try:
            collection_space = cursor.next()
            collection_spaces.append(collection_space) 
         except SDBEndOfCursor:
            break
      cursor.close()
      for i in range(len(collection_spaces)):
         cs_name = collection_spaces[i]["Name"]
         if(collection_space_name == cs_name):
            return True
      return False