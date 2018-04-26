# @decription: test attach cl, repeat range
# @testlink:   seqDB-15209
# @interface:  attach_collection(self,cl_full_name,options)
# @author:     liuxiaoxuan 2018-04-25

import unittest
from pysequoiadb.error import SDBBaseError
from lib import testlib

class TestMainCLAttach15209(testlib.SdbTestBase):
   def setUp(self):
      # skip standalone
      if testlib.is_standalone():
         self.skipTest('run mode is standalone')

   def test_miancl_attach_15209(self):
   
      cs_name = 'testcs15209'
      maincl_name = 'testmaincl15209'
      subcl_name1 = 'testsubcl15209_1'
      subcl_name2 = 'testsubcl15209_2'
      
      # create maincl and subcls
      cs = self.db.create_collection_space(cs_name)
      maincl = cs.create_collection(maincl_name, {'ShardingKey' : {'a' : 1}, 'IsMainCL': True})
      cs.create_collection(subcl_name1)
      cs.create_collection(subcl_name2, {'ShardingKey' : {'b' : 1}})
      
      # attach cl
      maincl.attach_collection(cs_name + '.' + subcl_name1, {'LowBound': {'a':1}, 'UpBound': {'a':100}})
      
      # repeat range
      try:
         maincl.attach_collection(cs_name + '.' + subcl_name2, {"LowBound": {'a':1}, "UpBound": {'a':100}})
         self.fail('Need attachCL fail')
      except SDBBaseError as e:
         self.assertEqual(e.code, -237)
         
      # drop cs
      self.db.drop_collection_space(cs_name)
		
   def tearDown(self):
      try:
         self.db.drop_collection_space('testcs15209')
      except SDBBaseError as e:
         self.assertEqual(e.code, -34)
      self.db.disconnect()