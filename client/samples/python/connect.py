#! /usr/bin/python

import pysequoiadb
from pysequoiadb import client
from pysequoiadb import const
from pysequoiadb.error import (InvalidParameter, SequoiaDBError)

if "__main__" == __name__:

   # connect to local db, using default args value.
   # host= 'localhost', port= 11810, user= '', password= ''
   try:
      db_default = client()
      del db_default
   except (InvalidParameter, SequoiaDBError), e:
      pysequoiadb._print(e)

   # connect to db, using default args value.
   # host= '192.168.20.111', port= 50000, user= 'db_admin', password= 'password'
   try:
      db_to_1 = client('192.168.20.111', 50000, 'db_admin', 'password')
      del db_to_1
   except (InvalidParameter, SequoiaDBError), e:
      pysequoiadb._print(e)

   # connect to db, using default args value.
   # host= 'localhost', port= 11810, user= '', password= ''
   try:   
      db = client()

      host = "192.168.30.63"
      service = '80000'
      db.connect(host, 11810, 'admin', '*****')

      # try to connect another db server by service
      db.connect(host, service)
      db.disconnect()
      # try to connect other db server
      hosts = [ {'host':'192.168.20.48', 'service':11810},
                {'host':'192.168.20.111', 'service':50000},
                {'host':'localhost', 'service':11810} ]
      db.connect_to_hosts(hosts)

      # close connection to db server
      db.disconnect()

      # release clinet
      del db

   except (InvalidParameter, SequoiaDBError), e:
      pysequoiadb._print(e)
      
   
