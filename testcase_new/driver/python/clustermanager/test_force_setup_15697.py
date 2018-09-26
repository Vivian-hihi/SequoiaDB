# @decription force setup
# @testlink   seqDB-15697
# @interface  force_stepup (self, options=None)
# @author     liuxiaoxuan 2018-09-26

import unittest
from lib import testlib
from lib import sdbconfig
from clustermanager.commlib import *
from pysequoiadb.error import (SDBBaseError, SDBEndOfCursor)

class TestForceSetup15697(testlib.SdbTestBase):
   def setUp(self):
      # skip standalone
      if testlib.is_standalone():
         self.skipTest('run mode is standalone')

   def test_force_setup_15697(self) :
      # get catalog group
      cata_rg = self.db.get_replica_group_by_name("SYSCatalogGroup")
         
      # check master node 
      check_rg_master(cata_rg)
		
      # force setup with none options, should fail
      slave_node = cata_rg.get_slave().connect()
      try:
         slave_node.force_stepup()
      except SDBBaseError as e:
         self.assertEqual(-280, e.code, "force setup no options: " + str(e.code))
		
      # force setup with options, should fail
      try:
         slave_node.force_stepup({ "Seconds" : 300 } )
      except SDBBaseError as e:
         self.assertEqual(-280, e.code, "force setup no options: " + str(e.code)) 
		
   def tearDown(self):
      pass