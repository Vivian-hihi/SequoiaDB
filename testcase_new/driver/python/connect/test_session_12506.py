# @decription: test session
# @testlink:   seqDB-12506
# @interface:  set_session_attri(self,options)
# @author:     liuxiaoxuan 2017-9-08

from lib import testlib
from pysequoiadb.error import (SDBBaseError)

insert_nums = 100


class TestSession12506(testlib.SdbTestBase):
   def setUp(self):
      testlib.drop_cs(self.db, self.cs_name, ignore_not_exist=True)
      self.cs = self.db.create_collection_space(self.cs_name)
      self.cl = self.cs.create_collection(self.cl_name)
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
         self.db.drop_collection_space(self.cs_name)

   def insert_datas(self):
      flag = 0
      doc = []
      for i in range(0, insert_nums):
         doc.append({"a": "test" + str(i)})
      try:
         self.cl.bulk_insert(flag, doc)
      except SDBBaseError as e:
         self.fail('insert fail: ' + e.detail)

   def check_session(self, opts):
      try:
         self.db.set_session_attri(options=opts)
      except SDBBaseError as e:
         self.fail('set session fail: ' + e.detail)
