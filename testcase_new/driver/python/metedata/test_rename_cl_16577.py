# @decription rename collection and metadata operation
# @testlink   seqDB-16577
# @interface  rename_collection	( self, old_name, new_name, options = None )		
# @author     yinzhen 2018-12-06

from lib import testlib
from lib import sdbconfig
from pysequoiadb.error import (SDBBaseError, SDBEndOfCursor)


class TestRenameCL16577(testlib.SdbTestBase):
   def setUp(self):
      # create cs 
      testlib.drop_cs(self.db, self.cs_name, ignore_not_exist=True)      
      self.cs = self.db.create_collection_space(self.cs_name)

   def test_rename_cs_16577(self):
      #create collection
      cl_name = "test_cl_16577"
      self.cl = self.cs.create_collection(cl_name)
      self.assertTrue(self.is_collection_exist(cl_name), "===> step 1 create collection fail")
      
      #rename collection
      cl_new_name = "test_cl_new_16577"
      self.cs.rename_collection(cl_name, cl_new_name)
      self.assertTrue(self.is_collection_exist(cl_new_name), "===> step 2 rename collection fail")
      self.assertFalse(self.is_collection_exist(cl_name), "===> step 3 rename collection fail")
      
      #insert
      self.cl = self.cs.get_collection(cl_new_name)
      obj_id = self.cl.insert({"name":"zsan"})
      cursor = self.cl.query()
      record = cursor.next()
      self.assertEqual(str(obj_id), str(record["_id"]), "===> step 4 insert fail")
      
      #update
      self.cl.update({"$set":{"name":"lisi"}})
      cursor = self.cl.query()
      record = cursor.next()
      self.assertEqual(str(obj_id), str(record["_id"]), "===> step 5 update fail")
      
      #delete
      self.cl.delete()
      try:
         self.cl.query().next()
      except SDBBaseError as e:
         self.assertEqual(-29, e.code, "===> step 6 delete fail")
      
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
         
         