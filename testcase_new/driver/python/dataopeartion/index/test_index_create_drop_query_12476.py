# @decription: create/drop/query common index
# @author:     liuxiaoxuan 2017-8-30

import unittest
import datetime
from pysequoiadb import client
from pysequoiadb import collectionspace
from pysequoiadb.error import (SDBTypeError, SDBBaseError, SDBEndOfCursor,SDBError)
from lib.config import *

cs_name = "cs_12476"
cl_name = "cl_12476"
insert_nums = 100
class TestIndex12476(unittest.TestCase):
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
 
   def testIdIndex12476(self):
      try:  
         aIndex = {'a':1}
         aIdxName = 'a'
         bIndex = {'b':1}
         bIdxName = 'b'     
          
         isOption = True          
         self.create_index(aIndex,aIdxName,isOption)
         self.create_index(bIndex,bIdxName,not isOption)
         self.query_datas(aIdxName)
          
         self.check_indexes()
         self.check_one_index(aIdxName)
         self.check_one_index(bIdxName)
          
         self.drop_index(aIdxName)
         self.drop_index(bIdxName)
         self.query_datas("")
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
            self.cl.insert({"_id":i,"a":i, "b":"test" + str(i)})  
         except SDBError as e:
            print(e.detail) 
            raise e        
    
   def check_one_index(self,index_name):
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
    
   def check_indexes(self):
      print( '---check all indexes---' )
      try:
         cursor = self.cl.get_indexes(); 
         idxs = []
         while True:
            try:
               rec = cursor.next()
               index_name = rec['IndexDef']['name']
               idxs.append(index_name)
               print(index_name)
            except SDBEndOfCursor:
               break
         if('a' not in idxs or 'b' not in idxs):
            assert False        
      except SDBBaseError as e:
         print(e.detail) 
         raise e             
       
   def drop_index(self,index_name):   
      print( '---drop index---' )
      try:
         self.cl.drop_index(index_name)  
      except SDBBaseError as e:
         print(e.detail) 
         raise e      
      
   def create_index(self,index,index_name,isOption):   
      print( '---create index---' )
      try:
         isUnique = False
         isEnforced = False
         buffer_size = 0
         if(isOption):
            isUnique = True
            isEnforced = True
            buffer_size = 128
         self.cl.create_index(index, index_name, isUnique, isEnforced, buffer_size)
      except SDBBaseError as e:
         print(e.detail) 
         raise e       
      
   def query_datas(self,index_name):   
      print( '---check data with index---' )
      try:
         rec = self.cl.query(condition={"a":{'$et':10}},\
                             order_by={"_id":1},\
                             hint={"":index_name},\
                             flags=1).next()
         exprec = {"_id":10,"a":10, "b":"test" + str(10)}
         self.assertEqual( rec,exprec)                     
      except SDBBaseError as e:
         print(e.detail) 
         raise e                              
                        
if __name__ == "__main__":
    unittest.main() 