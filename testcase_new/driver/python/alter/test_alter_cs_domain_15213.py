# @decription: test alter cs domains
# @testlink:   seqDB-15213
# @interface:  set_domain(self,options)
# @interface:  remove_domain(self)
# @author:     liuxiaoxuan 2018-04-25

import unittest
from pysequoiadb.error import (SDBBaseError, SDBEndOfCursor)
from lib import testlib

class TestAlterCSDomain15213(testlib.SdbTestBase):
   def setUp(self):
      # skip standalone
      if testlib.is_standalone():
         self.skipTest('run mode is standalone')
        
      self.domain_name1 = 'testdomain15213_1'
      self.domain_name2 = 'testdomain15213_2'
         
   def test_alter_cs_domain_15213(self):
    
      # drop domain if exists
      try:
         self.db.drop_domain(self.domain_name1)
      except SDBBaseError as e:     		
         self.assertEqual(e.code, -214)
      try:
         self.db.drop_domain(self.domain_name2)
      except SDBBaseError as e:     		
         self.assertEqual(e.code, -214)   
    
      # get groups
      groups = testlib.get_data_groups()
      domain_group_names = [x['GroupName'] for x in groups]
      
      # create domains
      domain1 = self.db.create_domain(self.domain_name1, options =  { 'Groups': domain_group_names })
      domain2 = self.db.create_domain(self.domain_name2, options =  { 'Groups': domain_group_names })
      
      # create cs
      cs_name = 'testaltercsdomain15213'
      cs = self.db.create_collection_space(cs_name)
      
      # add domain
      cs.set_domain(options = {'Domain' : self.domain_name1})
      
      # check after add domain
      self.check_domain_cs(domain1, [{'Name': cs_name}])
      self.check_domain_cs(domain2, [])
      
      # set domain
      cs.set_domain(options = {'Domain' : self.domain_name2})
      
      # check after alter domain
      self.check_domain_cs(domain1, [])
      self.check_domain_cs(domain2, [{'Name': cs_name}])
      
      # set domain, remove domain
      cs.remove_domain()
      
      # check after remove domain
      self.check_domain_cs(domain1, [])
      self.check_domain_cs(domain2, [])
  
      # drop cs
      self.db.drop_collection_space(cs_name)
		
   def tearDown(self):
      try:
         self.db.drop_collection_space('testaltercsdomain15213')
      except SDBBaseError as e:     		
         self.assertEqual(e.code, -34)
      self.db.drop_domain(self.domain_name1)
      self.db.drop_domain(self.domain_name2)
      self.db.disconnect()
      
   def check_domain_cs(self, domain, expect_domain_cs):
      act_domain_cs = list()
      cursor = domain.list_collection_spaces()
      while(True):
         try:
            act_domain_cs.append(cursor.next())
         except SDBEndOfCursor:     		
            break
      self.assertEqual(act_domain_cs, expect_domain_cs)