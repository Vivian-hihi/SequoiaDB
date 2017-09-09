from lib import testlib

class TestSql12481(testlib.TestDataOprtBase):
   def setUp(self):
      self.init_db()

   def tearDown(self):
      self.close_db()

   def test_select(self):
      self.create_cs_cl()
      self.cl.bulk_insert(0,[{"a":1} for i in range(10)])
      db=self.db
      #select
      sql="select a from "+self.cl_name_qualified
      cur=db.exec_sql(sql)
      e=[{"a":1} for i in range(10)]
      self.assert_list_equal(e,self.get_records(cur))

      #select avg
      sql="select avg(a) as avg from "+self.cl_name_qualified
      cur=db.exec_sql(sql)
      a=self.get_records(cur)
      e=[{"avg":1}]
      self.assert_list_equal(e,a)

      #select group by
      sql="select count(a) as a from "+self.cl_name_qualified+" group by a"
      cur=db.exec_sql(sql)
      a=self.get_records(cur)
      e=[{"a":10}]
      self.assert_list_equal(e,a)

      #select join
      sql="select t1.a,t2.a from "+self.cl_name_qualified+" as t1 inner join "+self.cl_name_qualified+" as t2 on t1.a =t2.a"
      cur=db.exec_sql(sql)
      a=self.get_records(cur)
      e=[{"a":1,"a":1} for i in range(100)]
      self.assert_list_equal(e,a)
      self.drop_cs()

   def test_sql(self):
      self.drop_cs()
      #create CS
      db=self.db
      sql="create collectionspace "+self.cs_name
      db.exec_update(sql)

      #list CS
      sql="list collectionspaces"
      cur=db.exec_sql(sql)
      r=self.get_records(cur=cur)
      self.assertIn({"Name":self.cs_name},r)

      #create CL
      sql="create collection "+self.cs_name+"."+self.cl_name
      db.exec_update(sql)

      #list CL
      sql="list collections"
      cur=db.exec_sql(sql)
      r=self.get_records(cur)
      self.assertIn({"Name":self.cl_name_qualified},r)

      #insert
      for i in range(10):
         sql="insert into "+self.cl_name_qualified+"(a) values("+str(i)+")"
         db.exec_update(sql)
      e=[{"a":i} for i in range(10)]
      a=self.get_records()
      self.assert_list_equal(e,a)

      #update
      sql="update "+self.cl_name_qualified+" set a=0"
      db.exec_update(sql)
      e=[{"a":0} for i in range(10)]
      a=self.get_records()
      self.assert_list_equal(e,a)

      #delete
      sql="delete from "+self.cl_name_qualified
      db.exec_update(sql)
      e=[]
      a=self.get_records()
      self.assert_list_equal(e,a)

      #dropCs
      sql="drop collectionspace "+self.cs_name
      db.exec_update(sql)
      cur=db.list_collection_spaces()
      r=self.get_records(cur)
      self.assertNotIn({"Nam":self.cs_name},r)

