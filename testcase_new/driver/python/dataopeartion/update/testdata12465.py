# @decription: data  opeartion
# @testlink:   seqDB-12465
# @author:     LaoJingTang 2017-8-30
import unittest

from lib import testlib

NUM = 10


class Data12465Sdb(testlib.SdbTestBase):
   def setUp(self):
      self.create_cs_cl()

   def upsert_test(self, cl_list__expect, upsert, **kwargs):
      for i in self.original_list:
         self.cl.insert(i)
      if "return_new" not in kwargs:
         kwargs["return_new"] = True

      self.cl.upsert(upsert, **kwargs)
      list1 = testlib.get_all_records_noid(self.cl.query())
      self.assertListEqualUnordered(cl_list__expect, list1)
      self.cl.delete()

   def test12465(self):
      self.original_list = [{"a": 1} for i in range(NUM)]
      original_list = self.original_list

      upsert = {"$inc": {"a": 1}}

      # condition+upsert
      condition = {"a": {"$et": 1}}
      l = [{"a": 2} for i in range(NUM)]
      self.upsert_test(l, upsert, condition=condition)

   def tearDown(self):
      if self.should_clean_env():
         self.drop_cs()
