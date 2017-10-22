# @decription: test update shardingkey and non-shardingkey
# @testlink:   seqDB-12641
# @interface:  update(self,rule,kwargs)
# @author:     liuxiaoxuan 2017-10-20

import unittest
import datetime
from pysequoiadb import collection
from pysequoiadb.error import (SDBTypeError, SDBBaseError, SDBEndOfCursor, SDBError)
import time
from lib import testlib

class TestUpdate12641(testlib.SdbTestBase):
   def setUp(self):
      if testlib.is_standalone():
         self.skipTest('current environment is standalone')	
	
      testlib.drop_cs(self.db, self.cs_name, ignore_not_exist=True)
      self.cs = self.db.create_collection_space(self.cs_name)
      cl_option = {"ShardingKey": {'a': 1, 'b': -1}, "ShardingType": 'hash'}			
      self.cl = self.cs.create_collection(self.cl_name,options = cl_option)
		#insert records
      self.insert_datas()		

   def test_update_12641(self):  
	
		# update shardingkey  
      modify_rule1 = {'$set': {'a': 'NewA', 'b': 'NewB'}}		
      condition1 = {'a': {'$gt' : 1, '$lt' : 5}}
      hint1 = {'' : '$shard'}
      self.update_data(modify_rule1, condition = condition1, hint = hint1,   
							  flags = collection.UPDATE_FLG_KEEP_SHARDINGKEY )
							   						  
		# check result		
      expect_result1 = [{'a': 'NewA', 'b': 'NewB', 'c': 'oldValue' + str(i)} for i in range(2,5)]
      matcher1 = {'a' : 'NewA'}
      sort = {'a' : 1}
      self.check_update_result(expect_result1, condition = matcher1, order_by = sort)

      # update non-shardingkey   
      modify_rule2 = {'$set': {'c' : 'NewC'}}		
      condition2 = {'a' : {'$et' : 'NewA'}}
      self.update_data(modify_rule2, condition = condition2,
		                 flags = collection.UPDATE_FLG_KEEP_SHARDINGKEY )
							  
		# check result	
      expect_result2 = [{'a': 'NewA', 'b': 'NewB', 'c': 'NewC'} for i in range(3)]
      matcher2 = {'c' : 'NewC'}
      self.check_update_result(expect_result2, condition = matcher2)					  
		
   def tearDown(self):
      if self.should_clean_env():
         self.db.drop_collection_space(self.cs_name)

   def insert_datas(self):
      insert_nums = 100
      flag = 0
      doc = []
      for i in range(0, insert_nums):
         doc.append({ "a": i, "b": 'test' + str(i), "c": 'oldValue' + str(i)})
      try:
         self.cl.bulk_insert(flag, doc)
      except SDBError as e:
         self.fail('insert fail: ' + e.detail)
			
   def update_data(self,rule, **kwargs):
      try:  
         self.cl.update(rule, **kwargs)
      except SDBBaseError as e:
         self.fail('update fail: ' + e.detail)

   def check_update_result(self, expect_esult, **kwargs):
      cursor = self.cl.query(**kwargs)	 
      act_result = testlib.get_all_records_noid(cursor)
      
      self.assertListEqualUnordered(expect_esult, act_result)
