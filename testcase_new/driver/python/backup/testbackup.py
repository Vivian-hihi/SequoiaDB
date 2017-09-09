# -- coding: utf-8 --
# @decription: test backup
# @testlink:  测试用例 seqDB-12494 :: 版本: 1 :: 备份/列出备份/删除备份全部数据库信息
#             测试用例 seqDB-12493 :: 版本: 1 :: 备份/列出备份/删除备份指定组的数据库
# @author:     LaoJingTang 2017-8-30
from lib import testlib
class TestBackup(testlib.TestDataOprtBase):
   def setUp(self):
      self.init_db()

   def tearDown(self):
      self.close_db()

   def test_backup(self):
      self.db.remove_backup({"Name": "mybk"})
      self.db.backup_offline({"Name": "mybk"})
      cur = self.db.list_backup({})
      l = self.get_records(cur)
      names = []
      for x in l:
         names.append(x["Name"])
      self.assertIn("mybk", names)
      self.db.remove_backup({"Name": "mybk"})
