# @decription rename collection not exist
# @testlink   seqDB-16578
# @interface  rename_collection	( self, old_name, new_name, options = None )		
# @author     yinzhen 2018-12-06

from lib import testlib
from lib import sdbconfig
from pysequoiadb.error import (SDBBaseError, SDBEndOfCursor)


class TestRenameNotExistCL16578(testlib.SdbTestBase):
   def setUp(self):
      # create cs 
      testlib.drop_cs(self.db, self.cs_name, ignore_not_exist=True)      
      self.cs = self.db.create_collection_space(self.cs_name)
      self.test_cl_name = "test_cl_16578"

   def test_rename_cs_16578(self):
      #create collection
      self.cs.create_collection(self.test_cl_name)
      self.assertTrue(self.is_collection_exist(self.test_cl_name), "===> step 1 create collection fail")
      
      #drop collection
      self.cs.drop_collection(self.test_cl_name)
      self.assertFalse(self.is_collection_exist(self.test_cl_name), "===> step 2 drop collection fail")
      
      #rename not exist collection
      try:
         self.cs.rename_collection_space(self.test_cl_name, "test2_16578")
      except SDBBaseError as e:
         self.assertEqual(-23, e.code, "===> step 3 check rename not exist collection fail")
         
      self.assertFalse(self.is_collection_exist("test2_16578"), "===> step 4 check collection not exist fail")
      
   def tearDown(self):
      self.db.drop_collection_space(self.cs_name)   
      self.db.disconnect()
      
   def is_collection_exist(self, collection_name):
      collections = []
      cursor = self.db.list_collections()
      while True:
         try:
            collection = cursor.next()
            collections.append(collection) 
         except SDBEndOfCursor:
            break
      cursor.close()
      for i in range(len(collections)):
         cl_name = collections[i]["Name"]
         if(self.cs_name + "." + collection_name == cl_name):
            return True
      return False
         
         