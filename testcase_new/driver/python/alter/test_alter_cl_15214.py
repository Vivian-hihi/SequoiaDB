# @decription: test alter cl
# @testlink:   seqDB-15214
# @interface:  alter(self, options)
# @author:     liuxiaoxuan 2018-04-25

import unittest
from pysequoiadb.error import (SDBBaseError)
from alter.commlib import *
from lib import testlib

class TestAlterCL15214(testlib.SdbTestBase):
   def setUp(self):
      # skip standalone
      if testlib.is_standalone():
         self.skipTest('run mode is standalone')
      
      # create cs cl   
      testlib.drop_cs(self.db, self.cs_name, ignore_not_exist=True)      
      self.cs = self.db.create_collection_space(self.cs_name)
      self.cl = self.cs.create_collection(self.cl_name)

   def test_alter_cl_15214(self):
      
      # alter sharding attributes
      alter_opts1 = {'ShardingKey': {'a':1}, 'ShardingType': 'hash', 'Partition': 128, 'EnsureShardingIndex': False, 'AutoSplit': True}
      self.cl.alter(options = alter_opts1)
      
      # check attributes
      expect_attributes1 = [{'AttributeDesc': '', 'ShardingKey': {'a':1}, 'ShardingType': 'hash', 'Partition': 128, 'EnsureShardingIndex': False, 'AutoSplit': True}];
      self.check_collection_attributes(expect_attributes1, condition = {'Name' : self.cs_name + '.' + self.cl_name})
      
      # alter compressed attributes
      alter_opts2 = {'CompressionType': 'lzw'}
      self.cl.alter(options = alter_opts2)
      
      # check attributes
      opts2_attributes = {'AttributeDesc': 'Compressed', 'CompressionType': 1, 'CompressionTypeDesc': 'lzw'}
      # include alter_opts1
      new_attributes = expect_attributes1[0].copy()
      new_attributes.update(opts2_attributes)
      expect_attributes2 = [new_attributes];
      self.check_collection_attributes(expect_attributes2, condition = {'Name' : self.cs_name + '.' + self.cl_name})
      
      # alter index attributes
      alter_opts3 = {'AutoIndexId': False}
      self.cl.alter(options = alter_opts3)
      
      # check attributes
      # include alter_opts2
      new_attributes = expect_attributes2[0].copy()
      new_attributes.update(alter_opts3)
      expect_attributes3 = [new_attributes];
      self.check_collection_attributes(expect_attributes3, condition = {'Name' : self.cs_name + '.' + self.cl_name})
      
      # alter other common attributes
      alter_opts4 = {'ReplSize': 3, 'StrictDataMode': True}
      self.cl.alter(options = alter_opts4)
      
      # check attributes
      opts4_attributes = {'ReplSize': 3, 'AttributeDesc': 'Compressed | StrictDataMode'}
      # include alter_opts3
      new_attributes = expect_attributes3[0].copy()
      new_attributes.update(opts4_attributes)
      expect_attributes4 = [new_attributes];
      self.check_collection_attributes(expect_attributes4, condition = {'Name' : self.cs_name + '.' + self.cl_name})
      
      # alter capped attributes
      capped_cs = self.db.create_collection_space('cappedcs15214', {'Capped': True})
      capped_cl = capped_cs.create_collection('cappedcl15214', {'Capped': True,'Size': 32, 'Max': 10000, 'OverWrite': False, 'AutoIndexId': False})
      
      alter_opts5 = {'Size': 1, 'Max': 9999999, 'OverWrite': True}
      capped_cl.alter(options = alter_opts5)
      
      # check capped cl attributes
      expect_attributes5 = [{'AttributeDesc': 'NoIDIndex | Capped', 'AutoIndexId': False, 'Max': 9999999, 'OverWrite': True, 'Size': 33554432}]
      self.check_collection_attributes(expect_attributes5, condition = {'Name' : 'cappedcs15214.cappedcl15214'})
    
      # drop cappedcs
      self.db.drop_collection_space('cappedcs15214')
    
      # create new cl
      newcl_name = 'testaltercl15214_new'
      newcl = self.cs.create_collection(newcl_name)
      
      # check new cl attributes before bulk alter
      expect_bulk_attributes = [{'AttributeDesc': ""}]
      self.check_collection_attributes(expect_bulk_attributes, condition = {'Name' : self.cs_name + '.' + newcl_name})

      # bulk alter attributes, ignore exception
      bulk_opts = {'Alter':[ {'Name': 'enable sharding', 'Args':{'ShardingKey':{'a':1},'ShardingType':'hash','AutoSplit':True}}, 
                             {'Name': 'enable compression', 'Args':{'CompressionType':'lzw'}},
                             {'Name':'set attributes', 'Args': {'Name':'cs.cl'}}, 
                             {'Name':'set attributes', 'Args': {'StrictDataMode':True}}], 
                   'Options':{'IgnoreException': True}} 
      newcl.alter(options = bulk_opts)
  
      # check cl attributes after bulk alter
      expect_bulk_attributes = [{'ShardingKey':{'a': 1},'ShardingType':'hash','AutoSplit':True,
                                 'EnsureShardingIndex': True, 'Partition': 4096,
                                 'CompressionType': 1, 'CompressionTypeDesc': 'lzw',
                                 'AttributeDesc': 'Compressed | StrictDataMode'}]
      self.check_collection_attributes(expect_bulk_attributes, condition = {'Name' : self.cs_name + '.' + newcl_name})

      # bulk alter, not ignore exception, must fail
      bulk_opts = {'Alter':[ {'Name':'set attributes', 'Args': {'Name':'cs.cl'}},
                             {'Name': 'enable sharding', 'Args':{'ShardingKey':{'b':1},'ShardingType':'range'}}, 
                             {'Name':'set attributes', 'Args': {'ReplSize':3}}], 
                   'Options':{'IgnoreException': False}} 
      try:
         newcl.alter(options = bulk_opts)  
         self.fail('need alter failed!')       
      except SDBBaseError as e:
         self.assertEqual(e.code, -6)  

      # check after bulk fail
      self.check_collection_attributes(expect_bulk_attributes, condition = {'Name' : self.cs_name + '.' + newcl_name})
          
   def tearDown(self):
      try:
         self.db.drop_collection_space('cappedcs15214')
      except SDBBaseError as e:
         self.assertEqual(e.code, -34)
      self.db.drop_collection_space(self.cs_name)   
      self.db.disconnect()
      
   def check_collection_attributes(self, expect_attributes, **kwargs):
      act_attributes = get_collection_attributes(self.db, **kwargs)
      act_attributes = get_sort_result(act_attributes)
      expect_attributes = get_sort_result(expect_attributes)
      self.assertEqual(act_attributes, expect_attributes)
       