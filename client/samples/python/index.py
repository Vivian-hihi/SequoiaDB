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
      rc, cs = cs.create_collection(cl_name)
      if const.SDB_OK != rc:
         pysequoiadb.cout("create collection[%s] failed, %s"\
                                 % (cl_name, pysequoiadb.getErr(rc)))

   index = {'Item':1, 'Rank':-1}
   index_name = 'idx'
   rc = cl.create_index(index, index_name, False, False)
   if const.SDB_OK != rc:
      pysequoiadb.cout("create index:[%s] failed, %s"\
                               % (index_name, pysequoiadb.getErr(rc)))
   pysequoiadb.cout("create index:[%s] success." % index_name)
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