#! /usr/bin/python

import pysequoiadb
from pysequoiadb import client
from pysequoiadb import const
from pysequoiadb.error import SequoiaDBError

from bson.objectid import ObjectId
from bson.son import SON

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
   data1 = {'no':1000,'score':80,'interest':['basketball','football'],'major':'computer th','dep':'computer','info':{'name':'tom','age':25,'gender':'man'}}
   records.append(data1)
   data2 = {'no':1001,'score':90,'interest':['basketball','football'],'major':'computer sc','dep':'computer','info':{'name':'mike','age':24,'gender':'lady'}}
   records.append(data2)
   data3 = {'no':1002,'score':85,'interest':['basketball','football'],'major':'computer en','dep':'computer','info':{'name':'kkk','age':25,'gender':'man'}}
   records.append(data3)
   data4 = {'no':1003,'score':92,'interest':['basketball','football'],'major':'computer en','dep':'computer','info':{'name':'mmm','age':25,'gender':'man'}}
   records.append(data4)
   data5 = {'no':1004,'score':88,'interest':['basketball','football'],'major':'computer sc','dep':'computer','info':{'name':'ttt','age':25,'gender':'man'}}
   records.append(data5)

   rc = cl.bulk_insert(1, records)
   if const.SDB_OK != rc:
      pysequoiadb.cout("bulk insert record: %s failed, %s"\
                                 % (tennis, pysequoiadb.getErr(rc)))

   match = SON({'$match':{'interest':{'$exists':1}}})
   print "match = ", match

   # I'd like grouped by avg_age first, and major secondly
   group = {'$group':SON([ ('_id','$major'),
                           ('avg_age', SON([('$avg','$info.age')])),
                           ('major',SON([('$First','$major')]))
                            ])}
   print group

   # I'd love the result sort ascending by 'avg_age' first,
   # then, sort descending by 'major'.
   # I need avoid the dict built-in and store data ordere, so I use bson.SON
   sort = {'$sort':SON([('avg_age',1),('major',-1)])}

   print sort
   skip = {'$skip':0}
   limit = {'$limit':5}

   options = [match, group, sort, skip, limit]
   rc, cr = cl.aggregate(options)
   if const.SDB_OK != rc:
      print "failed to aggregate"
   else:
      rc, _ = cr.next()
      if const.SDB_OK == rc:
         rc, doc = cr.current()
         while const.SDB_DMS_EOC != rc:
            if const.SDB_OK != rc:
               print "next error"
            else:
               print doc
            rc, doc = cr.next()

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