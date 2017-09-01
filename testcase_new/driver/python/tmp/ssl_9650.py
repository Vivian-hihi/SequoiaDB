#! /usr/bin/python

import pysequoiadb
import unittest
#from nose import with_setup
from pysequoiadb import client
from pysequoiadb import const
from pysequoiadb.error import (SDBTypeError,
                               SDBBaseError,
                               SDBEndOfCursor)

from bson.objectid import ObjectId

cs_name = "gymnasium"
cl_name = "testssl"
#class
class sslTestCase(unittest.TestCase):
   @classmethod
   def create_cl(cls):
      
      cls.cs = cls.db.create_collection_space(cs_name)
      #create a cl
      
      cls.cl = cls.cs.create_collection(cl_name, {"ReplSize":0})
      print( '---create cl success---' )   
  
   @classmethod   
   def insertDatas( cls ):   
      print( '---begin to insert records---' )
      cls.cl.insert({"age":23,"name":"mike10","test":[{"b":1},{"c":"test"}],"decimal":{"$decimal":"123.345"}})
   

   @classmethod
   def setUp(cls):
      cls.db = client("192.168.31.1", 11810,"","",True)
  
   @classmethod
   def tearDown(cls):
     cls.db.drop_collection_space(cs_name)

   #coord use ssl ,and the connect use ssl
   def test_ssl(self):
      try:
         self.create_cl()
         self.insertDatas()
         
         pysequoiadb._print("---Before test cl---")     
         cl = self.db.get_collection(cs_name+"."+cl_name)         
         print( "name=",cl.get_full_name() )
         self.assertEqual( cl.get_full_name(),cs_name+"."+cl_name) 
         self.assertEqual( cl.get_count(),1)    
      except (SDBTypeError, SDBBaseError) as e:
         assert False
         pysequoiadb._print(e)
      except SDBBaseError as e:
         assert False
         pysequoiadb._print(e.detail)     
     
if __name__ == "__main__":
   unittest.main()      

         
