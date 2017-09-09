# @decription: data  opeartion
# @testlink:   seqDB-12473
# @author:     LaoJingTang 2017-8-30

from lib import testlib
from pysequoiadb.error import (SDBBaseError)
import unittest


class Data12473(testlib.TestDataOprtBase):
   def setUp(self):
      self.create_cs_cl()

   def query_update_test(self, cl_list__expect, return_list_expect, update, **kwargs):
      for i in [{"a": i, "b": i} for i in range(10)]:
         self.cl.insert(i)
      if "return_new" not in kwargs:
         kwargs["return_new"] = True

      cur = self.cl.query_and_update(update, **kwargs)
      list1 = self.get_records()
      list2 = self.get_records(cur)
      self.assert_list_equal(cl_list__expect, list1)
      self.assert_list_equal(return_list_expect, list2)
      self.cl.delete()

   @unittest.skip("result different in standalong and cluster")
   def test12473(self):
      original_list = [{"a": i, "b": i} for i in range(10)]

      update = {"$inc": {"a": 1}}

      # condition+update
      condition = {"a": {"$et": 1}}
      return_list_expect = [{"a": 2, "b": 1}]
      l = list(original_list)
      l[1] = {"a": 2, "b": 1}
      self.query_update_test(l, return_list_expect, update, condition=condition)

      # selector+update
      updated_list = [{"a": i + 1, "b": i} for i in range(10)]
      updated_list_a = [{"a": i + 1} for i in range(10)]
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

   def tearDown(self):
      self.drop_cs()
      self.close_db()
