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

insert_nums = 100
class TestIdIndex12477(testlib.SdbTestBase):
   def setUp(self):
      self.create_cs_cl()
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
      if self.should_clean_env():
         self.drop_cs()  

   def insert_datas(self):
      doc = []
      for i in range(0, insert_nums):
         doc.append({"a": i, "b": "test" + str(i)})
      try:
         flags = 0
         self.cl.bulk_insert(flags, doc)
      except SDBBaseError as e:
         self.fail('insert fail: ' + e.detail)  

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
