# @decription rename collection_space  exist
# @testlink   seqDB-16575
# @interface  rename_collection_space	( self, old_name, new_name, options = None )		
# @author     yinzhen 2018-12-06

from lib import testlib
from lib import sdbconfig
from pysequoiadb.error import (SDBBaseError, SDBEndOfCursor)


class TestRenameExistCS16575(testlib.SdbTestBase):
   def setUp(self):
      self.test_cs_name1 = "test1_16575"
      self.test_cs_name2 = "test2_16575"
      testlib.drop_cs(self.db, self.test_cs_name1, ignore_not_exist=True)
      testlib.drop_cs(self.db, self.test_cs_name2, ignore_not_exist=True)      
		 
   def test_rename_cs_16575(self):
      #create collection space
      self.db.create_collection_space(self.test_cs_name1)
      self.assertTrue(self.is_collection_space_exist(self.test_cs_name1), "===> step 1 create collection space fail")
      
      #create another collection space
      self.db.create_collection_space(self.test_cs_name2)
      self.assertTrue(self.is_collection_space_exist(self.test_cs_name2), "===> step 2 create collection space fail")
      
      #rename exist collection space
      try:
         self.db.rename_collection_space(self.test_cs_name2, self.test_cs_name1)
      except SDBBaseError as e:
         self.assertEqual(-33, e.code, "===> step 3 check rename exist collection space fail")
         
   def tearDown(self):
      self.db.drop_collection_space(self.test_cs_name1)
      self.db.drop_collection_space(self.test_cs_name2)  
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