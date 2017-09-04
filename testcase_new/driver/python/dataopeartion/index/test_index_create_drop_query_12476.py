# @decription: create/drop/query common index
# @testlink:   seqDB-12476
# @interface:  create_index(self,index_def,idx_name,is_unique,is_enforced,buffer_size)
#              drop_index(self,idx_name)
#              get_indexes(self,idx_name)
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
 
   def testIdIndex12476(self):
      try:  
         aIndex = {'a':1}
         aIdxName = 'a'    
         isOption = True          
         self.create_index(aIndex,aIdxName,isOption)
         
         expectResult = []
         for i in range(11,20):
            expectResult.append({"_id":i,"a":i,"b":"test" + str(i)})
         expectExplainRec = ['ixscan',aIdxName]	
         condition = {"a":{'$gt':10,'$lt':20}}		
         self.query_datas(expectResult,condition,aIdxName)
         self.get_explain(expectExplainRec,condition)
          
         expectIdxResult = [aIdxName,aIndex,isOption]
			#self.check_indexes(expectResult)
         self.check_indexes(expectIdxResult,aIdxName)
          
         self.drop_index(aIdxName)
			
         tbscan = ""
         expectExplainRec = ['tbscan',tbscan]			
         self.query_datas(expectResult,condition,tbscan)
         self.get_explain(expectExplainRec,condition)
			
      except SDBBaseError as e:
         print(e.detail) 
         self.fail("test idIndex failed" + e.detail)
		 
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
         self.cs = self.db.create_collection_space(cs_name)              
         self.cl = self.cs.create_collection(cl_name)
         print( '---create cl success---' )   
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
    
   def check_indexes(self,expectResult,index_name):
      try:
         if index_name==None:
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
               self.assertEqual(index_name,expectResult[0])
               self.assertEqual(key,expectResult[1])
               self.assertEqual(isUnique,expectResult[2])
               self.assertEqual(isEnforced,expectResult[2])							
            except SDBEndOfCursor:
               break   
      except SDBBaseError as e:
         print(e.detail) 
         raise e             
       
   def drop_index(self,index_name):   
      try:
         self.cl.drop_index(index_name)  
         print('drop index success')
      except SDBBaseError as e:
         print(e.detail) 
         raise e      
      
   def create_index(self,index,index_name,isOption):   
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
         print(e.detail) 
         raise e       
      
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
         self.check_result(actResult,expectResult)	                     
      except SDBBaseError as e:
         print(e.detail) 
         raise e   
		
   def check_result(self,actResult,expectResult):
      if not( len(actResult) == len(expectResult) ):
         self.fail('actResult is not equal to expectResult')
		
      flag = True
      size = len(actResult)
      for i in range(0,size):
         if not(actResult[i] == expectResult[i]):
            print(actResult[i])
            print(expectResult[i])
            flag = False
            break
      self.assertEqual(flag, True)	    

   def get_explain(self,expectExplainRec,cond):   
      try:
         cursor = self.cl.explain(condition=cond,\
                             order_by={"_id":1},\
                             flags=1)
         rec = cursor.next()
         actScanType = rec['ScanType']
         actIndexName = rec['IndexName']
         actExplainRec = [actScanType,actIndexName]
         self.check_explain(actExplainRec,expectExplainRec)	                     
      except SDBBaseError as e:
         print(e.detail) 
         raise e   
		
   def check_explain(self,actExplainRec,expectExplainRec):
      if not( len(actExplainRec) == len(expectExplainRec) ):
         self.fail('actExplainRec is not equal to expectExplainRec')
		
      flag = True
      size = len(actExplainRec)
      for i in range(0,size):
         if not(actExplainRec[i] == expectExplainRec[i]):
            print(actExplainRec[i])
            print(expectExplainRec[i])
            flag = False
            break
      self.assertEqual(flag, True)	 
                        
if __name__ == "__main__":
    unittest.main() 