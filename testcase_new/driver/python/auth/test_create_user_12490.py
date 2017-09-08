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
class TestCreateUsr12490(unittest.TestCase):
   def setUp(self):
      testlib.print_setup_msg(self)
      self.db = testlib.default_db()
      if (self.is_stand_alone()):
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
      try:
         testlib.print_teardown_msg(self)
         self.db.remove_user(USERNAME,PASSWORD)
         self.db.disconnect()
      except SDBBaseError as e:
         # user or password not exist
         if (-300 != e.code):
            print(e.detail)
            raise e

   def is_stand_alone(self):
      try:
         cursor = self.db.list_replica_groups()
      except SDBBaseError as e:
         if(-159 == e.code):
            return True
      return False

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
