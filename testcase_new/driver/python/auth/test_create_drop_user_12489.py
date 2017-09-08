# @decription: test create/remove user
# @testlink:   seqDB-12489
# @interface:  create_user(self,name,psw)
#              remove_user(self,name,psw)
# @author:     liuxiaoxuan 2017-9-08

import unittest
import datetime
from pysequoiadb.error import (SDBTypeError, SDBBaseError, SDBEndOfCursor, SDBError)
from pysequoiadb import client
from lib import sdbconfig
from lib import testlib

USERNAME = "admin"
PASSWORD = "admin"
cs_name = "cs_12489"
cl_name = "cl_12489"
class TestCreateDropUsr12489(unittest.TestCase):
   def setUp(self):
      testlib.print_setup_msg(self)
      self.db = testlib.default_db()
      if (self.is_stand_alone()):
         self.skipTest('current environment is standalone')

   def testCreateDropUsr12489(self):

      # create user
      self.check_create_user(USERNAME,PASSWORD)
      # disconnect sdb
      self.db.disconnect()
      # reconnect sdb with user
      self.check_reconnect_db(USERNAME,PASSWORD)
      # create cl
      self.create_cl()
      # check insert
      self.check_insert()
      # drop user
      self.db.remove_user(USERNAME, PASSWORD)
      # check drop result
      self.check_drop_user(USERNAME,PASSWORD)

   def tearDown(self):
      try:
         testlib.print_teardown_msg(self)
         self.db.drop_collection_space(cs_name)
         self.db.disconnect()
      except SDBBaseError as e:
         if (-34 != e.code):
            print(e.detail)
            raise e

   def is_stand_alone(self):
      try:
         cursor = self.db.list_replica_groups()
      except SDBBaseError as e:
         if(-159 == e.code):
            return True
      return False

   def check_create_user(self,username,password):
      try:
         self.db.create_user(username, password)
      except SDBBaseError as e:
         if(-295 != e.code):
            self.fail('create user fail: ' + e.detail)

   def check_create_user(self,username,password):
      try:
         self.db.create_user(username, password)
      except SDBBaseError as e:
         if(-295 != e.code):
            self.fail('create user fail: ' + e.detail)

   def check_reconnect_db(self,username,password):
       try:
           config = sdbconfig.SdbConfig()
           self.db = client(config.host_name, config.service, username, password)
       except SDBBaseError as e:
           self.fail('reconnect with username fail: ' + e.detail)

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
         print(e.detail)
         raise e

   def check_insert(self):
       flag = 0
       doc = []
       insert_nums = 100
       for i in range(0, insert_nums):
           doc.append({"a": "test" + str(i)})
       try:
           self.cl.bulk_insert(flag, doc)
       except SDBBaseError as e:
           self.fail('insert fail: ' + e.detail)

   def check_drop_user(self,username,password):
      try:
         self.db.remove_user(username, password)
         self.fail('NEED DROP USER FAIL')
      except SDBBaseError as e:
          self.assertEqual(-300, e.code, "error msg: " + e.detail)
