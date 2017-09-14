# @decription: test commit transation
# @testlink:   seqDB-12487
# @interface:  transaction_begin(self)
#              transaction_commit(self)
# @author:     liuxiaoxuan 2017-9-09

import unittest
import datetime
from pysequoiadb.error import (SDBTypeError, SDBBaseError, SDBEndOfCursor, SDBError)
from lib import testlib

insert_nums = 100
class TestTransaction12487(testlib.SdbTestBase):
   def setUp(self):
      if testlib.is_standalone():
         self.skipTest('current environment is standalone')
      self.create_cs_cl()

   def testTransaction12487(self):
      # begin to do transaction
      self.begin_transaction()

      # insert
      self.insert_datas()
      # check insert result
      condition = None
      expectInsResult = []
      for i in range(0, insert_nums):
         expectInsResult.append({"a": i, "b": "test" + str(i)})
      self.check_result(condition,expectInsResult)

      # update
      rule = {'$set': {'b': 'update'}}
      condition = {'a': {'$gt': 11, '$lt': 20}}
      self.update_datas(rule,condition)

      # remove
      condition = {'a': {'$gte': 20}}
      self.remove_datas(condition)

      # commit transaction
      self.commit_transaction()
      # check commit result
      condition = {'a': {'$gt': 11}}
      expectResult = []
      for i in range(12, 20):
          expectResult.append({"a": i, "b": "update"})
      self.check_result(condition, expectResult)

   def tearDown(self):
      if self.should_clean_env():
         self.drop_cs()   

   def begin_transaction(self):
      try:
         self.db.transaction_begin()
      except SDBBaseError as e:
         self.fail('begin transaction fail: ' + e.detail)

   def insert_datas(self):
      doc = []
      for i in range(0, insert_nums):
         doc.append({"a": i , "b": "test" + str(i)})
      try:
         flags = 0
         self.cl.bulk_insert(flags, doc)
      except SDBBaseError as e:
         self.fail('insert fail: ' + e.detail)

   def update_datas(self,rule,cond):
      try:
         self.cl.update(rule, condition = cond)
      except SDBBaseError as e:
         self.fail('update fail: ' + e.detail)

   def remove_datas(self,cond):
      try:
         self.cl.delete(condition = cond)
      except SDBBaseError as e:
         self.fail('remove fail: ' + e.detail)

   def commit_transaction(self):
      try:
         self.db.transaction_commit()
      except SDBBaseError as e:
         self.fail('commit transaction fail: ' + e.detail)

   def check_result(self,cond,expectRec):
      try:
         if cond == None:
            cursor = self.cl.query(order_by = {"_id": 1})
         else:
            cursor = self.cl.query(condition = cond, order_by = {"_id": 1})

         actRec = testlib.get_all_records_noid(cursor)
         # check result
         self.assertListEqualUnordered(expectRec, actRec)
      except SDBBaseError as e:
         self.fail('check result fail: ' + e.detail)
