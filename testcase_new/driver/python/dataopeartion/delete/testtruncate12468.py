# -- coding: utf-8 --
"""
 @decription:
 @testlink:   seqDB-12468 :: 版本: 1 :: truncate所有记录
 @author:     laojingtang
"""
from lib import testlib

class TestTruncate12468(testlib.TestDataOprtBase):

   def setUp(self):
      self.create_cs_cl()

   def tearDown(self):
      if testlib.should_clear_env(self):
         self.drop_cs()
      self.close_db()

   def test_truncate(self):
      cl=self.cl
      cl.bulk_insert(0,[{"a":i} for i in range(10)])
      cl.truncate()
      r=self.get_records(cl.query())
      self.assertEqual(0,len(r))


