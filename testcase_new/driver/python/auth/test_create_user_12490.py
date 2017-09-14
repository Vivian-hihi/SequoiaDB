# @decription: test create user
# @testlink:   seqDB-12490
# @interface:  create_user(self,name,psw)
# @author:     liuxiaoxuan 2017-9-08

import unittest
import datetime
from pysequoiadb.error import (SDBTypeError, SDBBaseError, SDBEndOfCursor, SDBError)
from lib import testlib

USERNAME = "admin"
PASSWORD = "admin"
class TestCreateUsr12490(testlib.SdbTestBase):
   def setUp(self):
      if testlib.is_standalone():
         self.skipTest('current environment is standalone')

   def testCreateUsr12490(self):

      # create user at first time
      is_success = True
      self.check_create_user(USERNAME,PASSWORD,is_success)

      # repeat to create user
      repeat_time = 10
      for i in range(repeat_time):
         self.check_create_user(USERNAME, PASSWORD, not is_success)

   def tearDown(self):
      if self.should_clean_env():
         try:
            self.db.remove_user(USERNAME,PASSWORD)
         except SDBBaseError as e:
            # user or password not exist
            if (-300 != e.code):
               self.fail('teardown fail: ' + e.detail)

   def check_create_user(self,username,password,is_success):
      try:
         self.db.create_user(username, password)
         if not is_success:
            self.fail("NEED CREATE_USER FAIL")
      except SDBBaseError as e:
         if is_success:
            self.fail("create user fail,error: " + e.detail)
         else:
            self.assertEqual(-295,e.code,"error msg: " + e.detail)
