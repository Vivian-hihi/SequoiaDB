#! /usr/bin/python

import pysequoiadb
from pysequoiadb import client
from pysequoiadb import const
from pysequoiadb.error import SequoiaDBError

from bson.objectid import ObjectId

if __name__ == "__main__":

   # connect to local db, using default args value.
   # host= 'localhost', port= 11810, user= '', password= ''
   try:
      db = client()
   except SequoiaDBError:
      print('SDB_OOM appeared. Please check available memory.')
   
   cs_name = "gymnasium"
   rc, cs = db.get_collection_space(cs_name)
   if const.SDB_OK != rc and rc == -34:
      rc, cs = db.create_collection_space(cs_name)
      if const.SDB_OK != rc:
         pysequoiadb.cout("create collection space[%s] failed, %s"\
                               % (cs_name, pysequoiadb.getErr(rc)))

   cl_name = "sports"
   rc, cl = cs.get_collection(cl_name)
   if const.SDB_OK != rc and rc == -23:
      rc, cl = cs.create_collection(cl_name)
      if const.SDB_OK != rc:
         pysequoiadb.cout("create collection[%s] failed, %s"\
                                 % (cl_name, pysequoiadb.getErr(rc)))

   # insert records
   records = []
   for idx in range(0, 10):
      name = 'SequoiaDB' + str(idx)
      sport = {"Rank":idx, "Name":name}
      records.append(sport)
   rc = cl.bulk_insert(1, records)
   if const.SDB_OK != rc:
      pysequoiadb.cout("bulk insert record: %s failed, %s"\
                                 % (tennis, pysequoiadb.getErr(rc)))


   full_name = cl.get_full_name()
   sql1 = "select * from %s" % full_name
   sql2 = "insert into %s ( Rank, Name ) values( 10000, 'SequoiaDB' )"\
          % full_name

   # execute sql1
   rc, cr = db.exec_sql(sql1)
   if const.SDB_OK != rc:
      pysequoiadb.cout("execute sql: %s failed, %s"\
                                 % (sql1, pysequoiadb.getErr(rc)))
   pysequoiadb.cout("The result are below after execute sql:%s" % sql1)
   rc, record = cr.next()
   while const.SDB_DMS_EOC != rc:
      if const.SDB_OK != rc:
         pysequoiadb.cout("get current record failed. %s"\
                                        % pysequoiadb.getErr(rc))
      else:
         pysequoiadb.cout(record)
      rc, record = cr.next()

   pysequoiadb.cout('\n')

   # execute sql2
   rc = db.exec_update(sql2)
   if const.SDB_OK != rc:
      pysequoiadb.cout("execute sql: %s failed, %s"\
                                 % (sql2, pysequoiadb.getErr(rc)))

   pysequoiadb.cout("The result are below after execute sql:%s" % sql2)
   rc, cr = cl.query()
   if const.SDB_OK != rc:
      pysequoiadb.cout("query failed, %s" % pysequoiadb.getErr(rc))
  
   rc, record = cr.next()
   while const.SDB_DMS_EOC != rc:
      if const.SDB_OK != rc:
         pysequoiadb.cout("get current record failed. %s",\
                                          pysequoiadb.getErr(rc))
      else:
         pysequoiadb.cout(record)
      rc, record = cr.next()

   # drop collection
   cl_name = cl.get_collection_name()
   rc = cs.drop_collection( cl_name )
   if const.SDB_OK != rc:
      pysequoiadb.cout("drop collection: %s failed, %s"\
                                 % (cl_name, pysequoiadb.getErr(rc)))

   # drop collection space
   cs_name = cs.get_collection_space_name()
   rc = db.drop_collection_space(cs_name)
   if const.SDB_OK != rc:
      pysequoiadb.cout("drop collection space: %s failed, %s"\
                                 % (cs_name, pysequoiadb.getErr(rc)))

   del cl
   del cs

   db.disconnect()
   del db