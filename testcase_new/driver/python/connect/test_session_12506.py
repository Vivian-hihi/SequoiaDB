# @decription: test session
# @testlink:   seqDB-12506
# @interface:  set_session_attri(self,options)
# @author:     liuxiaoxuan 2017-9-08

import unittest
import datetime
from pysequoiadb.error import (SDBTypeError, SDBBaseError, SDBEndOfCursor, SDBError)
from lib import testlib

cs_name = "cs_12506"
cl_name = "cl_12506"
insert_nums = 100
class TestSession12506(unittest.TestCase):
   def setUp(self):
      testlib.print_setup_msg(self)
      self.db = testlib.default_db()
      self.create_cs_cl(cs_name,cl_name)
      self.insert_datas()

   def testSession12506(self):
      # primary
      pri_option = {'PreferedInstance': 'M'}
      self.check_session(pri_option)

      # slave
      slave_option = {'PreferedInstance': 'S'}
      self.check_session(slave_option)

   def tearDown(self):
      try:
         testlib.print_teardown_msg(self)
         self.db.drop_collection_space(cs_name)
         self.db.disconnect()
      except SDBBaseError as e:
         if (-34 != e.code):
            self.fail('teardown fail: ' + e.detail)

   def clean_cs(self,csname):
      try:
         self.db.drop_collection_space(csname)
      except SDBBaseError as e:
         pass

   def create_cs_cl(self,csname,clname):
       self.clean_cs(csname)
       try:
          self.cs = self.db.create_collection_space(csname)
          self.cls = []
          # create cl
          self.cl = self.cs.create_collection(clname)
          print('create CLs success')
       except SDBBaseError as e:
          self.fail('create cl fail: ' + e.detail)

   def insert_datas(self):
      flag = 0
      doc = []
      for i in range(0,insert_nums):
         doc.append({"a":"test" + str(i)})
      try:
         self.cl.bulk_insert(flag, doc)
      except SDBBaseError as e:
         self.fail('insert fail: ' + e.detail)

   def check_session(self,opts):
      try:
         self.db.set_session_attri(options = opts)
      except SDBBaseError as e:
         self.fail('set session fail: ' + e.detail)