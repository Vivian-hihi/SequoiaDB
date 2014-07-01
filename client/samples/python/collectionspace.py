#! /usr/bin/python

import pysequoiadb
from pysequoiadb import client
from pysequoiadb import const
from pysequoiadb import SequoiaDBError

if __name__ == "__main__":

   # connect to local db, using default args value.
   # host='localhost', port= 11810, user= '', password= ''
   try:
      db = client()
   except SequoiaDBError:
      pysequoiadb.cout('SDB_OOM appeared. Please check available memory.')
      del da_default

   # if no error occurs, connect to specified server successfully

   # create collection space
   # try to get a collection space named by cs_name specified
   cs_name = 'sports'
   rc, cs = db.create_collection_space( cs_name )
   if const.SDB_OK != rc:
      pysequoiadb.cout("create collection space:[%s]\
                             failed, %s" % (cs_name, pysequoiadb.getErr(rc) ) )
   else:
      # ok, succeed to create collection space
      # do something operation
      pass

   # 1.get collection space
   cs_name = 'sports'
   # try to get a collection space named by cs_name specified
   rc, cs = db.get_collection_space( cs_name )
   if const.SDB_OK != rc:
      pysequoiadb.cout("get collection space:[%s]\
                             failed, %s" % (cs_name, pysequoiadb.getErr(rc) ) )
   else:
      # ok, succeed to create collection space
      # do something operation
      pass

   # 2.get collection space
   # try to get a collection named by 'anther_cs'
   cs_name = 'anther_cs'
   cs_another = db[cs_name]
   if cs_another is None:
      pysequoiadb.cout("get collection space:[%s] failed" % cs_name)
   else:
      # ok, succeed to get collection space
      # do something operation
      pass

   # 3.get collection space
   # try to get a collection named by 'cs_test'
   cs_third = cs.cs_third
   if cs_third is None:
      pysequoiadb.cout("get collection space:[%s] failed." % 'cs_third')
   else:
      # ok, succeed to get collection space
      # do something operation
      pass

   cl_name = 'tst'
   rc = cs.drop_collection(cl_name)
   if const.SDB_OK != rc:
      pysequoiadb.cout("drop collection:[%s] failed." % cl_name)

   # get the name of collection space
   name = cs.get_collection_space_name();
   pysequoiadb.cout( name )

   db.drop_collection_space(name)

   del cs
   del db