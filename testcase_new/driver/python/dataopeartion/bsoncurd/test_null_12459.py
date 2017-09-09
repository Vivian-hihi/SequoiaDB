# @decription: insert null data
# @testlink:   seqDB-12459
# @interface:  insert(record)
#              update(rule, kwargs)
#              delete(kwargs)
# @author:     zhaoyu 2017-9-6

import unittest
from pysequoiadb.error import (SDBBaseError)
from dataopeartion.bsoncurd.commlib import *
from lib import testlib
from lib import sdbconfig
from bson.json_util import loads
from bson.json_util import dumps


class TestNull12459(unittest.TestCase):
   def setUp(self):
      testlib.print_setup_msg(self)
      self.db = testlib.default_db()
      
   def test_null_12459(self):
      # create cs and cl
      self.cs_name = "cs_12459"
      self.cl_name = "cl_12459"
      try:
         self.db.drop_collection_space(self.cs_name)
      except SDBBaseError as e:
         if (-34 != e.code):
            print(e.detail)
            self.fail("drop_cs_fail")
      self.cs = self.db.create_collection_space(self.cs_name)
      self.cl = self.cs.create_collection(self.cl_name)

      # insert data
      data1 = None
      data2 = 'null'
      record = [{"a": data1}, {"a": loads(data2)}]
      self.cl.bulk_insert(0, record)

      # query data and check
      expect_type = [{"a": "null"}, {"a": "null"}]
      check_Result(self.cl, {}, {"a": {"$type": 2}}, record, expect_type, False)

      # update data
      data1_after_update = 1
      data2_after_update = 0
      self.cl.update({"$set": {"a": data1_after_update}}, condition={"a": data1})
      self.cl.update({"$set": {"a": data2_after_update}}, condition={"a": loads(data2)})

      # query data and check
      expect_type_after_update = [{"a": "int32"}, {"a": "int32"}]
      record_after_update = [{"a": data1_after_update}, {"a": data2_after_update}]
      check_Result(self.cl, {}, {"a": {"$type": 2}}, record_after_update, expect_type_after_update, False)

      # update data
      self.cl.update({"$set": {"a": loads(data2)}}, condition={"a": data2_after_update})
      self.cl.update({"$set": {"a": data1}}, condition={"a": data1_after_update})

      # query data and check
      check_Result(self.cl, {}, {"a": {"$type": 2}}, record, expect_type, False)

      # delete data
      self.cl.delete()

      # query data and check
      check_Result(self.cl, {}, {}, {}, {}, False)

      # insert data
      self.cl.bulk_insert(0, record)

      # query data and check
      check_Result(self.cl, {}, {"a": {"$type": 2}}, record, expect_type, False)

      # delete data
      self.cl.delete(condition={"a": data1})
      self.cl.delete(condition={"a": {"$et": loads(data2)}})

      # query data and check
      check_Result(self.cl, {}, {}, {}, {}, False)

      # json to bson
      json = '{"a": null}'
      self.assertEqual(json, dumps(loads(json)))
      
   def tearDown(self):
      try:
         self.db.drop_collection_space(self.cs_name)
         self.db.disconnect()
      except SDBBaseError as e:
         if (-34 != e.code):
            print(e.detail)
            self.fail("tear_down_fail")
      testlib.print_teardown_msg(self)