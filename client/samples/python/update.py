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

   # insert single record
   basketball = {"Item":"basketball", "id":0}
   rc, oid = cl.insert(basketball)
   if const.SDB_OK != rc:
      pysequoiadb.cout("insert record: %s failed, %s"\
                                 % (basketball, pysequoiadb.getErr(rc)))

   pysequoiadb.cout("before update")
   rc, cr = cl.query()
   pysequoiadb.ASSERT(rc == const.SDB_OK)
   rc, record = cr.next()
   while const.SDB_DMS_EOC != rc:
      if const.SDB_OK != rc:
         pysequoiadb.cout("get current record failed: %s" \
                           % pysequoiadb.getErr(rc) )
      else:
         pysequoiadb.cout(record)
      rc, record = cr.next()

   # update records
   update = {'$set':{"Item":"football", "Rank":1 }}
   condition = {'id':{'$exists':0}}
   rc = cl.update(update, condition)
   if const.SDB_OK != rc:
      pysequoiadb.cout("update record:% failed, %s"\
                        % (update, pysequoiadb.getErr(rc)))

   pysequoiadb.cout("after update")
   rc, cr = cl.query()
   pysequoiadb.ASSERT(rc == const.SDB_OK)
   rc, record = cr.next()
   while const.SDB_DMS_EOC != rc:
      if const.SDB_OK != rc:
         pysequoiadb.cout("get current record failed: %s" \
                           % pysequoiadb.getErr(rc) )
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