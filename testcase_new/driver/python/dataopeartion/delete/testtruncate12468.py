# -- coding: utf-8 --
"""
 @decription:
 @testlink:   seqDB-12468 :: 版本: 1 :: truncate所有记录
 @author:     laojingtang
"""
from lib import testlib

class SdbTestTruncate12468(testlib.SdbTestBase):

   def setUp(self):
      self.create_cs_cl()

   def tearDown(self):
      if self.should_clean_env():
         self.drop_cs()

   def test_truncate(self):
      cl=self.cl
      cl.bulk_insert(0,[{"a":i} for i in range(10)])
      cl.truncate()
      r=testlib.get_all_records_noid(cl.query())
      self.assertEqual(0,len(r))


