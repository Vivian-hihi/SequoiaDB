# -- coding: utf-8 --
from lib import testlib
from pysequoiadb import collection

default_list = [{"a": i} for i in range(10)]

class TestQueryAndUpdate(testlib.TestDataOprtBase):
   def setUp(self):
      self.create_cs_cl()

   def tearDown(self):
      self.drop_cs()
      self.close_db()

   # 子测试
   def __sub_test(self, expect, expect_return, insert=None, **kwargs):
      if "flags" not in kwargs:
         kwargs.update({"falgs": collection.QUERY_FLG_WITH_RETURNDATA})

      cl = self.cl
      if insert == None:
         insert = default_list
      cl.bulk_insert(0, insert)

      cur = cl.query_and_remove(**kwargs)
      l = self.get_records()
      rl = self.get_records(cur)
      self.assert_list_equal(expect, l)
      self.assert_list_equal(expect_return, rl)
      cl.truncate()

   def test_query_and_update(self):
      # remove all
      e = []
      re = default_list
      self.__sub_test([], re)

      # remove + condition
      condition = {"a": 0}
      e = [{"a": i} for i in range(1, 10)]
      re = [{"a": 0}]
      self.__sub_test(e, re, condition=condition)

      condition = {"$and": [{"a": 0}, {"b": 0}]}
      list_insert = [{"a": i, "b": i} for i in range(10)]
      e = [{"a": i, "b": i} for i in range(1, 10)]
      re = [{"a": 0, "b": 0}]
      self.__sub_test(e, re, list_insert, condition=condition)

      list_insert = [{"a": {"a": 1}}, {"a": {"b": 1}}]
      condition = {"a": {"$elemMatch": {"a": 1}}}
      e = [{"a": {"b": 1}}]
      re=[{"a":{"a":1}}]
      self.__sub_test(e,re, list_insert, condition=condition)

      # remove + selector
      selector = {"b": {"$include": 0}}
      i_list = [{"a": i, "b": i} for i in range(10)]
      e = []
      re=[{"a":i} for i in range(10)]
      self.__sub_test(e,re,insert=i_list, selector=selector)

      # remove + order_by
      order_by = {"_id": -1}
      i_list = default_list
      e = []
      re=default_list
      self.__sub_test(e,re, insert=i_list, order_by=order_by)

      # remove + num_to_skip
      num_to_skip = 5
      i_list = default_list
      e=i_list[0:5]
      re=i_list[5:10]
      self.__sub_test(e,re,insert=i_list,num_to_skip=num_to_skip)

      # remove + hint
      hint = {"": "index"}
      i_list=default_list
      e=[]
      re=i_list
      self.__sub_test(e,re,insert=i_list,hint=hint)

      #remove + num_to_return
      num_to_return=5
      i_list=default_list
      e=i_list[5:10]
      re=i_list[0:5]
      self.__sub_test(e,re,insert=i_list,num_to_return=num_to_return)
