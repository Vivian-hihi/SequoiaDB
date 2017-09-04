# @decription: test aggregate
# @author:     liuxiaoxuan 2017-8-30

import unittest
import datetime
from pysequoiadb import client
from pysequoiadb import collectionspace
from pysequoiadb.error import (SDBTypeError, SDBBaseError, SDBEndOfCursor,SDBError)
from lib.config import *
from bson.son import SON

cs_name = "cs_12480"
cl_name = "cl_12480"
insert_nums = 100
class TestAggregate12480(unittest.TestCase):
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
                  
   def testAggregate12480(self):
      try:  
         self.check_aggregate()
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
            self.cl.insert({"_id":i, "name":"test" + str(i),"major":"major" + str(i%10),"age":20+i%5})  
         except SDBError as e:
            print(e.detail) 
            raise e        
    
   def check_aggregate(self):   
      print( '---check aggregate result---' )
      try:
         match = SON({'$match':{'name':{'$exists':1}}})
         group = SON({'$group':{'_id':'$major','avg_age':{'$avg':'$age'},'major':{'$first':'$major'}}})
         sort = SON({'$sort':{'avg_age':1}})
         skip  = {'$skip':0}
         limit = {'$limit':1}  
         aggregate_options = [match,group,sort,skip,limit]
         cursor = self.cl.aggregate(aggregate_options)
         rec = cursor.next()  
         expect = {'avg_age': 20.0, 'major': 'major0'}
         self.assertEqual( expect,rec) 
      except SDBBaseError as e:
         print(e.detail) 
         raise e               
         
if __name__ == "__main__":
    unittest.main() 