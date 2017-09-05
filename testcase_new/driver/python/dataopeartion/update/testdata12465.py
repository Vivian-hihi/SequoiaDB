# @decription: data  opeartion
# @testlink:   seqDB-12465
# @author:     LaoJingTang 2017-8-30
import unittest

from lib import testlib

NUM = 10


class Data12465(unittest.TestCase):
   def setUp(self):
      testlib.print_setup_msg(self)
      self.db = testlib.default_db()
      self.create_cs_cl()

   def subtest(self, cl_list__expect, return_list_expect, upsert, **kwargs):
      for i in self.original_list:
         self.cl.insert(i)
      if "return_new" not in kwargs:
         kwargs["return_new"] = True

      cur = self.cl.upsert(upsert, **kwargs)
      list1 = testlib.get_records(self.cl.query())
      list2 = testlib.get_records(cur)
      testlib.assert_list_equal(cl_list__expect,list1)
      testlib.assert_list_equal(return_list_expect,list2)
      self.cl.delete()

   def test(self):
      self.original_list = [{"a": 1} for i in range(NUM)]
      original_list = self.original_list

      upsert = {"$inc": {"a": 1}}

      # condition+upsert
      condition = {"a": {"$et": 1}}
      l = [{"a": 2} for i in range(NUM)]
      self.subtest(l, l, upsert, condition=condition)

      self.db.drop_collection_space(self.cs_name)

   def tearDown(self):
      testlib.print_teardown_msg(self)
      self.db.disconnect()

   def create_cs_cl(self):
      self.cs_name = self.__class__.__name__ + "_cs"
      self.cl_name = self.__class__.__name__ + "_cl"
      try:
         self.db.drop_collection_space(self.cs_name)
      except BaseException as e:
         pass
      self.cs = self.db.create_collection_space(self.cs_name)
      self.cl = self.cs.create_collection(self.cl_name)
