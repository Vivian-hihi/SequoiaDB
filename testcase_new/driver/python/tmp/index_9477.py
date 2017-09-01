#! /usr/bin/python

import pysequoiadb
import unittest
#from nose import with_setup
from pysequoiadb import client
from pysequoiadb import const
from pysequoiadb.error import (SDBTypeError,
                               SDBBaseError,
                               SDBEndOfCursor,
                               SDBError)

from bson.objectid import ObjectId
from bson.decimal import Decimal

cs_name = "test9477"
cl_name = "testindex9477"
#class
class IndexTestCase(unittest.TestCase):
   @classmethod
   def create_cl(cls):
     try:
         print( '---begin create cs---')
         cls.cs = cls.db.create_collection_space(cs_name)            
         
         #create a cl
         cls.cl = cls.cs.create_collection(cl_name, {"ReplSize":0})
         print( '---create cl success---' )   
     except SDBError as e:
         pysequoiadb._print(e.detail) 
         raise e
         
   @classmethod   
   def insertDatas( cls ):   
      print( '---begin to insert records---' )
      for i in range(1,100):
         cls.cl.insert({"_id":i,"age":i,"name":"mike"+str(i)})
         
   @classmethod
   def setUp(cls):
      cls.db = client("192.168.31.1", 11810)      
      cls.create_cl()
      cls.insertDatas()  

   @classmethod
   def tearDown(cls):
     cls.db.drop_collection_space(cs_name)

   def test_getindex(self):
      try:
         pysequoiadb._print("---create index---")     
         index = {'Item':1, 'Rank':-1}
         index_name = 'testidx'
         self.cl.create_index(index, index_name, False, False, 128)
         
         pysequoiadb._print("---check index---")         
         rc = self.cl.get_indexes(index_name)
         doc=rc.next()        
         self.assertEqual(doc["IndexDef"]["name"],index_name)
         
         pysequoiadb._print("---begin to check data---")   
         rec = self.cl.query(condition={"age":4},hint={"":"testidx"}).next()
         exprec = {"_id":4,"age":4,"name":"mike4"}
         self.assertEqual( rec,exprec) 

      except (SDBTypeError, SDBBaseError) as e:
         assert False
         pysequoiadb._print(e)
      except SDBBaseError as e:
         assert False
         pysequoiadb._print(e.detail) 
if __name__ == "__main__":
   unittest.main()      

         
