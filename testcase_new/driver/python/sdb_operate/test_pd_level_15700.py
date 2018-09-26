# @decription: check pd level
# @testlink:   seqDB-15700
# @interface:  set_pdlevel(self,level,options=None)
# @author:     liuxiaoxuan 2018-09-26

from lib import testlib
from pysequoiadb.error import (SDBBaseError, SDBEndOfCursor)

class TestPDLevel15700(testlib.SdbTestBase):
   def setUp(self):
      # skip standalone
      if testlib.is_standalone():
         self.skipTest('run mode is standalone')
      
   def test_pdlevel_15700(self):
      
      # set pd level of coord
      self.db.set_pdlevel(5)
     
      # get data group
      groups = testlib.get_data_groups()
      data_groupname = groups[0]['GroupName']
		
      # set pd level of node
      rg = self.db.get_replica_group_by_name(data_groupname)
      master = rg.get_master()
      master_hostname = master.get_hostname();
      master_svcname = master.get_servicename();
      self.db.set_pdlevel(3, {"HostName" : master_hostname, "svcname" : master_svcname})

      # set pd level of group
      self.db.set_pdlevel(4, {"GroupName" : data_groupname})

   def tearDown(self):
      pass