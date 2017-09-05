# @decription: data  opeartion
# @testlink:   seqDB-12466
# @author:     LaoJingTang 2017-8-30
import unittest

from lib import testlib
from pysequoiadb.error import (SDBBaseError)

NUM=10
class Data12466(unittest.TestCase):
   def setUp(self):
      testlib.print_setup_msg(self)
      self.db=testlib.default_db()
      self.create_cs_cl()

   def upsert_test(self,cl_list__expect,upsert,**kwargs):
      for i in [{"a":i,"b":i} for i in range(NUM)]:
         self.cl.insert(i)
      if "return_new" not in kwargs:
         kwargs["return_new"]=True
      
      self.cl.upsert(upsert,**kwargs)
      list1=testlib.get_records(self.cl.query())
      testlib.assert_list_equal(self,cl_list__expect,list1)
      self.cl.delete()

   def test(self):
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

      self.db.drop_collection_space(self.cs_name)

   def tearDown(self):
      testlib.print_teardown_msg(self)
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
      
