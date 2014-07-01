#! /usr/bin/python

import pysequoiadb
from pysequoiadb import client
from pysequoiadb import const
from pysequoiadb.error import SequoiaDBError

if "__main__" == __name__:

   # connect to local db, using default args value.
   # host= 'localhost', port= 11810, user= '', password= ''
   try:
      db_default = client()
   except SequoiaDBError:
      pysequoiadb.cout('SDB_OOM appeared. Please check available memory.' )
      del db_default
      exit()

   # if no error occurs, connect to specified server successfully
   #
   # do something
   #
   # Need to release clinet whether it connected db server successfully or not
   del db_default

   # connect to db, using default args value.
   # host= '192.168.20.111', port= 50000, user= 'db_admin', password= 'password'
   try:
      db_to_1 = client('192.168.20.111', 50000, 'db_admin', 'password')
   except SequoiaDBError:
      pysequoiadb.cout('SDB_OOM appeared. Please check available memory.')
      del db_to_1
      exit()
   # if no error occurs, connect to specified server successfully
   #
   # do something
   #
   # Need to release client whether it connected db server successfully or not
   del db_to_1

   # connect to db, using default args value.
   # host= 'localhost', port= 11810, user= '', password= ''
   try:   
      db = client()
   except SequoiaDBError:
      pysequoiadb.cout('SDB_OOM appeared. Please check available memory.')
      del db
      exit()

   # if no error occurs, succeed to connect db server at localhost
   #
   # do something
   #
   # try to connect other db server
   host = "192.168.30.63"
   service = '80000'
   rc = db.connect_by_host(host, 11810, 'admin', '*****')
   if const.SDB_OK != rc:
      pysequoiadb.cout("connect to server [%s] failed, %s"\
                               % (host, pysequoiadb.getErr(rc)))
   #
   # do something
   #
   # close connection to db server
   db.disconnect()

   # try to connect another db server
   rc = db.connect_by_service(host, service)
   if const.SDB_OK != rc:
      pysequoiadb.cout("connect to server [%s] failed, %s"\
                                 % (host, pysequoiadb.getErr(rc)))
   #
   # do something
   #
   # create a new user, if succeed, new user can connect to db server.
   user = 'guest'
   psw  = 'guest'
   rc = db.create_user(user, psw)
   if const.SDB_OK != rc:
      pysequoiadb.cout("failed to create user %s, %s"\
                               % (user, pysequoiadb.getErr(rc)))

   #
   # do something
   #
   # remove a user
   rc = db.remove_user(user, psw)
   if const.SDB_OK != rc:
      pysequoiadb.cout("failed to remove user:%s, %s"\
                              % (user, pysequoiadb.getErr(rc)))
   # Need to release clinet
   del db
   
