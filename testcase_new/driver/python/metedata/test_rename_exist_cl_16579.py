# @decription rename collection exist
# @testlink   seqDB-16579
# @interface  rename_collection	( self, old_name, new_name, options = None )		
# @author     yinzhen 2018-12-06

from lib import testlib
from lib import sdbconfig
from pysequoiadb.error import (SDBBaseError, SDBEndOfCursor)


class TestRenameExistCL16579(testlib.SdbTestBase):
   def setUp(self):
      # create cs 
      testlib.drop_cs(self.db, self.cs_name, ignore_not_exist=True)      
      self.cs = self.db.create_collection_space(self.cs_name)
      self.test_cl_name = "test_cl_16579"
      self.test_cl_name2 = "test_cl2_16579"

   def test_rename_cs_16579(self):
      #create collection
      self.cs.create_collection(self.test_cl_name)
      self.assertTrue(self.is_collection_exist(self.test_cl_name), "===> step 1 create collection fail")
      
      #create another collection
      self.cs.create_collection(self.test_cl_name2)
      self.assertTrue(self.is_collection_exist(self.test_cl_name2), "===> step 2 create collection fail")
      
      #rename exist collection
      try:
         self.cs.rename_collection(self.test_cl_name2, self.test_cl_name)
      except SDBBaseError as e:
         self.assertEqual(-22, e.code, "===> step 3 check rename exist collection fail")
      
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
         
         