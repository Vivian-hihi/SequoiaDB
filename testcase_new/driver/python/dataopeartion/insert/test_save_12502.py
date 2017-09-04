# @decription: save records 
# @author:     liuxiaoxuan 2017-8-29

import unittest
import datetime
from pysequoiadb import client
from pysequoiadb import collectionspace
from pysequoiadb.error import (SDBTypeError, SDBBaseError, SDBEndOfCursor,SDBError)
from lib.config import *
from bson.objectid import ObjectId

cs_name = "cs_12502"
cl_name = "cl_12502"
class TestSave12502(unittest.TestCase):
   def setUp(self):
      try:
         print(datetime.datetime.now())
         config = Config()
         self.db = client( config.host_name, config.service ) 
         if(self.is_stand_alone()):
            self.skipTest('current environment less than tow groups')
         if(self.one_group_mode()):
            self.skipTest('need at least 2 data groups')  
             
         dataGroupNames = self.get_data_groupnames()
         srcGroupName = dataGroupNames[0]
         destGroupName = dataGroupNames[1]
          
         self.clean_cs()
         self.create_cl(srcGroupName)
         self.insert_datas()    
         self.split_cl(srcGroupName,destGroupName)
      except SDBBaseError as e:
         print(e.detail)
         raise e        
      
   def testSaveFields(self):   
      print( '---begin to save records---' )
      try:
         doc_commNoMatchId = {"a":"newA_withNoMatchId"}
         doc_commNotexistId = {"a":"newA_withNotExistId","_id":ObjectId("66bb5667c5d061d6f579d000")}
         doc_commExistId = {"a":"newA_withExistId","_id":ObjectId("53bb5667c5d061d6f579d0bb")}
         doc_commNewField = {"newField":"upsertNewField","_id":ObjectId("53bb5667c5d061d6f579d0bc")}
         
         self.cl.save(doc_commNoMatchId)  
         self.cl.save(doc_commNotexistId)
         self.cl.save(doc_commExistId)
         self.cl.save(doc_commNewField)
          
         doc_shardNoMatchId = {"no":9999}
         doc_shardNotexistId = {"no":1001,"_id":ObjectId("92bb5667c5d061d6f580d0ab")}
         doc_shardExistId = {"no":2002,"_id":ObjectId("53bb5667c5d061d6f579d0bb")}
         doc_shardNewField = {"newNo":"newNo","_id":ObjectId("53bb5667c5d061d6f579d0bd")}
          
         self.cl.save(doc_shardNoMatchId)  
         self.cl.save(doc_shardNotexistId)
         self.cl.save(doc_shardExistId)
         self.cl.save(doc_shardNewField)           
          
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
      print( '---begin to get data group names' )
      groupNames = []
       
      cr = self.db.get_list(7)   
      while True:
         try:
            rec = cr.next()
            groupNames.append(rec['GroupName'])
         except SDBEndOfCursor:
            break
         except Exception as e:
            raise e
      groupNames.remove("SYSCatalogGroup")
      groupNames.remove("SYSCoord")         
      return groupNames   
 
   def clean_cs(self):
      try:
         print( '---begin to clean cs---')
         self.db.drop_collection_space(cs_name) 
      except SDBError as e:
         pass	
			
   def create_cl(self,srcGroupName):
      try:
         print( '---begin to create cs---')
         self.cs = self.db.create_collection_space(cs_name)            
         #create cl
         option = {"ShardingKey":{'no':1},"ShardingType":'hash',"Group":srcGroupName}
         self.cl = self.cs.create_collection(cl_name , option)
         print( '---create cl success---' )   
      except SDBError as e:
         print(e.detail) 
         raise e    
  
   def insert_datas(self):   
      print( '---begin to insert records---' )
      objectIds = [ObjectId("53bb5667c5d061d6f579d0bb"),\
                   ObjectId("53bb5667c5d061d6f579d0bc"),\
                   ObjectId("53bb5667c5d061d6f579d0bd"),\
                   ObjectId("53bb5667c5d061d6f579d0be"),\
                   ObjectId("53bb5667c5d061d6f579d0bf")]
      for i in range(1,len(objectIds)):
         try:
            self.cl.insert({"_id":objectIds[i],"no":i,"a":"test" + str(i)})  
         except SDBError as e:
            print(e.detail) 
            raise e  
         
   def split_cl(self,srcGroupName,destGroupName):   
      print( '---begin to split---' )
      try:
         self.cl.split_async_by_percent(srcGroupName,destGroupName,50.0)
      except (SDBTypeError, SDBBaseError) as e:
         print(e) 
         raise e               
                             
if __name__ == "__main__":
    unittest.main() 