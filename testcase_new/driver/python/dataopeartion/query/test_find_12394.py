# @decription: find records with option FLAGS
# @testlink:   seqDB-12394
# @interface:  query(self,kwargs)
#              query_one(self,kwargs)
# @author:     liuxiaoxuan 2017-8-29

import unittest
import datetime
from pysequoiadb.error import (SDBTypeError, SDBBaseError, SDBEndOfCursor,SDBError)
from lib import testlib

cs_name = "cs_12394"
cl_name = "cl_12394"
insert_nums = 100
class TestFind12394(unittest.TestCase):
   def setUp(self):
      testlib.print_setup_msg(self)
      self.db = testlib.default_db()
      self.create_cs_cl(cs_name,cl_name)
      self.insert_datas()

   def testFind12394(self):
      # query all
      expectAllRec = []
      for i in range(0,insert_nums):
         expectAllRec.append({"_id": i,"a": "test" + str(i)})

      flag_0 = 0
      self.query_all(expectAllRec,flag_0)
      flag_1 = 1
      self.query_all(expectAllRec, flag_1)

      # query one
      condition = {"_id":{"$gt": 5}}
      expectOneRec = {"_id": 6,"a": "test6"}
      flag_10 = 10
      self.query_one(expectOneRec, condition,flag_10)

   def tearDown(self):
      try:
         testlib.print_teardown_msg(self)
         self.db.drop_collection_space(cs_name)
         self.db.disconnect()
      except SDBBaseError as e:
         if(-34 != e.code):
            self.fail('teardown fail: ' + e.detail)         
				
   def clean_cs(self,csname):
      try:
         self.db.drop_collection_space(csname)
      except SDBBaseError as e:
         pass	
			
   def create_cs_cl(self,csname,clname):
      self.clean_cs(csname)
      try:
         self.cs = self.db.create_collection_space(csname)
         self.cl = self.cs.create_collection(clname)
         print( 'create cl success' )
      except SDBError as e:
         self.fail('create cl fail: ' + e.detail)    

   def insert_datas(self):
      flag = 0
      doc = []
      for i in range(0,insert_nums):
         doc.append({"_id":i,"a":"test" + str(i)})
      try:
         self.cl.bulk_insert(flag,doc)
      except SDBError as e:
         self.fail('insert fail: ' + e.detail) 

   def query_all(self,expectRec,flag):
      try:
         sort = {"_id": 1}
         cursor = self.cl.query(order_by = sort, flags = flag)
         actRec = []
         while True:
            try:
               rec = cursor.next()
               actRec.append(rec)
            except SDBEndOfCursor:
               break
         testlib.assert_list_equal(self,expectRec,actRec)
      except SDBBaseError as e:
         self.fail("query all error: " + e.detail)

   def query_one(self,expectRec,cond,flag):
      try:
         sort = {"_id": 1}
         rec = self.cl.query_one(order_by = sort,condition = cond,flags = flag)
         self.assertEqual( rec,expectRec)
      except SDBBaseError as e:
         self.fail("query one error: " + e.detail)