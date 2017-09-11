# @decription: data  opeartion
# @testlink:   seqDB-12466
# @author:     LaoJingTang 2017-8-30
import unittest

from lib import testlib
from pysequoiadb.error import (SDBBaseError)

NUM=10
class Data12466(testlib.TestDataOprtBase):
   def setUp(self):
      self.create_cs_cl()

   def upsert_test(self,cl_list__expect,upsert,**kwargs):
      for i in [{"a":i,"b":i} for i in range(NUM)]:
         self.cl.insert(i)
      if "return_new" not in kwargs:
         kwargs["return_new"]=True
      
      self.cl.upsert(upsert,**kwargs)
      list1=self.get_records(self.cl.query())
      self.assert_list_equal(cl_list__expect,list1)
      self.cl.delete()

   def test12466(self):
      original_list=[{"a":i,"b":i} for i in range(NUM)]
      upserted_list=[{"a":i,"b":i} for i in range(NUM)]
      upserted_list.append({"a":100})

      upsert={"$inc":{"a":1}}

      #condition+upsert
      condition={"a":{"$et":99}}
      self.upsert_test(upserted_list,upsert,condition=condition)

      #hint+upsert
      hint={"":"index"}
      l=[{"a":i+1,"b":i} for i in range(10)]
      self.upsert_test(l,upsert,hint=hint)

      #flags+upsert
      QUERY_FLG_FORCE_HINT=128
      try:  
         self.upsert_test(l,upsert,flagss=QUERY_FLG_FORCE_HINT,hint=hint)
      except SDBBaseError as e:
         pass
         
      #setOnInsert+upsert
      setOnInsert={"a":"aaa"}
      l=[{"a":i,"b":i} for i in range(10)]
      l.append({"a":"aaa"})
      self.upsert_test(l,upsert,condition=condition,setOnInsert=setOnInsert)

   def tearDown(self):
      if testlib.should_clear_env(self):
         self.drop_cs()
      self.close_db()

