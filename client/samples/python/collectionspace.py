#! /usr/bin/python

import pysequoiadb
from pysequoiadb import client
from pysequoiadb import const
from pysequoiadb.error import (SequoiaDBError, InvalidParameter)

if __name__ == "__main__":

   try:
      # connect to local db, using default args value.
      # host='localhost', port= 11810, user= '', password= ''
      db = client()
      # create collection space
      # try to get a collection space named by cs_name specified
      cs_name = 'sports'
      cs = db.create_collection_space( cs_name )

      # 1.get collection space
      cs_name = 'sports'
      # try to get a collection space named by cs_name specified
      cs = db.get_collection_space( cs_name )

      # 2.get collection space
      # try to get a collection named by 'sports' use __getitem__
      cs = db[cs_name]
      pysequoiadb.cout("get collection space:[%s] success" % cs_name)

      # 3.get collection space
      # try to get a collection named by 'sports' use __getattri__
      cs = cs.sports
      pysequoiadb.cout("get collection space:[%s] success." % 'cs_third')

      # release
      cs_name = cs.get_collection_space_name()
      db.drop_collection_space(cs_name)
      del cs

      del db

   except (InvalidParameter, SequoiaDBError), e:
      pysequoiadb.cout(e)
      

   

   

   

   

   