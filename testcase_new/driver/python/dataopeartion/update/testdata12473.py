# @decription: data  opeartion
# @testlink:   seqDB-12473
# @author:     LaoJingTang 2017-8-30
import unittest

from lib import testlib
from pysequoiadb.error import (SDBBaseError)


class Data12473(unittest.TestCase):
   def setUp(self):
      testlib.print_setup_msg(self)
      self.db = testlib.default_db()
      self.create_cs_cl()

   def query_update_test(self, cl_list__expect, return_list_expect, update, **kwargs):
      for i in [{"a": i, "b": i} for i in range(10)]:
         self.cl.insert(i)
      if "return_new" not in kwargs:
         kwargs["return_new"] = True

      cur = self.cl.query_and_update(update, **kwargs)
      list1 = testlib.get_records(self.cl.query())
      list2 = testlib.get_records(cur)
      testlib.assert_list_equal(self, cl_list__expect, list1)
      testlib.assert_list_equal(self, return_list_expect, list2)
      self.cl.delete()

   def test(self):
      original_list = [{"a": i, "b": i} for i in range(10)]
      updated_list = [{"a": i + 1, "b": i} for i in range(10)]
      updated_list_a = [{"a": i + 1} for i in range(10)]

      update = {"$inc": {"a": 1}}

      # condition+update
      condition = {"a": {"$et": 1}}
      return_list_expect = [{"a": 2, "b": 1}]
      l = list(original_list)
      l[1] = {"a": 2, "b": 1}
      self.query_update_test(l, return_list_expect, update, condition=condition)

      # selector+update
      selector = {"b": {"$include": 0}}
      self.query_update_test(updated_list, updated_list_a, update, selector=selector)

      # order_by+update
      order_by = {"_id": -1}
      self.query_update_test(updated_list, updated_list, update, order_by=order_by)

      # num_to_skip+update
      num_to_skip = 5
      l = list()
      for i in range(10):
         if i >= 5:
            l.append({"a": i + 1, "b": i})
         else:
            l.append({"a": i, "b": i})
      self.query_update_test(l, updated_list[num_to_skip:], update, num_to_skip=num_to_skip)

      # num_to_return+update
      num_to_return = 5
      l = list()
      for i in range(10):
         if i < 5:
            l.append({"a": i + 1, "b": i})
         else:
            l.append({"a": i, "b": i})
      self.query_update_test(l, updated_list[:num_to_return], update, num_to_return=num_to_return)

      # hint+update
      hint = {"": "index"}
      self.query_update_test(updated_list, updated_list, update, hint=hint)

      # selector+update
      selector = {"b": {"$include": 0}}
      self.query_update_test(updated_list, [{"a": i + 1} for i in range(10)], update, selector=selector)

      # return_new+update
      self.query_update_test(updated_list, original_list, update, return_new=False)

      # flags+update
      QUERY_FLG_FORCE_HINT = 128
      try:
         self.query_update_test(updated_list, updated_list, update, flagss=QUERY_FLG_FORCE_HINT, hint=hint)
      except SDBBaseError as e:
         pass

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
