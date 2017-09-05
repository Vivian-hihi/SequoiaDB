# @decription: data  opeartion
# @testlink:   seqDB-12464
# @author:     LaoJingTang 2017-8-30
import unittest

from lib import testlib
from pysequoiadb.error import (SDBBaseError)


class Data12464(unittest.TestCase):
   def setUp(self):
      testlib.print_setup_msg(self)
      self.db = testlib.default_db()
      self.create_cs_cl()

   def update_test(self, list__expect, update, **kwargs):
      for i in self.original_list:
         self.cl.insert(i)
      self.cl.update(update, **kwargs)
      list1 = testlib.get_records(self.cl.query())
      testlib.assert_list_equal(self,list__expect,list1)
      self.cl.delete()

   def test(self):
      self.original_list = ({"a": 0, "b": 0}, {"a": 1, "b": 1}, {"a": 2, "b": 2})
      original_list = self.original_list
      update = {"$inc": {"a": 1}}

      # condition+update
      condition = {"a": {"$et": 0}}
      update_list = ({"a": 1, "b": 0}, {"a": 1, "b": 1}, {"a": 2, "b": 2})
      self.update_test(update_list, update, condition=condition)

      # hint+update
      hint = {"": "index"}
      l = ({"a": 1, "b": 0}, {"a": 2, "b": 1}, {"a": 3, "b": 2})
      self.update_test(l, update, hint=hint)

      # flags+update
      QUERY_FLG_FORCE_HINT = 128
      try:
         self.update_test(l, update, flagss=QUERY_FLG_FORCE_HINT, hint=hint)
      except SDBBaseError as e:
         pass

   def tearDown(self):
      testlib.print_teardown_msg(self)
      self.db.drop_collection_space(self.cs_name)
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
