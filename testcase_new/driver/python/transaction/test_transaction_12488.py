# @decription: test rollback transation
# @testlink:   seqDB-12488
# @interface:  transaction_begin(self)
#              transaction_rollback(self)
# @author:     liuxiaoxuan 2017-9-09

import unittest
import datetime
from pysequoiadb.error import (SDBTypeError, SDBBaseError, SDBEndOfCursor, SDBError)
from lib import testlib

cs_name = "cs_12488"
cl_name = "cl_12488"
insert_nums = 100

class TestTransaction12488(unittest.TestCase):
    def setUp(self):
        testlib.print_setup_msg(self)
        self.db = testlib.default_db()
        self.clean_cs(cs_name)

    def testTransaction12488(self):
        # begin to do transaction
        self.begin_transaction()
        # do create cl
        self.create_cs_cl(cs_name,cl_name)
        # insert
        self.insert_datas()

        # update
        rule = {'$set': {'b': 'update'}}
        condition = {'a': {'$gt': 11, '$lt': 20}}
        self.update_datas(rule,condition)

        # remove
        condition = {'a': {'$gte': 20}}
        self.remove_datas(condition)

        # check data before rollback transaction
        condition = {'a': {'$gt': 11}}
        expectResult = []
        for i in range(12, 20):
            expectResult.append({"a": i, "b": "update"})
        self.check_result(condition, expectResult)

        # do rollback
        self.rollback_transaction()

        # check rollback result
        condition = None
        expectResult = []
        self.check_result(condition, expectResult)

    def tearDown(self):
       try:
          testlib.print_teardown_msg(self)
          self.db.drop_collection_space(cs_name)
          self.db.disconnect()
       except SDBBaseError as e:
          if (-34 != e.code):
             self.fail('tearDown fail: ' + e.detail)

    def clean_cs(self,csname):
       try:
          self.db.drop_collection_space(csname)
       except SDBBaseError as e:
          pass

    def begin_transaction(self):
       try:
          self.db.transaction_begin()
       except SDBBaseError as e:
          self.fail('begin transaction fail: ' + e.detail)

    def create_cs_cl(self,csname,clname):
       try:
          self.cs = self.db.create_collection_space(csname)
          self.cl = self.cs.create_collection(clname)
          print('create cl success')
       except SDBBaseError as e:
          self.fail('create cl fail: ' + e.detail)

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

    def rollback_transaction(self):
        try:
            self.db.transaction_rollback()
        except SDBBaseError as e:
            self.fail('rollback transaction fail: ' + e.detail)

    def check_result(self,cond,expectRec):
       try:
          if cond == None:
              cursor = self.cl.query(order_by = {"_id": 1})
          else:
              cursor = self.cl.query(condition = cond, order_by = {"_id": 1})

          actRec = self.get_act_result(cursor)
          # check result
          testlib.assert_list_equal(self, expectRec, actRec)
       except SDBBaseError as e:
          self.fail('check result fail: ' + e.detail)

    def get_act_result(self, cursor):
       actResult = []
       while True:
          try:
             rec = cursor.next()
             rec.pop('_id')
             actResult.append(rec)
          except SDBEndOfCursor:
             cursor.close()
             break
       return actResult
