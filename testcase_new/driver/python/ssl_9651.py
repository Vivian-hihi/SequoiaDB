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

cs_name = "testssl"
cl_name = "testssl"
#class
class sslTestCase(unittest.TestCase):
  # @classmethod
   def create_cl(cls):      
      cls.cs = cls.db.create_collection_space(cs_name)
      #create a cl
      
      cls.cl = cls.cs.create_collection(cl_name, {"ReplSize":0})
      print( '---create cl success---' )  

  # @classmethod
   def setUp(cls):
      cls.db = client("192.168.31.2", 11810,"","",False)     
      
    # @classmethod
   def tearDown(cls):
     cls.db.drop_collection_space(cs_name)

   #coord no use ssl ,and the connect no use ssl
   def test_nossl(self):
      #seqDB-9650:
      try:
         self.create_cl()         
         pysequoiadb._print("---test cl---")     
         cl = self.db.get_collection(cs_name+"."+cl_name)         
         print( "name=",cl.get_full_name() )
         self.assertEqual( cl.get_full_name(),cs_name+"."+cl_name)  
      except (SDBTypeError, SDBBaseError) as e:
         assert False
         pysequoiadb._print(e)
      except SDBBaseError as e:
         assert False
         pysequoiadb._print(e.detail)   
      
      #seqDB-9651:
      pysequoiadb._print("---test ssl connect and coord no ssl---") 
      try:
         self.db = client("192.168.31.28", 11810,"","",True)
      except (SDBTypeError, SDBBaseError) as e:
         self.assertEqual(str(e),"Network Error: Failed to connect to 192.168.31.28:11810")
         assert True
         pysequoiadb._print(e)
      except SDBBaseError as e:
         assert False
         pysequoiadb._print(e.detail)            
   
if __name__ == "__main__":
   unittest.main()      

         
