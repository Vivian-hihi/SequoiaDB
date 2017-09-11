# @decription: find records with query/get_count/close/current/next/close_all_cursors
# @testlink:   seqDB-12471
# @interface:  query(self,kwargs)
#              get_count(self,condition)
#              close(self)
#              current(self,ordered)
#              close_all_cursors(self)
# @author:     liuxiaoxuan 2017-8-29

from bson.py3compat import (PY3,long_type)
import unittest
import datetime
from pysequoiadb.error import (SDBTypeError, SDBBaseError, SDBEndOfCursor,SDBError)
from lib import testlib

cs_name = "cs_12471"
cl_name = "cl_12471"
insert_nums = 100

class TestFind12471(unittest.TestCase):
   def setUp(self):
      testlib.print_setup_msg(self)
      self.db = testlib.default_db()
      self.create_cl()
      self.insert_datas()

   def testFind12471(self):
	  #query all records
      expectResult = []
      for i in range(0,insert_nums):
         expectResult.append({"_id":i,"a":"test" + str(i)})		   
      self.query_datas(expectResult)

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
      except SDBBaseError as e:
         pass

   def create_cl(self):
      self.clean_cs()
      try:
         self.cs = self.db.create_collection_space(cs_name)
         self.cl = self.cs.create_collection(cl_name)
         print( 'create cl success' )
      except SDBBaseError as e:
         if(-34 != e.code):
            print(e.detail)
            raise e

   def insert_datas(self):
      doc = []
      for i in range(0, insert_nums):
         doc.append({"_id": i, "a": "test" + str(i)})
      try:
         flags = 0
         self.cl.bulk_insert(flags, doc)
      except SDBBaseError as e:
         print(e.detail)
         raise e

   def get_count(self,expectCount,cond):
      try:
         if cond == None:
            actCount = self.cl.get_count()
         else:
            actCount = self.cl.get_count(condition = cond)
         self.assertEqual(expectCount,actCount)
      except SDBBaseError as e:
         self.fail('get count fail: ' + e.detail)
				
   def query_datas(self,expectResult):
      cursor_one = self.cl.query()
      actResult = []
      while True:
         try:
            rec = cursor_one.current()
            actResult.append(rec)
            cursor_one.next()
         except SDBEndOfCursor:
            # self.check_next(cursor_one)
            break
      testlib.assert_list_equal(self,expectResult, actResult)
      cond_one = None
      expect_one = insert_nums
      self.get_count(expect_one, cond_one)

      cursor_one.close()
      # self.check_cursor_close(cursor_one)

      cond_two = {"_id": {"$et": 1}}
      cursor_two = self.cl.query(condition=cond_two)
      expCount_two = 1
      self.get_count(expCount_two, cond_two)

      cond_three = {"_id": {"$gt": 90}}
      cursor_three = self.cl.query(condition=cond_three)
      expCount_three = 9
      self.get_count(expCount_three, cond_three)

      self.db.close_all_cursors()
      # self.check_cursor_close(cursor_two)
      # self.check_cursor_close(cursor_three)

   def check_next(self,cursor):
      try:
         cursor.next()
         self.fail("need next cursor fail")
      except SDBBaseError as e:
         print(e.detail)
         # self.assertEqual(e, e, 'e not equal to e')

   def check_cursor_close(self,cursor):
      try:
         cursor.current()
         self.fail("need current cursor fail")
      except SDBBaseError as e:
         print(e.detail)
         # self.assertEqual(e, e, 'e not equal to e')
