from lib import testsplitbase
import unittest

class TestTask(testsplitbase.TestSplitBase):
   def setUp(self):
      self.init_db()
      l=self.get_data_group()
      self.group1_name=l[0]["GroupName"]
      self.group2_name=l[1]["GroupName"]
      cl_option = {"ShardingKey": {"a": 1}, "ShardingType": "hash", "Group": self.group1_name}
      self.create_cs_cl(cl_option=cl_option)

   def tearDown(self):
      self.drop_cs()
      self.close_db()

   @unittest.skip("find bug")
   def test_task(self):
      list=[{"a":i} for i in range(10000)]
      self.cl.split_async_by_percent(self.group1_name,self.group2_name,50.0)
      self.db.list_task()


