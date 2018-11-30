# @decription appoint hint/num_to_skip/num_to_return to get SDB_SNAP_ACCESSPLANS information
# @testlink   seqDB-16660
# @interface  get_snapshot	( self, snap_type, kwargs )		
# @author     yinzhen 2018-11-29

from lib import testlib
from lib import sdbconfig
from pysequoiadb.error import (SDBBaseError, SDBEndOfCursor)
import time

SDB_SNAP_ACCESSPLANS = 11
SDB_SNAP_CONFIGS = 13

class TestGetSnapshot16660(testlib.SdbTestBase):
   def setUp(self):
      # create cs cl   
      testlib.drop_cs(self.db, self.cs_name, ignore_not_exist=True)      
      self.cs = self.db.create_collection_space(self.cs_name)
      self.cl = self.cs.create_collection(self.cl_name)
		 
   def test_get_snapshot_16660(self):
      #insert data
      self.insert_records()
      
      # get_snapshot snap_type is SDB_SNAP_ACCESSPLANS
      actResult = self.get_snapshot_result(SDB_SNAP_ACCESSPLANS, num_to_skip = 10, num_to_return = 10)
      actResultCount = len(actResult)
      expResultCount = 10
      self.assertEqual(actResultCount, expResultCount, "actResult length is not equal to expResult length ====> NOT appoint")
      
      actResult = self.get_snapshot_result(SDB_SNAP_ACCESSPLANS, condition = {"NodeName":{"$exists":1}}, num_to_skip = 8, num_to_return = 8)
      actResultCount = len(actResult)
      expResultCount = 8
      self.assertEqual(actResultCount, expResultCount, "actResult length is not equal to expResult length ====> appoint condition")
      
      actResult = self.get_snapshot_result(SDB_SNAP_ACCESSPLANS, selector = {"NodeName":""}, num_to_skip = 9, num_to_return = 9)
      actResultCount = len(actResult)
      expResultCount = 9
      self.assertEqual(actResultCount, expResultCount, "actResult length is not equal to expResult length ====> appoint selector")
      
      actResult = self.get_snapshot_result(SDB_SNAP_ACCESSPLANS, order_by = {"NodeName":-1}, num_to_skip = 12, num_to_return = 12)
      actResultCount = len(actResult)
      expResultCount = 12
      self.assertEqual(actResultCount, expResultCount, "actResult length is not equal to expResult length ====> appoint order_by")
      
      actResult = self.get_snapshot_result(SDB_SNAP_CONFIGS, hint = {"$Option":{"expand":"false"}}, num_to_skip = 2, num_to_return = 2)
      actResultCount = len(actResult)
      expResultCount = 2
      self.assertEqual(actResultCount, expResultCount, "actResult length is not equal to expResult length ====> appoint hint")
	
   def tearDown(self):
      self.db.drop_collection_space(self.cs_name)   
      self.db.disconnect()
	    
   def get_snapshot_result(self, snap_type, **kwargs):
      result = []
      cursor = self.db.get_snapshot(snap_type, **kwargs)
      while True:
         try:
            rc = cursor.next()
            result.append(rc) 
         except SDBEndOfCursor:
            break
      cursor.close()
      return result
   
   def insert_records(self):
      doc = []
      for i in range(0, 1000):
         doc.append({"a": i , "b": "test" + str(i)})
      try:
         flags = 0
         self.cl.bulk_insert(flags, doc)
      except SDBBaseError as e:
         self.fail('insert fail: ' + e.detail)