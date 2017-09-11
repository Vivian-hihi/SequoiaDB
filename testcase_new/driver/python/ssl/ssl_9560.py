# @decription: test ssl,open ssl=true
# @testlink:   seqDB-9560
# @author:     liuxiaoxuan 2017-9-09

import unittest
import datetime
from pysequoiadb.error import (SDBTypeError, SDBBaseError, SDBEndOfCursor, SDBError)
from pysequoiadb import client
from lib import sdbconfig
from lib import testlib

cs_name = "cs_9560"
cl_name = "cl_9560"
class TestSSL9560(unittest.TestCase):
    def setUp(self):
        testlib.print_setup_msg(self)
        self.config = sdbconfig.SdbConfig()
        self.db = client(self.config.host_name, self.config.service, '', '', False)
        self.clean_cs()

    def testSSL9560(self):
       # ssl = false
       self.check_create_cl(self.db)
       # ssl = true
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

    def check_create_cl(self,db):
       try:
          self.cs = db.create_collection_space(cs_name)
          self.cl = self.cs.create_collection(cl_name)
          print('create cl success')
       except SDBBaseError as e:
          self.fail('create cl fail: ' + e.detail)

    def check_with_ssl(self):
       new_db = None
       try:
          new_db = client(self.config.host_name, self.config.service, '', '', True)
          new_db.drop_collection_space(cs_name)
          self.check_create_cl(new_db)
       except SDBBaseError as e:
          self.fail('ssl fail: ' + e.detail)
       finally:
          if not new_db == None:
             new_db.disconnect()
