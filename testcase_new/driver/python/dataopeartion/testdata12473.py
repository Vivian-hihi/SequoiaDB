# @decription: data  opeartion
# @testlink:   seqDB-12473
# @author:     LaoJingTang 2017-8-30
import unittest
from datetime import datetime
from pysequoiadb import client
from pysequoiadb import collectionspace
from pysequoiadb import lob
from pysequoiadb.error import (SDBTypeError, SDBBaseError, SDBEndOfCursor)

from bson.objectid import ObjectId

import util

class Data12473(unittest.TestCase):
   def setUp(self):
      print("begin: "+str(datetime.now()))
      self.db=util.get_default_client()
      self.create_cs_cl()


   def subtest(self,cl_list__expect,return_list_expect,update,**kwargs):
      for i in [{"a":i,"b":i} for i in range(10)]:
         self.cl.insert(i)
      if "return_new" not in kwargs:
         kwargs["return_new"]=True
      
      cur=self.cl.query_and_update(update,**kwargs)
      list1=self.get_result()
      list2=self.get_result(cur)
      self.check_result(list1,cl_list__expect)
      self.check_result(list2,return_list_expect)
      self.cl.query_and_remove()
      

   def test(self):
      original_list=[{"a":i,"b":i} for i in range(10)]
      updated_list=[{"a":i+1,"b":i} for i in range(10)]
      updated_list_a=[{"a":i+1} for i in range(10)]

      update={"$inc":{"a":1}}

      #condition+update
      condition={"a":{"$et":1}}
      return_list_expect=[{"a":2,"b":1}]
      l=list(original_list)
      l[1]={"a":2,"b":1}
      self.subtest(l,return_list_expect,update,condition=condition)

      #selector+update
      selector={"b":{"$include":0}}
      self.subtest(updated_list,updated_list_a,update,selector=selector)

      #order_by+update
      order_by={"_id":-1}
      self.subtest(updated_list,updated_list,update,order_by=order_by)

      #num_to_skip+update
      num_to_skip=5
      l=list()
      for i in range(10):
         if i>=5:
            l.append({"a":i+1,"b":i})
         else:
            l.append({"a":i,"b":i})
      self.subtest(l,updated_list[num_to_skip:],update,num_to_skip=num_to_skip)

      #num_to_return+update
      num_to_return=5
      l=list()
      for i in range(10):
         if i<5:
            l.append({"a":i+1,"b":i})
         else:
            l.append({"a":i,"b":i})
      self.subtest(l,updated_list[:num_to_return],update,num_to_return=num_to_return)

      #hint+update
      hint={"":"index"}
      self.subtest(updated_list,updated_list,update,hint=hint)

      #selector+update
      selector={"b":{"$include":0}}
      self.subtest(updated_list,[{"a":i+1} for i in range(10)],update,selector=selector)
      
      #return_new+update
      self.subtest(updated_list,original_list,update,return_new=False)

      #flags+update
      QUERY_FLG_FORCE_HINT=128
      try:  
         self.subtest(updated_list,updated_list,update,flagss=QUERY_FLG_FORCE_HINT,hint=hint)
      except SDBBaseError as e:
         pass
         
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
      
