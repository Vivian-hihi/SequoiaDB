# @decription: lob opeartion
# @testlink:   seqDB-12478
# @author:     LaoJingTang 2017-8-30

import unittest

from bson.objectid import ObjectId
from lib import testlib
from lob import util
from pysequoiadb.error import (SDBEndOfCursor)


class Lob12478(unittest.TestCase):
   def setUp(self):
      testlib.print_setup_msg(self)
      self.db = testlib.default_db()
      self.create_cs_cl()

   def test(self):
      self.lob_size = 1024
      self.lob_context = util.random_str(self.lob_size)
      self.md5 = util.get_md5(self.lob_context)

      self.create_lob(10)
      self.read_lob()
      self.del_lob()
      self.db.drop_collection_space(self.cs_name)

   def create_lob(self, num=10):
      self.oid_set = set()
      # not specify oid
      for i in range(num):
         lob = self.cl.create_lob()
         lob.write(self.lob_context, len(self.lob_context))
         oid = lob.get_oid()
         self.oid_set.add(oid)
         lob.close()

      # specify oid
      for i in range(num):
         oid = ObjectId()
         lob = self.cl.create_lob(oid)
         self.oid_set.add(oid)
         lob.write(self.lob_context, len(self.lob_context))
         lob.close()

   def read_lob(self):
      for oid in self.oid_set:
         lob = self.cl.get_lob(oid)
         context = lob.read(lob.get_size())
         self.check_lob(lob, context)

         lob.seek(0)
         context = lob.read(lob.get_size())
         self.check_lob(lob, context)

         lob.seek(lob.get_size(), 2)
         context = lob.read(lob.get_size())
         self.check_lob(lob, context)

         lob.seek(0)
         size = int(lob.get_size() / 2)
         lob.seek(size, 1)
         context = lob.read(lob.get_size() - 1)
         if isinstance(context, str):
            pass
         else:
            context = context.decode('utf-8')

         if context not in self.lob_context:
            self.fail("seek context not in lob_context")
         lob.close()

   def del_lob(self):
      for oid in self.oid_set:
         self.cl.remove_lob(oid)
      cr = self.cl.list_lobs()
      count = 0
      while True:
         try:
            cr.next()
            count += 1
         except SDBEndOfCursor:
            break
      if count != 0:
         self.fail("not remove all lob ")

   def check_lob(self, lob, context):
      if util.get_md5(context) != self.md5:
         self.fail("lobid: " + lob.get_oid() + " except md5: " + self.md5)
      if lob.get_size() != self.lob_size:
         self.fail(
            "lobid: " + str(lob.get_oid()) + " except size: " + str(self.lob_size) + "actually: " + str(lob.get_size()))

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
