# @decription: find records with options
# @author:     liuxiaoxuan 2017-8-29

from bson.py3compat import (PY3,long_type)
import unittest
import datetime
from pysequoiadb import client
from pysequoiadb import collectionspace
from pysequoiadb.error import (SDBTypeError, SDBBaseError, SDBEndOfCursor,SDBError)
from lib.config import *

cs_name = "cs_12470_12471_12474"
cl_name = "cl_12470_12471_12474"
insert_nums = 100
class TestFind12470(unittest.TestCase):
   def setUp(self):
      try:
         print(datetime.datetime.now())
         config = Config()
         self.db = client( config.host_name, config.service )  
         self.create_cl()
         self.insert_datas()                 
      except SDBBaseError as e:
         print(e.detail)
         raise e        
         
   def testquery12471(self):
      try:
         print("---begin to check data---") 
         i = 1
         cursor = self.cl.query(order_by={"_id":1} , flags=0)
         flag = True
         while True:
            try:
               rec = cursor.next()
               exprec = {"_id":i,"a":"test" + str(i)}              
               if(rec != exprec):
                  flag = False
                  print(rec)  
                  print(exprec)
                  break
               i = i + 1                       
            except SDBEndOfCursor:
               break      
         self.assertEqual( flag,True) 
                  
      except SDBBaseError as e:
         print(e.detail)  
         assert False       
                        
   def testQuery12474(self):   
      try:
         print("---begin to check data---")  
         skip = long_type(1)
 
         rec = self.cl.query_one(condition={"_id":{'$gt':4,'$lt':7}},\
                                 selected={"a":{"$elemMatch":"test" + str(5)}},\
                                 order_by={"_id":1},\
                                 hint={"":""},\
                                 num_to_skip=skip,\
                                 flags=1)
         exprec = {"_id":6,"a":"test" + str(6)}
         print(rec)
         print(exprec)
         self.assertEqual( rec,exprec)        
      except SDBBaseError as e:
         print(e.detail)  
         assert False
          
   def testQuery12470(self):   
      try:
         print("---begin to check data---")   
         skip = long_type(1)
         retrn = long_type(10)
        
         cursor = self.cl.query(condition={"_id":{'$gt':11,'$lt':50}},\
                                selected={"_id":{"$include":1}},\
                                order_by={"_id":1},\
                                hint={"":""},\
                                num_to_skip=skip,\
                                num_to_return=retrn,\
                                flags=1)
         i = 13
         flag = True
         while True:
            try:
               rec = cursor.next()
               exprec = {"_id":i,"a":"test" + str(i)}              
               if(rec != exprec):
                  flag = False
                  print(rec)  
                  print(exprec)
                  break
               i = i + 1                       
            except SDBEndOfCursor:
               break      
         self.assertEqual( flag,True)         
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
            self.cl.insert({"_id":i,"a":"test" + str(i)})  
         except SDBError as e:
            print(e.detail) 
            raise e  
if __name__ == "__main__":
    unittest.main() 
