# @decription: test store procedure
# @testlink:   seqDB-12491/seqDB-12492
# @interface:  create_procedure(self,code)
#              list_procedures(self,condition)
#              eval_procedure(self,name)
#              remove_procedures(self,name)
# @author:     liuxiaoxuan 2017-9-08

import unittest
import datetime
from pysequoiadb.error import (SDBTypeError, SDBBaseError, SDBEndOfCursor, SDBError)
from bson.code import Code
from lib import testlib


class TestProcedure12491(unittest.TestCase):
   def setUp(self):
      testlib.print_setup_msg(self)
      self.db = testlib.default_db()
      if (self.is_stand_alone()):
         self.skipTest('current environment is standalone')

   def testProcedure12491(self):
      # check create result
      code = 'function sum(x,y) { return x + y; }'
      self.check_create_procedure(code)

      # check list result
      condition = {'name': 'sum'}
      expectResult = {'name': 'sum','code': Code(code)}
      self.check_list_procedure(condition,expectResult)

      # check eval
      name = 'sum(1,2)'
      expectResult = 3
      self.check_eval_procedure(name,expectResult)

      # check remove result
      name = 'sum'
      self.check_remove_procedure(name)

      # check remove not exist procedure(seqDB-12492)
      names = ["sum", "find", "remove", "update", "insert"]
      self.check_remove_none_procedure(names)

   def tearDown(self):
      try:
         testlib.print_teardown_msg(self)
         self.db.disconnect()
      except SDBBaseError as e:
          print(e.detail)
          raise e

   def is_stand_alone(self):
      try:
         cursor = self.db.list_replica_groups()
      except SDBBaseError as e:
         if(-159 == e.code):
            return True
      return False

   def check_create_procedure(self,code):
      try:
         self.db.create_procedure(code)
      except SDBBaseError as e:
         self.fail("create procedure fail: " + e.detail)

   def check_list_procedure(self,cond,expectResult):
      try:
         cursor = self.db.list_procedures(condition = cond)
         actResult = cursor.next()
         self.assertEqual(expectResult['name'],actResult['name'])
         self.assertEqual(expectResult['code'],actResult['func'])
      except SDBBaseError as e:
         self.fail("list procedure fail: " + e.detail)

   def check_eval_procedure(self,name,expectResult):
      try:
         cursor = self.db.eval_procedure(name)
         actResult = cursor.next()
         self.assertEqual(expectResult, actResult['value'])
      except SDBBaseError as e:
         self.fail("eval procedure fail: " + e.detail)

   def check_remove_procedure(self,name):
      try:
         self.db.remove_procedure(name)
         # check result
         result = 0
         cursor = self.db.list_procedures(condition = {'name': name})
         try:
             cursor.next()
             result = result + 1
         except SDBEndOfCursor:
             self.assertEqual(0, result, 'remove procedure fail')
      except SDBBaseError as e:
         self.fail("remove procedure fail: " + e.detail)

   def check_remove_none_procedure(self,names):
      for i in range(len(names)):
         try:
            self.db.remove_procedure(names[i])
            self.fail("NEED REMOVE PROCEDURE FAIL")
         except SDBBaseError as e:
            # procedure not exist
            self.assertEqual(-233, e.code, "remove procedure(" + names[i] + ") fail, msg: " + e.detail)
