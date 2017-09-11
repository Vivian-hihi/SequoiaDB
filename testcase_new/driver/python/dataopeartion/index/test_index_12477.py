# @decription: create/drop/query id index
# @testlink:   seqDB-12477
# @interface:  create_id_index(self,options)
#              drop_id_index(self)
#              get_indexes(self,idx_name)
# @author:     liuxiaoxuan 2017-8-30

import unittest
import datetime
from pysequoiadb.error import (SDBTypeError, SDBBaseError, SDBEndOfCursor, SDBError)
from lib import testlib

cs_name = "cs_12477"
cl_name = "cl_12477"
insert_nums = 100

class TestIdIndex12477(unittest.TestCase):
   def setUp(self):
      testlib.print_setup_msg(self)
      self.db = testlib.default_db()
      self.create_cl()
      self.insert_datas()

   def testIdIndex12477(self):
      # create index without option
      option = None
      self.create_id_index(option)
      # drop index
      self.drop_id_index()
      # check index
      expect_idx_name = ''
      self.check_id_index(expect_idx_name)
      # check drop result
      is_success = True
      self.check_update_result(not is_success)
      # create index with option
      option = {'SortBufferSize': 64}
      self.create_id_index(option)
      # check index
      expect_idx_name = '$id'
      self.check_id_index(expect_idx_name)
      self.check_update_result(is_success)

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
      except SDBError as e:
         pass

   def create_cl(self):
      self.clean_cs()
      try:
         self.cs = self.db.create_collection_space(cs_name)
         self.cl = self.cs.create_collection(cl_name,{'AutoIndexId': False})
         print('create cl success')
      except SDBError as e:
         print(e.detail)
         raise e

   def insert_datas(self):
      doc = []
      for i in range(0, insert_nums):
         doc.append({"a": i, "b": "test" + str(i)})
      try:
         flags = 0
         self.cl.bulk_insert(flags, doc)
      except SDBBaseError as e:
         print(e.detail)
         raise e

   def create_id_index(self, opt):
      try:
         if opt == None:
            self.cl.create_id_index()
         else:
            self.cl.create_id_index(options = opt)
      except SDBBaseError as e:
         self.fail('create index fail: ' + e.detail)

   def drop_id_index(self):
      try:
         self.cl.drop_id_index()
      except SDBBaseError as e:
         self.fail('drop index fail: ' + e.detail)

   def check_update_result(self,is_success):
      try:
         # check update
         rule = {'$set': {'b': "update"}}
         cond = {'a': {'$gt': 10}}
         self.cl.update(rule,condition = cond)
         # update fail without id index
         if not is_success:
            self.fail('NEED UPDATE FAIL')
      except SDBBaseError as e:
         if is_success:
            self.fail('update fail error: ' + e.detail)
         else:
            self.assertEqual(-279,e.code,'update fail error: ' + e.detail)

   def check_id_index(self,expect_name):
      act_name = ''
      try:
         cursor = self.cl.get_indexes(expect_name)
         while True:
            try:
               rec = cursor.next()
               act_name = rec['IndexDef']['name']
            except SDBEndOfCursor:
               cursor.close()
               break
         self.assertEqual(expect_name,act_name,expect_name + ' not equal to ' + act_name)
      except SDBBaseError as e:
         self.fail('check id index fail: ' + e.detail)
