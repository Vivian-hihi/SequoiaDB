# @decription: test list
# @testlink:   seqDB-12504
# @interface:  get_list(self,list_type,kwargs)
# @author:     liuxiaoxuan 2017-9-08

import unittest
import datetime
from pysequoiadb.error import (SDBTypeError, SDBBaseError, SDBEndOfCursor, SDBError)
from lib import testlib

cs_name = "cs_12504"
cl_name = "cl_12504"
cl_num = 5
insert_nums = 100
class TestList12504(unittest.TestCase):
   def setUp(self):
      testlib.print_setup_msg(self)
      self.db = testlib.default_db()
      self.create_cl()
      self.insert_datas()

   def testList12504(self):

      cl_full_name = cs_name + "." + cl_name + "_1"
      condition = [{"Name": cl_full_name} , None]

      # check list with option
      expectResult1 = [cl_full_name]
      self.get_list_4(expectResult1, condition[0])

      # check list without option
      expectResult2 = []
      for i in range(1, cl_num + 1):
         expectResult2.append(cs_name + "." + cl_name  + "_" + str(i))
      self.get_list_4(expectResult2, condition[1])

   def tearDown(self):
      try:
         testlib.print_teardown_msg(self)
         self.db.drop_collection_space(cs_name)
         self.db.disconnect()
      except SDBBaseError as e:
         if (-34 != e.code):
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
          self.cls = []
          # create CLs
          for i in range(1, cl_num + 1):
             self.cl = self.cs.create_collection(cl_name + "_" + str(i))
             self.cls.append(self.cl)
          print('create CLs success')
       except SDBBaseError as e:
          print(e.detail)
          raise e

   def insert_datas(self):
      flag = 0
      doc = []
      for i in range(0,insert_nums):
         doc.append({"a":"test" + str(i)})
      try:
         for i in range(0, cl_num):
            self.cls[i].bulk_insert(flag,doc)
      except SDBBaseError as e:
         print(e.detail)
         raise e

   def get_list_4(self,expectResult,cond):
       try:
          list_type_4 = 4
          if(cond == None):
              cursor = self.db.get_list(list_type_4)
          else:
             cursor = self.db.get_list(list_type_4, condition = cond)

          actResult = self.get_act_result(cursor)
          self.check_list_result(expectResult,actResult)
       except SDBBaseError as e:
          self.fail('get list fail: ' + e.detail)

   def get_act_result(self,cursor):
      actResult = []
      while True:
         try:
            rec = cursor.next()
            actResult.append(rec['Name'])
         except SDBEndOfCursor:
            cursor.close()
            break
      return actResult

   def check_list_result(self,expect,act):
      for x in expect:
         self.assertIn(x, act, x + " not in " + str(act))