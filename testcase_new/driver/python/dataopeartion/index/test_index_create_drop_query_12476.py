# @decription: create/drop/query common index
# @testlink:   seqDB-12476
# @interface:  create_index(self,index_def,idx_name,is_unique,is_enforced,buffer_size)
#              drop_index(self,idx_name)
#              get_indexes(self,idx_name)
# @author:     liuxiaoxuan 2017-8-30

import unittest
import datetime
from pysequoiadb.error import (SDBTypeError, SDBBaseError, SDBEndOfCursor,SDBError)
from lib import testlib

cs_name = "cs_12476"
cl_name = "cl_12476"
insert_nums = 100
class TestIndex12476(unittest.TestCase):
   def setUp(self):
      testlib.print_setup_msg(self)
      self.db = testlib.default_db()
      self.create_cl()
      self.insert_datas() 
 
   def testIndex12476(self):
      try:  
         aIndex = {'a':1}
         aIdxName = 'a'    
         isOption = True          
         self.create_index(aIndex,aIdxName,isOption)

         condition = {"a": {'$gt': 1, '$lt': 11}}
         expectResult = []
         for i in range(2,11):
            expectResult.append({"_id":i,"a":i,"b":"test" + str(i)})
         self.query_datas(expectResult, condition, aIdxName)

         expScanType = 'ixscan'
         expIdxName = aIdxName
         expQuery = {"$and": [{ "a": {"$gt":1}},{"a": {"$lt": 11}}]}
         expectExplainRec = {"expScanType": expScanType,"expIdxName": expIdxName,"Query": expQuery}
         self.check_explain(expectExplainRec,condition)
          
         expectIdxResult = {"expIdName": expIdxName,"expKey": aIndex,"expUnique": isOption,"expEnforced": isOption}
         self.check_indexes(expectIdxResult,aIdxName)
          
         self.drop_index(aIdxName)
         expScanType = 'tbscan'
         expIdxName = ''
         expectExplainRec = {"expScanType": expScanType, "expIdxName": expIdxName, "Query": expQuery}
         self.query_datas(expectResult,condition,None)
         self.check_explain(expectExplainRec,condition)
			
      except SDBBaseError as e:
         print(e.detail) 
         self.fail("test idIndex failed" + e.detail)
		 
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
      except SDBError as e:
         pass			
			
   def create_cl(self):
      self.clean_cs()
      try:
         self.cs = self.db.create_collection_space(cs_name)              
         self.cl = self.cs.create_collection(cl_name)
         print( 'create cl success' )
      except SDBError as e:
         print(e.detail) 
         raise e    
  
   def insert_datas(self):   
      for i in range(1,insert_nums):
         try:
            self.cl.insert({"_id":i,"a":i, "b":"test" + str(i)})  
         except SDBError as e:
            print(e.detail) 
            raise e

   def create_index(self, index, index_name, isOption):
      try:
         if isOption:
            isUnique = True
            isEnforced = True
            buffer_size = 128
            self.cl.create_index(index, index_name, isUnique, isEnforced, buffer_size)
         else:
            pass
         print ('create index success')

      except SDBBaseError as e:
         self.fail('create index fail: ' + e.detail)

   def drop_index(self,index_name):   
      try:
         self.cl.drop_index(index_name)  
         print('drop index success')
      except SDBBaseError as e:
         self.fail('drop index fail: ' + e.detail)

   def check_indexes(self,expectResult,index_name):
      try:
         if index_name == None:
            cursor = self.cl.get_indexes();
         else:
            cursor = self.cl.get_indexes(index_name);
         idxs = []
         while True:
            try:
               rec = cursor.next()
               index_name = rec['IndexDef']['name']
               key = rec['IndexDef']['key']
               isUnique = rec['IndexDef']['unique']
               isEnforced = rec['IndexDef']['enforced']
               self.assertEqual(index_name,expectResult['expIdName'])
               self.assertEqual(key,expectResult['expKey'])
               self.assertEqual(isUnique,expectResult['expUnique'])
               self.assertEqual(isEnforced,expectResult['expEnforced'])
            except SDBEndOfCursor:
               break
      except SDBBaseError as e:
         self.fail('check index fail: ' + e.detail)

   def query_datas(self,expectResult,cond,index_name):   
      try:
         cursor = self.cl.query(condition=cond,\
                             order_by={"_id":1},\
                             hint={"":index_name},\
                             flags=1)
         actResult = []							  
         while True:
            try:
               rec = cursor.next()
               actResult.append(rec)
            except SDBEndOfCursor:
               break
         testlib.assert_list_equal(self, actResult, expectResult)
      except SDBBaseError as e:
         self.fail('query fail: ' + e.detail)

   def check_explain(self,expectExplainRec,cond):
      try:
         cursor = self.cl.explain(condition=cond,\
                             order_by={"_id":1},\
                             flags=1)
         rec = cursor.next()
         expScanType = expectExplainRec['expScanType']
         expIdxName = expectExplainRec['expIdxName']
         expQuery = expectExplainRec['Query']['$and']
         actScanType = rec['ScanType']
         actIndexName = rec['IndexName']
         actQuery = rec['Query']['$and']
         self.assertEqual(expScanType, actScanType)
         self.assertEqual(expIdxName, actIndexName)
         testlib.assert_list_equal(self, expQuery, actQuery)
      except SDBBaseError as e:
         self.fail('check explain fail: ' + e.detail)
                        
if __name__ == "__main__":
    unittest.main() 