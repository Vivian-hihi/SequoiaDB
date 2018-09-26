# @decription reelect
# @testlink   seqDB-15696
# @interface  reelect(self, options=None)
# @author     liuxiaoxuan 2018-09-26

import unittest
from lib import testlib
from lib import sdbconfig
from clustermanager.commlib import *
from pysequoiadb.error import (SDBBaseError, SDBEndOfCursor)

class TestReelect15696(testlib.SdbTestBase):
   def setUp(self):
      # skip standalone
      if testlib.is_standalone():
         self.skipTest('run mode is standalone')
      self.data_rg_name = "data15696"	   
      
   def test_reelect_15696(self):
      # create data rg
      data_rg = self.db.create_replica_group(self.data_rg_name)
      
      #create node 1
      data_hostname = self.db.get_replica_group_by_name("SYSCatalogGroup").get_master().get_hostname()
      service_name1 = str(sdbconfig.sdb_config.rsrv_port_begin)
      data_dbpath1 = sdbconfig.sdb_config.rsrv_node_dir + service_name1
      data_rg.create_node(data_hostname, service_name1, data_dbpath1)

      #create node 2
      service_name2 = str(sdbconfig.sdb_config.rsrv_port_begin + 10)
      data_dbpath2 = sdbconfig.sdb_config.rsrv_node_dir + service_name2
      data_rg.create_node(data_hostname, service_name2, data_dbpath2)
      
      #start rg       
      data_rg.start()
     
      # check master node 
      check_rg_master(data_rg)
		
		# reelect with none options
      data_rg.reelect()
		
		# check master node 
      check_rg_master(data_rg)
		
		# reelect with options
      data_rg.reelect({"Seconds" : 120})
		
		# check master node 
      check_rg_master(data_rg)  

   def tearDown(self):
      try:
         self.db.remove_replica_group(self.data_rg_name)
         self.db.disconnect()
      except SDBBaseError as e:
         if -154 != e.code:
            self.fail("tear_down_fail: " + e.detail)