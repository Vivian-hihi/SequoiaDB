# @decription: save records
# @testlink:   seqDB-12502
# @interface:  save(self,doc)
# @author:     liuxiaoxuan 2017-8-29

import unittest
import datetime
from pysequoiadb.error import (SDBTypeError, SDBBaseError, SDBEndOfCursor,SDBError)
from bson.objectid import ObjectId
from lib import testlib

cs_name = "cs_12502"
cl_name = "cl_12502"
class TestSave12502(unittest.TestCase):
   def setUp(self):
      testlib.print_setup_msg(self)
      self.db = testlib.default_db()
      if (self.is_stand_alone()):
         self.skipTest('current environment less than tow groups')
      if (self.one_group_mode()):
         self.skipTest('need at least 2 data groups')

      dataGroupNames = self.get_data_groupnames()
      srcGroupName = dataGroupNames[0]
      destGroupName = dataGroupNames[1]

      self.create_cl(srcGroupName)
      self.insert_datas()
      self.split_cl(srcGroupName, destGroupName)
      
   def testSaveFields(self):   
      try:
         # insert common field without match id
         doc_commNoMatchId = {"a":"newA_withNoMatchId"}
         self.cl.save(doc_commNoMatchId)

         condition1 = doc_commNoMatchId
         expectCount1 = 1
         self.check_result(condition1,expectCount1)

         # insert common field and id not exsit
         doc_commNotexistId = {"a": "newA_withNotExistId","_id": ObjectId("66bb5667c5d061d6f579d000")}
         self.cl.save(doc_commNotexistId)

         condition2 = doc_commNotexistId
         expectCount2 = 1
         self.check_result(condition2, expectCount2)

         # upsert common field and id exsit
         doc_commExistId = {"a": "newA_withExistId","_id": ObjectId("53bb5667c5d061d6f579d0bb")}
         self.cl.save(doc_commExistId)

         condition3 = doc_commExistId
         expectCount3 = 1
         self.check_result(condition3, expectCount3)

         # insert shared field without match id
         doc_shardNoMatchId = {"no":9999}
         self.cl.save(doc_shardNoMatchId)

         condition4 = doc_shardNoMatchId
         expectCount4 = 1
         self.check_result(condition4, expectCount4)

         # insert shared field and id not exist
         doc_shardNotexistId = {"no":1001,"_id":ObjectId("92bb5667c5d061d6f580d0ab")}
         self.cl.save(doc_shardNotexistId)

         condition5 = doc_shardNotexistId
         expectCount5 = 1
         self.check_result(condition5, expectCount5)

         # update shared field and id exist
         doc_shardExistId = {"no":2002,"_id":ObjectId("53bb5667c5d061d6f579d0bb")}
         self.cl.save(doc_shardExistId)

         condition6 = doc_shardExistId
         expectCount6 = 0
         self.check_result(condition6, expectCount6)

         # insert new field
         doc_commNewField = {"newField": "upsertNewField"}
         self.cl.save(doc_commNewField)

         condition7 = doc_commNewField
         expectCount7 = 1
         self.check_result(condition7, expectCount7)

      except SDBError as e:
         print(e.detail) 
         raise e                    
                
   def tearDown(self):
      try:
         print(datetime.datetime.now())
         self.db.drop_collection_space(cs_name)
         self.db.disconnect()
      except SDBBaseError as e:
         if(-34 != e.code):
            print(e.detail)
            raise e
 
   def is_stand_alone(self):
      try:
         cursor = self.db.list_replica_groups()
      except SDBBaseError as e:
         if(-159 == e.code):
            return True
      return False           
   
   def one_group_mode(self):
      size = len(self.get_data_groupnames())
      if (2 > size):
         return True; 
      return False;

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
 
   def clean_cs(self):
      try:
         self.db.drop_collection_space(cs_name)
      except SDBBaseError as e:
         pass	
			
   def create_cl(self,srcGroupName):
      self.clean_cs()
      try:
         self.cs = self.db.create_collection_space(cs_name)
         #create cl
         option = {"ShardingKey":{'no':1},"ShardingType":'hash',"Group":srcGroupName}
         self.cl = self.cs.create_collection(cl_name , option)
         print( 'create cl success' )
      except SDBBaseError as e:
         print(e.detail) 
         raise e    
  
   def insert_datas(self):   
      objectIds = [ObjectId("53bb5667c5d061d6f579d0bb"),\
                   ObjectId("53bb5667c5d061d6f579d0bc"),\
                   ObjectId("53bb5667c5d061d6f579d0bd"),\
                   ObjectId("53bb5667c5d061d6f579d0be"),\
                   ObjectId("53bb5667c5d061d6f579d0bf")]
      flag = 0
      doc = []
      for i in range(0,len(objectIds)):
         doc.append({"_id":objectIds[i],"no":i,"a":"test" + str(i)})
      try:
         self.cl.bulk_insert(flag,doc)
      except SDBBaseError as e:
         print(e.detail)
         raise e
         
   def split_cl(self,srcGroupName,destGroupName):   
      try:
         self.cl.split_async_by_percent(srcGroupName,destGroupName,50.0)
      except SDBBaseError as e:
         print(e.detail)
         raise e

   def check_result(self,cond,expectCount):
      actCount = 0
      try:
         actCount = self.cl.get_count(condition = cond)
         self.assertEqual(actCount,expectCount)
      except SDBBaseError as e:
         self.fail('check result fail: ' + e.detail)
                             