# @decription: test ssl,not open ssl=true
# @testlink:   seqDB-9561
# @author:     liuxiaoxuan 2017-9-09

import unittest
import datetime
from pysequoiadb.error import (SDBTypeError, SDBBaseError, SDBEndOfCursor, SDBError)
from pysequoiadb import client
from lib import sdbconfig
from lib import testlib

cs_name = "cs_9561"
cl_name = "cl_9561"
class TestSSL9561(unittest.TestCase):
    def setUp(self):
        testlib.print_setup_msg(self)
        self.config = sdbconfig.SdbConfig()
        self.db = client(self.config.host_name, self.config.service, '', '', False)
        self.clean_cs()

    def testSSL9561(self):
       self.check_create_cl()
       self.check_with_ssl()

    def tearDown(self):
       try:
          testlib.print_teardown_msg(self)
          self.db.drop_collection_space(cs_name)
          self.db.disconnect()
       except SDBBaseError as e:
          if (-34 != e.code):
             print(e.detail)
             raise e

    def clean_cs(self):
       try:
          self.db.drop_collection_space(cs_name)
       except SDBBaseError as e:
          pass

    def check_create_cl(self):
       try:
          self.cs = self.db.create_collection_space(cs_name)
          self.cl = self.cs.create_collection(cl_name)
          print('create cl success')
       except SDBBaseError as e:
          self.fail('create cl fail: ' + e.detail)

    def check_with_ssl(self):
       new_db = None
       try:
          new_db = client(self.config.host_name, self.config.service, '', '', True)
          self.fail('NEED SSL FAIL')
       except SDBBaseError as e:
          self.assertTrue(True)
       finally:
          if not new_db == None:
             new_db.disconnect()
