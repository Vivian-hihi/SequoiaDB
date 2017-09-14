# @decription: test connect/disconnect node
# @testlink:   seqDB-12501
# @interface:  connect(self,host,service,kwargs )
#              connect_to_hosts(self,hostskwargs )
#              disconnect(self)
#              is_valid(self)
# @author:     liuxiaoxuan 2017-9-08

import unittest
import datetime
from pysequoiadb.error import (SDBTypeError, SDBBaseError, SDBEndOfCursor, SDBError)
from lib import testlib
import random

cs_name = "cs_12501"
cl_name = "cl_12501"
username = 'user'
password = 'password'
cl_full_name = cs_name + "." + cl_name
insert_nums = 100
class TestConnect12501(unittest.TestCase):
   def setUp(self):
      testlib.print_setup_msg(self)
      self.db = testlib.default_db()
      if (self.is_stand_alone()):
         self.skipTest('current environment is standalone')
      dataGroupNames = self.get_data_groupnames()
      self.groupName = dataGroupNames[0]
      self.create_cs_cl(cs_name,cl_name)
      self.insert_datas()

   def testConnect12501(self):
      # check catalog
      condition = {'Name': cl_full_name}
      expectResult = {'Name': cl_full_name , 'ReplSize': 3 , 'GroupName': self.groupName}
      self.get_catalog_info(condition,expectResult)

      #check connect data node
      self.check_connect_node()
      #check disconnect
      self.check_disconnect_node()

   def tearDown(self):
      try:
         testlib.print_teardown_msg(self)
         # reconnect sdb
         self.db = testlib.default_db()
         self.db.drop_collection_space(cs_name)
         self.db.disconnect()
      except SDBBaseError as e:
         if (-34 != e.code):
            self.fail('teardown fail: ' + e.detail)

   def is_stand_alone(self):
      try:
         cursor = self.db.list_replica_groups()
      except SDBBaseError as e:
         if(-159 == e.code):
            return True
      return False

   def get_data_groupnames(self):
      groupNames = []
      cr = self.db.get_list(7)
      while True:
         try:
            rec = cr.next()
            groupNames.append(rec['GroupName'])
         except SDBEndOfCursor:
            break
      groupNames.remove("SYSCatalogGroup")
      groupNames.remove("SYSCoord")
      return groupNames

   def clean_cs(self,csname):
      try:
         self.db.drop_collection_space(csname)
      except SDBBaseError as e:
         pass

   def create_cs_cl(self,csname,clname):
      self.clean_cs(csname)
      try:
         self.cs = self.db.create_collection_space(csname)
         # create cl with options
         options = {'ReplSize': 3, 'Group': self.groupName}
         self.cl = self.cs.create_collection(clname, options)
         print('create cl success')
      except SDBBaseError as e:
         self.fail('create cl fail: ' + e.detail)

   def insert_datas(self):
      flag = 0
      doc = []
      for i in range(0, insert_nums):
         doc.append({"a": "test" + str(i)})
      try:
         self.cl.bulk_insert(flag, doc)
      except SDBBaseError as e:
         self.fail('insert fail: ' + e.detail)

   def get_catalog_info(self,cond,expectRec):
      try:
         # connect to catalog primary node
         cata_rg = self.db.get_replica_group_by_name('SYSCatalogGroup')
         cata_node = cata_rg.get_master().get_nodename()
         node_name = cata_node.split(":")

         hostname = node_name[0]
         svcname = node_name[1]
         self.db.connect(hostname,svcname,user = username,password = password)

         # check catalog info
         cata_cl = self.db.get_collection('SYSCAT.SYSCOLLECTIONS')
         actRec = cata_cl.query(condition = {'Name': cl_full_name}).next()

         self.check_catalog_result(expectRec, actRec)
      except SDBBaseError as e:
         self.fail('check catalog fail: ' + e.detail)

   def check_catalog_result(self,expectRec,actRec):
      msg = str(expectRec) + " not equal " + str(actRec)
      catalog = actRec['CataInfo']
      info = catalog[0]
      self.assertEqual(expectRec['Name'], actRec['Name'], msg)
      self.assertEqual(expectRec['ReplSize'], actRec['ReplSize'], msg)
      self.assertEqual(expectRec['GroupName'], info['GroupName'], msg)

   def get_data_nodes(self):
      nodeAddrs = []
      try:   
         # reconnect to coord
         self.db.disconnect()
         self.db = testlib.default_db()
         # get nodes
         rg = self.db.get_replica_group_by_name(self.groupName)
         rec = rg.get_detail()
         groups = rec['Group']

         for i in range(len(groups)):
            group = groups[i]
            host_name = group['HostName']
            service = group['Service']
            svc = service[0]
            svc_name = svc['Name']
            nodeAddrs.append({'host' : host_name , 'service': svc_name})

      except SDBBaseError as e:
         self.fail("get groupAdrr fail: " + e.detail)

      return nodeAddrs

   def check_connect_node(self):
      try:
         repeatTime = 10
         hosts = self.get_data_nodes()

         for i in range(repeatTime):
            # choose a random policy option
            option = random.choice(['local_first','one_by_one','random'])
            # connect to a data node with option
            self.db.connect_to_hosts(hosts, user = username, password = password, policy = option)
            # check data result
            self.check_connect_result()

      except SDBBaseError as e:
         self.fail("connect to node fail: " + e.detail)

   def check_connect_result(self):
      try:
         # get new cl
         new_cl = self.db.get_collection(cl_full_name)
         actCount = new_cl.get_count()
         self.assertEqual(insert_nums, actCount)
      except SDBBaseError as e:
         self.fail('check node fail: ' + e.detail)

   def check_disconnect_node(self):
      try:
         self.db.disconnect()
         # check disconnect
         expectRec = 0
         self.check_disconnect_result(expectRec)
      except SDBBaseError as e:
         self.fail('disconnect node fail: ' + e.detail)

   def check_disconnect_result(self,expectRec):
      try:
         actRec = self.db.is_valid()
         self.assertEqual(expectRec,actRec,'node still valid')
      except SDBBaseError as e:
         self.fail('check disconnect node fail: ' + e.detail)
