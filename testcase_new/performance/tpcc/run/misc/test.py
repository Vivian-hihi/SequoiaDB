#!/bin/env python


import os
s = os.sep

find = False;
for rt,dirs,file in os.walk('/opt/sequoiadb/conf/local'):
  for dir in dirs:
     cfgfile = open('/opt/sequoiadb/conf/local/' + dir + os.sep+'sdb.conf')
     lines = cfgfile.readlines()
     for line in lines:
       linepart = line.split('=')
       if linepart[0].strip() == 'dbpath':
          dbpath = linepart[1].strip()
       elif (linepart[0].strip() == 'role' and linepart[1].strip() == 'data') :
          find = True
          break
     if find:
        break

st = os.statvfs(dbpath)
print st.f_frsize * st.f_blocks / 1000 /1000/1000
