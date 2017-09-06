# @decription: data  opeartion
# @testlink:   seqDB-12463
# @author:     LaoJingTang 2017-8-30
import unittest

from lib import testlib

class Data12463(testlib.TestDataOprtBase):
   def setUp(self):
      self.create_cs_cl()

   # condition+update
   def test12463(self):
      self.original_list = [{"a": 1} for i in range(10)]
      self.cl.bulk_insert(1,self.original_list)
      update = {"$inc": {"a": 1}}
      condition = {"a": {"$et": 1}}
      self.cl.update(update,condition=condition)
      l = [{"a": 2} for i in range(10)]
      self.assert_list_equal(l, self.get_records(self.cl.query()))

   def tearDown(self):
      self.drop_cs()
      self.close_db()

