# @decription rename collection_space not exist
# @testlink   seqDB-16574
# @interface  rename_collection_space	( self, old_name, new_name, options = None )		
# @author     yinzhen 2018-12-06

from lib import testlib
from lib import sdbconfig
from pysequoiadb.error import (SDBBaseError, SDBEndOfCursor)


class TestRenameNotExistCS16574(testlib.SdbTestBase):
   def setUp(self):
      self.test_cs_name = "test1_16574"
      testlib.drop_cs(self.db, self.test_cs_name, ignore_not_exist=True)      
		 
   def test_rename_cs_16574(self):
      #create collection space
      self.db.create_collection_space(self.test_cs_name)
      self.assertTrue(self.is_collection_space_exist(self.test_cs_name), "===> step 1 create collection space fail")
      
      #drop collection space
      self.db.drop_collection_space(self.test_cs_name)
      self.assertFalse(self.is_collection_space_exist(self.test_cs_name), "===> step 2 drop collection space fail")
      
      #rename not exist collection space
      try:
         self.db.rename_collection_space(self.test_cs_name, "test2_16574")
      except SDBBaseError as e:
         self.assertEqual(-34, e.code, "===> step 3 check rename not exist collection space fail")
         
      self.assertFalse(self.is_collection_space_exist("test2_16574"), "===> step 4 check collection space not exist fail")

   def tearDown(self):
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