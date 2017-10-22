# @decription: test upsert shardingkey
# @testlink:   seqDB-12643
# @interface:  upsert(self,rule,kwargs)
# @author:     liuxiaoxuan 2017-10-20

import unittest
import datetime
from pysequoiadb import collection
from pysequoiadb.error import (SDBTypeError, SDBBaseError, SDBEndOfCursor, SDBError)
import time
from lib import testlib

class TestUpsert12643(testlib.SdbTestBase):
   def setUp(self):
      if testlib.is_standalone():
         self.skipTest('current environment is standalone')	
	
      testlib.drop_cs(self.db, self.cs_name, ignore_not_exist=True)
      self.cs = self.db.create_collection_space(self.cs_name)
      cl_option = {"ShardingKey": {'a': 1}, "ShardingType": 'hash'}			
      self.cl = self.cs.create_collection(self.cl_name,options = cl_option)
		#insert records
      self.insert_datas()		

   def test_upsert_12643(self):		
		
		# when not match , insert new record  
      modify_rule1 = {'$set' : {'a' : 'InsertANewValue'}}		
      condition1 = {'a' : {'$et' : 'NotExistValue'}}
      hint1 = {'' : '$id'}
      setOnInsert = {'a' : 'InsertANewValue'}
      self.upsert_data(modify_rule1, condition = condition1, 
		                 hint = hint1, setOnInsert = setOnInsert,
							  flags = collection.UPDATE_FLG_KEEP_SHARDINGKEY )
							   						  
		# check result		
      expect_result1 = [{'a' : 'InsertANewValue'}]
      matcher1 = {'a' : 'InsertANewValue'}
      self.check_upsert_result(expect_result1, condition = matcher1)

      # when match , update record   
      modify_rule2 = {'$set' : {'a' : 'ShardingKeyUpdateToNewValue'}}		
      condition2 = {'a' : {'$gt' : 1 , '$lt' : 5}}
      self.upsert_data(modify_rule2, condition = condition2,
		                 flags = collection.UPDATE_FLG_KEEP_SHARDINGKEY )
							  
		# check result	      
      expect_result2 = [{'a': 'ShardingKeyUpdateToNewValue', 'b': 'test' + str(i)} for i in range(2,5)]
      matcher2 = {'a' : 'ShardingKeyUpdateToNewValue'}
      sort = {'_id' : 1}
      self.check_upsert_result(expect_result2, condition = matcher2, order_by = sort)					  
		
   #def tearDown(self):
      #if self.should_clean_env():
         #self.db.drop_collection_space(self.cs_name)

   def insert_datas(self):
      insert_nums = 100
      flag = 0
      doc = []
      for i in range(0, insert_nums):
         doc.append({ "a": i , "b" : 'test' + str(i)})
      try:
         self.cl.bulk_insert(flag, doc)
      except SDBError as e:
         self.fail('insert fail: ' + e.detail)
			
   def upsert_data(self,rule, **kwargs):
      try:  
         self.cl.upsert(rule, **kwargs)
      except SDBBaseError as e:
         self.fail('upsert fail: ' + e.detail)

   def check_upsert_result(self, expect_esult, **kwargs):
      cursor = self.cl.query(**kwargs)	 
      act_result = testlib.get_all_records_noid(cursor)
      
      self.assertListEqualUnordered(expect_esult, act_result)
