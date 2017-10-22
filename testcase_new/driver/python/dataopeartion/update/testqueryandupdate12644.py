# @decription: test query and update shardingkey 
# @testlink:   seqDB-12644
# @interface:  query_and_update(self,rule,kwargs)
# @author:     liuxiaoxuan 2017-10-20

import unittest
import datetime
from pysequoiadb import collection
from pysequoiadb.error import (SDBTypeError, SDBBaseError, SDBEndOfCursor, SDBError)
import time
from lib import testlib

class TestQueryAndUpdate12644(testlib.SdbTestBase):
   def setUp(self):
      if testlib.is_standalone():
         self.skipTest('current environment is standalone')	
	
      testlib.drop_cs(self.db, self.cs_name, ignore_not_exist=True)
      self.cs = self.db.create_collection_space(self.cs_name)
      cl_option = {"ShardingKey": {'a': 1}, "ShardingType": 'hash'}			
      self.cl = self.cs.create_collection(self.cl_name,options = cl_option)
		#insert records
      self.insert_datas()		

   def test_query_and_update_12644(self):  		
		
		# update shardingkey  
      modify_rule = {'$set': {'a': 'NewA'}}		
      cond = {'a': {'$gt' : 1, '$lt' : 10}}
      select = {"_id": { "$include": 0}} 
      sort = {'a' : 1}		
      hintway = {'' : '$shard'}
      skip = 2
      retrn = 3
		
		# check result	    
      expect_result = [{'a': 'NewA', 'b': 'test' + str(i)} for i in range(4,7)]
      self.check_query_update_result(expect_result,
		                               modify_rule, 
		                               condition = cond,
                                     selector = select, 
												 order_by = sort,
												 hint = hintway,
												 num_to_skip = skip,
                                     num_to_return = retrn,
                                     flags = collection.QUERY_FLG_KEEP_SHARDINGKEY_IN_UPDATE,       												 
							                return_new = True)
							   						  
   def tearDown(self):
      if self.should_clean_env():
         self.db.drop_collection_space(self.cs_name)

   def insert_datas(self):
      insert_nums = 100
      flag = 0
      doc = []
      for i in range(0, insert_nums):
         doc.append({ "a": i, "b": 'test' + str(i)})
      try:
         self.cl.bulk_insert(flag, doc)
      except SDBError as e:
         self.fail('insert fail: ' + e.detail)
			
   def check_query_update_result(self, expect_result, rule, **kwargs):
      try:  
         cursor = self.cl.query_and_update(rule, **kwargs)
         act_result = testlib.get_all_records_noid(cursor)
         self.assertListEqualUnordered(expect_result, act_result)
      except SDBBaseError as e:
         self.fail('query and update fail: ' + e.detail)
