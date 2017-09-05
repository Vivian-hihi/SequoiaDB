# @decription: data  opeartion
# @testlink:   seqDB-12463
# @author:     LaoJingTang 2017-8-30
import unittest
from datetime import datetime

from dataopeartion.update import util
from lib import testlib

NUM = 10
class Data12463(unittest.TestCase):
   def setUp(self):
      testlib.print_setup_msg(self)
      self.db=testlib.default_db()
      self.create_cs_cl()

   def subtest(self, cl_list__expect, return_list_expect, update, **kwargs):
      for i in self.original_list:
         self.cl.insert(i)
      if "return_new" not in kwargs:
         kwargs["return_new"] = True

      cur = self.cl.update(update, **kwargs)
      list1 = self.get_result()
      list2 = self.get_result(cur)

      self.check_result(list1, cl_list__expect)
      self.check_result(list2, return_list_expect)
      self.cl.query_and_remove()

   def test(self):
      self.original_list = [{"a": 1} for i in range(NUM)]
      original_list = self.original_list

      update = {"$inc": {"a": 1}}

      # condition+update
      condition = {"a": {"$et": 1}}
      l = [{"a": 2} for i in range(NUM)]
      self.subtest(l, l, update, condition=condition)

      self.db.drop_collection_space(self.cs_name)

   def check_result(self, list1, expect_list):
      if not util.check_result(list1, expect_list):
         self.fail("check result fail")

   def get_result(self, cur=None):
      if cur == None:
         cur = self.cl.query()
      items = list()
      while True:
         try:
            item = cur.next()
            item.pop('_id')
            items.append(item)
         except BaseException as e:
            break
      return items

   def tearDown(self):
      print("end: " + str(datetime.now()))
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

