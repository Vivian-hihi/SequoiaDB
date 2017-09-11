# -- coding: utf-8 --
import unittest
from datetime import datetime

from lib import sdbconfig
from pysequoiadb import SDBError
from pysequoiadb import client


class TestDataOprtBase(unittest.TestCase):
   def __init__(self, methodName='runTest'):
      unittest.TestCase.__init__(self, methodName=methodName)
      self.cs_name = self.__class__.__name__ + "_cs"
      self.cl_name = self.__class__.__name__ + "_cl"
      self.cl_name_qualified = self.cs_name + "." + self.cl_name
      self.db_list = []

   @classmethod
   def setUpClass(cls):
      print(cls.__name__ + " setup: " + str(datetime.now()))

   # _my_result=None
   # def run(self, result=None):
   #    r=unittest.TestCase.run(self,result)
   #    self.__class__._my_result=r

   @classmethod
   def tearDownClass(cls):
      print(cls.__name__ + " teardown: " + str(datetime.now()))

   def init_db(self):
      if not hasattr(self, "db"):
         self.db = default_db()

   def drop_cs(self):
      try:
         self.db.drop_collection_space(self.cs_name)
      except SDBError as e:
         pass

   def create_cs_cl(self, cs_option=0, cl_option=None):
      if not hasattr(self, "db"):
         self.db = default_db()
      try:
         self.db.drop_collection_space(self.cs_name)
      except BaseException as e:
         pass
      self.cs = self.db.create_collection_space(self.cs_name, options=cs_option)
      self.cl = self.cs.create_collection(self.cl_name, options=cl_option)

   def assert_list_equal(self, expected, actual):
      assert_list_equal(self, expected, actual)

   def get_records(self, cur=None):
      if cur == None:
         if not hasattr(self, "cl"):
            self.cl = self.db.get_collection(self.cl_name_qualified)
         cur = self.cl.query()
      return get_records(cur)

   def close_db(self):
      for db in self.db_list:
         db.disconnect()
      if hasattr(self, "db"):
         self.db.disconnect()


def default_db():
   return client(sdbconfig.sdb_config.host_name, sdbconfig.sdb_config.service)


def print_setup_msg(self):
   print(str(self.__class__.__name__) + " setup: " + str(datetime.now()))


def print_teardown_msg(self):
   print(str(self.__class__.__name__) + " teardown: " + str(datetime.now()))


def assert_list_equal(self, expected, actual):
   """
   判断两个数组是否相等，并不要求两个数组具有相同的顺序
   :param expected: list expected
   :param actual: list actual
   """
   msg = "\nexpected: " + str(expected) + "\nactual: " + str(actual)

   unittest.TestCase.assertEqual(self, len(expected), len(actual), msg=msg)

   for x in actual:
      unittest.TestCase.assertIn(self, x, expected, msg)

   for x in expected:
      unittest.TestCase.assertIn(self, x, actual, msg)


def get_records(cur):
   """
   :param cur: 游标
   :return: 记录数组
   """
   items = list()
   while True:
      try:
         item = cur.next()
         if "_id" in item:
            item.pop('_id')
         items.append(item)
      except BaseException as e:
         break
   cur.close()
   return items

def should_clear_env(self):
   return True

