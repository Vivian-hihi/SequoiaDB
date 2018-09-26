# @decription check reload config
# @testlink   seqDB-15701
# @interface  reload_config(self,options=None)
# @author     liuxiaoxuan 2018-09-26

from lib import testlib
from pysequoiadb.error import (SDBBaseError, SDBEndOfCursor)

class TestReloadConfig15701(testlib.SdbTestBase):
   def setUp(self):
      # skip standalone
      if testlib.is_standalone():
         self.skipTest('run mode is standalone')
      
   def test_reload_config_15701(self):
      # get data group
      groups = testlib.get_data_groups()
      data_groupname = groups[0]['GroupName']
		
		# reload config of all
      self.db.reload_config()
		
      # reload conf of node
      rg = self.db.get_replica_group_by_name(data_groupname)
      master = rg.get_master()
      master_hostname = master.get_hostname();
      master_svcname = master.get_servicename();
      self.db.reload_config({"HostName" : master_hostname, "svcname" : master_svcname})

		# reload conf of group
      self.db.reload_config({"GroupName" : data_groupname})
		
   def tearDown(self):
      pass