# @decription: test explain
# @author:     liuxiaoxuan 2017-8-30

from bson.py3compat import (PY3,long_type)
import unittest
import datetime
from pysequoiadb import client
from pysequoiadb import collectionspace
from pysequoiadb.error import (SDBTypeError, SDBBaseError, SDBEndOfCursor,SDBError)
from lib.config import *

cs_name = "cs_12475"
cl_name = "cl_12475"
insert_nums = 100
class TestExplain12475(unittest.TestCase):
   def setUp(self):
      try:
         print(datetime.datetime.now())
         config = Config()
         self.db = client( config.host_name, config.service )
         self.clean_cs()			
         self.create_cl()
         self.insert_datas()         
      except SDBBaseError as e:
         print(e.detail)
         raise e        
                  
   def testExplain12475(self):
      try:  
         aIndex = {'a':1}
         aIdxName = 'a'
          
         self.create_index(aIndex,aIdxName)
         self.check_index(aIdxName)
         self.get_explain(aIdxName)
          
         self.dropIndex(aIdxName)
          
      except SDBBaseError as e:
         print(e.detail) 
         assert False
            
   def tearDown(self):
      try:
         print(datetime.datetime.now())
         self.db.drop_collection_space(cs_name)
         self.db.disconnect()
      except SDBBaseError as e:
         if(-34 != e.code):
            print(e.detail)
            raise e    
				
   def clean_cs(self):
      try:
         print( '---begin to clean cs---')
         self.db.drop_collection_space(cs_name) 
      except SDBError as e:
         pass	
   
   def create_cl(self):
      try:
         print( '---begin to create cs---')
         self.cs = self.db.create_collection_space(cs_name)            
    
         self.cl = self.cs.create_collection(cl_name)
         print( '---create cl success---' )   
      except SDBError as e:
         print(e.detail) 
         raise e    
  
   def insert_datas(self):   
      print( '---begin to insert records---' )
      for i in range(1,insert_nums):
         try:
            self.cl.insert({"_id":i, "a":"test" + str(i)})  
         except SDBError as e:
            print(e.detail) 
            raise e        
    
   def check_index(self,index_name):
      print( '---check one index---' )
      try:
         cursor = self.cl.get_indexes(index_name);  
         rec = cursor.next()
         act_idx_name = rec['IndexDef']['name']
         exp_idx_name = index_name
         self.assertEqual(exp_idx_name, act_idx_name,'index name is not exist')
      except SDBBaseError as e:
         print(e.detail) 
         raise e                 
       
   def dropIndex(self,index_name):   
      print( '---drop index---' )
      try:
         self.cl.drop_index(index_name)  
      except SDBBaseError as e:
         print(e.detail) 
         raise e      
      
   def create_index(self,index,index_name):   
      print( '---create index---' )
      try:
         self.cl.create_index(index, index_name, False, False, 64)
      except SDBBaseError as e:
         print(e.detail) 
         raise e       
      
   def get_explain(self,index_name):   
      print( '---check data with index---' )
      try:
			
         cursor = self.cl.explain(condition={"a":{'$et':10}},\
                                  selected={"a":{"$include":1}},\
                                  order_by={"_id":1},\
                                  hint={"":index_name},\
                                  num_to_skip=long_type(1),\
                                  num_to_return=long_type(5),\
                                  flags=1)
         rec = cursor.next()  
         expScanType='ixscan'
         expIdxName = index_name
         expQuery = {'$and': [{'a': {'$et': 10}}]}
         scanType = rec['ScanType']
         idxName = rec['IndexName']
         query = rec['Query']
         self.assertEqual( expScanType,scanType) 
         self.assertEqual( expIdxName,idxName)
         self.assertEqual( expQuery,query)
      except SDBBaseError as e:
         print(e.detail) 
         raise e               	  
if __name__ == "__main__":
    unittest.main() 
