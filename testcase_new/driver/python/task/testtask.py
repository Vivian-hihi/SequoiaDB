# -- coding: utf-8 --
"""
 @decription:
 @testlink:   测试用例 seqDB-12495 :: 版本: 1 :: 列出/取消/等待任务
 @author:     laojingtang
"""

from lib import testlib

class TestTask12495(testlib.SdbTestBase):
   def setUp(self):
      if testlib.is_standalone():
         self.skipTest("skip! This testcase do not support standlone")

      if testlib.get_data_groups().__len__() < 2:
         self.skipTest("only have signal group")

      l = testlib.get_data_groups()
      self.g1 = l[0]
      self.g2 = l[1]
      self.g1_name = self.g1["GroupName"]
      self.g2_name = self.g2["GroupName"]

      cl_option = {"ShardingKey": {"a": 1}, "ShardingType": "hash", "Group": self.g1_name}
      testlib.drop_cs(self.db, self.cs_name, ignore_not_exist=True)
      self.cs = self.db.create_collection_space(self.cs_name)
      self.cl = self.cs.create_collection(self.cl_name,cl_option)

   def tearDown(self):
      if self.should_clean_env():
         self.db.drop_collection_space(self.cs_name)

   def test_task(self):
      list=[{"a":i} for i in range(10000)]
      self.cl.bulk_insert(0,list)
      self.cl.split_async_by_percent(self.g1_name,self.g2_name,50.0)
      cur=self.db.list_tasks()
      list=testlib.get_all_records(cur)
      task_id=list[0]["TaskID"]
      if task_id is not None:
         self.db.cancel_task(task_id=task_id,is_async=True)
         self.db.wait_task([task_id],1)
