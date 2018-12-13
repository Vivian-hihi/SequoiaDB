# @decription rename collection_space and metadata operation
# @testlink   seqDB-16573
# @interface  rename_collection_space	( self, old_name, new_name, options = None )		
# @author     yinzhen 2018-12-06

from lib import testlib
from lib import sdbconfig
from pysequoiadb.error import (SDBBaseError, SDBEndOfCursor)


class TestRenameCS16573(testlib.SdbTestBase):
   def setUp(self):
      # create cs 
      self.test_cs_name = "test1_16573"
      self.test_cs_newname = "test2_16573"
      testlib.drop_cs(self.db, self.test_cs_name, ignore_not_exist=True)
      testlib.drop_cs(self.db, self.test_cs_newname, ignore_not_exist=True)      
      
		 
   def test_rename_cs_16573(self):
      #rename collection space
      self.cs = self.db.create_collection_space(self.test_cs_name)
      self.assertTrue(self.is_collection_space_exist(self.test_cs_name), "===> step 1 create collection space fail")
      
      self.db.rename_collection_space(self.test_cs_name, self.test_cs_newname)
      self.assertTrue(self.is_collection_space_exist(self.test_cs_newname), "===> step 2 rename collection space fail")
      self.assertFalse(self.is_collection_space_exist(self.test_cs_name), "===> step 3 rename collection space fail")
      
      #metadata operation
      self.cs = self.db.get_collection_space(self.test_cs_newname)
      self.cs.create_collection("test_cl")
      self.assertTrue(self.is_collection_exist("test_cl"), "===> step 4 create collection fail")
      
      self.cs.drop_collection("test_cl")
      self.assertFalse(self.is_collection_exist("test_cl"), "===> step 5 drop collection fail")
      
   def tearDown(self):
      self.db.drop_collection_space(self.test_cs_newname)   
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
         if(self.test_cs_newname + "." + collection_name == cl_name):
            return True
      return False
         
         