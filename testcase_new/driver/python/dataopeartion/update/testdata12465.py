# @decription: data  opeartion
# @testlink:   seqDB-12465
# @author:     LaoJingTang 2017-8-30
import unittest

from lib import testlib

NUM = 10


class Data12465Sdb(testlib.SdbTestBase):
   def setUp(self):
      self.create_cs_cl()

   def upsert_test(self, record_to_insert,cl_list__expect, upsert_rule, **kwargs):
      self.cl.bulk_insert(0,record_to_insert)

      self.cl.upsert(upsert_rule, **kwargs)
      list1 = testlib.get_all_records_noid(self.cl.query())
      self.assertListEqualUnordered(cl_list__expect, list1)
      self.cl.delete()

   def test12465(self):
      record_to_insert = [{"a": 1} for i in range(NUM)]

      upsert_rule= {"$inc": {"a": 1}}

      # condition+upsert
      condition = {"a": {"$et": 1}}
      expect = [{"a": 2} for i in range(NUM)]
      self.upsert_test(record_to_insert,expect, upsert_rule, condition=condition)

   def tearDown(self):
      if self.should_clean_env():
         self.drop_cs()
