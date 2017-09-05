# @decription: data  opeartion
# @testlink:   seqDB-12463
# @author:     LaoJingTang 2017-8-30
import unittest
from datetime import datetime

from lib import testlib

class Data12463(unittest.TestCase):
   def setUp(self):
      testlib.print_setup_msg(self)
      self.db=testlib.default_db()
      self.create_cs_cl()

   # condition+update
   def test(self):
      self.original_list = [{"a": 1} for i in range(10)]
      self.cl.bulk_insert(1,self.original_list)
      update = {"$inc": {"a": 1}}
      condition = {"a": {"$et": 1}}
      self.cl.update(update,condition=condition)
      l = [{"a": 2} for i in range(10)]
      testlib.assert_list_equal(self,l, testlib.get_records(self.cl.query()))

   def tearDown(self):
      print("end: " + str(datetime.now()))
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

