# @decription: test session
# @testlink:   seqDB-12506
# @interface:  set_session_attri(self,options)
# @author:     liuxiaoxuan 2017-9-08

import unittest
import datetime
from pysequoiadb.error import (SDBTypeError, SDBBaseError, SDBEndOfCursor, SDBError)
from lib import testlib

insert_nums = 100
class TestSession12506(testlib.SdbTestBase):
   def setUp(self):
      self.create_cs_cl()
      self.insert_datas()

   def testSession12506(self):
      # primary
      pri_option = {'PreferedInstance': 'M'}
      self.check_session(pri_option)

      # slave
      slave_option = {'PreferedInstance': 'S'}
      self.check_session(slave_option)

   def tearDown(self):
      if self.should_clean_env():
         self.drop_cs()

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