# @decription: test aggregate
# @testlink:   seqDB-12480
# @interface:  aggregate(self,aggregate_options)
# @author:     liuxiaoxuan 2017-8-30

import unittest
import datetime
from pysequoiadb.error import (SDBTypeError, SDBBaseError, SDBEndOfCursor,SDBError)
from bson.son import SON
from lib import testlib

cs_name = "cs_12480"
cl_name = "cl_12480"
insert_nums = 10
class TestAggregate12480(unittest.TestCase):
   def setUp(self):
      testlib.print_setup_msg(self)
      self.db = testlib.default_db()
      self.create_cl()
      self.insert_datas()

   def testAggregate12480(self):
      match = SON({'$match': {'name': {'$exists': 1}}})
      group = SON({'$group': {'_id': '$major', 'avg_age': {'$avg': '$age'}, 'major': {'$first': '$major'}}})
      sort = SON({'$sort': {'avg_age': 1}})
      skip = {'$skip': 1}
      limit = {'$limit': 2}

      list_aggr_options = [match, group, sort, skip, limit]
      tuple_aggr_options = (match, group, sort, skip, limit)

      list_cursor = self.cl.aggregate(list_aggr_options)
      tuple_cursor = self.cl.aggregate(list_aggr_options)

      list_actResult = self.get_act_result(list_cursor)
      tuple_actResult = self.get_act_result(tuple_cursor)

      expectResult = [{'avg_age': 20.0, 'major': 'major5'}, \
                      {'avg_age': 21.0, 'major': 'major1'}]

      testlib.assert_list_equal(self, list_actResult, expectResult)
      testlib.assert_list_equal(self, tuple_actResult, expectResult)
            
   def tearDown(self):
      try:
         testlib.print_teardown_msg(self)
         self.db.drop_collection_space(cs_name)
         self.db.disconnect()
      except SDBBaseError as e:
         if(-34 != e.code):
            print(e.detail)
            raise e    
	
   def clean_cs(self):
      try:
         self.db.drop_collection_space(cs_name)
      except SDBBaseError as e:
         pass	
			
   def create_cl(self):
      self.clean_cs()
      try:
         self.cs = self.db.create_collection_space(cs_name)
         self.cl = self.cs.create_collection(cl_name)
         print( 'create cl success' )
      except SDBBaseError as e:
         print(e.detail) 
         raise e    
  
   def insert_datas(self):   
      for i in range(0,insert_nums):
         try:
            self.cl.insert({"_id":i, "name":"test" + str(i),"major":"major" + str(i%10),"age":20+i%5})  
         except SDBError as e:
            print(e.detail) 
            raise e

   def get_act_result(self,cursor):
      actResult = []
      while True:
         try:
            rec = cursor.next()
            actResult.append(rec)
         except SDBEndOfCursor:
            break
      return actResult
         
if __name__ == "__main__":
    unittest.main() 