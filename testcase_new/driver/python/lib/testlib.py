import unittest
from datetime import datetime

from lib import sdbconfig
from pysequoiadb import client

config = sdbconfig.SdbConfig()


def default_db():
   return client(config.host_name, config.service)


def print_setup_msg(self):
   print(str(self.__class__.__name__) + " setup: " + str(datetime.now()))


def print_teardown_msg(self):
   print(str(self.__class__.__name__) + " teardown: " + str(datetime.now()))


def assert_list_equal(expected, actual):
   """
   判断两个数组是否相等，并不要求两个数组具有相同的顺序
   :param expected: list expected
   :param actual: list actual
   """
   msg = "\nexpected: " + str(expected) + "\nactual: " + str(actual)

   unittest.TestCase.assertSequenceEqual(len(expected), len(actual), msg=msg)

   for x in actual:
      unittest.TestCase.assertIn(x, expected, msg)

   for x in expected:
      unittest.TestCase.assertIn(x, actual, msg)

def get_records(cur):
   """

   :param cur: 游标
   :return: 记录数组
   """
   items = list()
   while True:
      try:
         item = cur.next()
         item.pop('_id')
         items.append(item)
      except BaseException as e:
         break
   cur.close()
   return items
