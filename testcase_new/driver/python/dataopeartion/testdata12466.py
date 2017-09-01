# @decription: data  opeartion
# @testlink:   seqDB-12466
# @author:     LaoJingTang 2017-8-30
import unittest
from datetime import datetime
from pysequoiadb import client
from pysequoiadb import collectionspace
from pysequoiadb import lob
from pysequoiadb.error import (SDBTypeError, SDBBaseError, SDBEndOfCursor)

from bson.objectid import ObjectId

import util

NUM=10
class Data12466(unittest.TestCase):
   def setUp(self):
      print("begin: "+str(datetime.now()))
      self.db=util.get_default_client()
      self.create_cs_cl()


   def subtest(self,cl_list__expect,return_list_expect,upsert,**kwargs):
      for i in [{"a":i,"b":i} for i in range(NUM)]:
         self.cl.insert(i)
      if "return_new" not in kwargs:
         kwargs["return_new"]=True
      
      cur=self.cl.upsert(upsert,**kwargs)
      list1=self.get_result()
      list2=self.get_result(cur)
      self.check_result(list1,cl_list__expect)
      self.check_result(list2,return_list_expect)
      self.cl.query_and_remove()
      

   def test(self):
      original_list=[{"a":i,"b":i} for i in range(NUM)]
      upserted_list=[{"a":i,"b":i} for i in range(NUM)]
      upserted_list.append({"a":100})

      upsert={"$inc":{"a":1}}

      #condition+upsert
      condition={"a":{"$et":99}}
      self.subtest(upserted_list,upserted_list,upsert,condition=condition)

      #hint+upsert
      hint={"":"index"}
      l=[{"a":i+1,"b":i} for i in range(10)]
      self.subtest(l,l,upsert,hint=hint)

      #flags+upsert
      QUERY_FLG_FORCE_HINT=128
      try:  
         self.subtest(l,l,upsert,flagss=QUERY_FLG_FORCE_HINT,hint=hint)
      except SDBBaseError as e:
         pass
         
      #setOnInsert+upsert
      setOnInsert={"a":"aaa"}
      l=[{"a":i,"b":i} for i in range(10)]
      l.append({"a":"aaa"})
      self.subtest(l,l,upsert,condition=condition,setOnInsert=setOnInsert)

      self.db.drop_collection_space(self.cs_name)

   def check_result(self,list1,expect_list):
      if not util.check_result(list1,expect_list): 
            self.fail("check result fail")

   def get_result(self,cur=None):
      if cur==None:
         cur=self.cl.query()
      items=list()
      while True:
         try:
            item=cur.next()
            item.pop('_id')
            items.append(item)
         except BaseException as e:
            break
      return items

   def tearDown(self):
      print("end: "+str(datetime.now()))
      self.db.disconnect()


   def create_cs_cl(self):
      self.cs_name=self.__class__.__name__+"_cs"
      self.cl_name=self.__class__.__name__+"_cl"
      try:
         self.db.drop_collection_space(self.cs_name)
      except BaseException as e:      
         pass
      self.cs=self.db.create_collection_space(self.cs_name)
      self.cl=self.cs.create_collection(self.cl_name)
      
