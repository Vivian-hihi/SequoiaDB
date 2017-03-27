#coding=utf-8

import threading
import shutil
import os
import commands
import sys

def copyData(dir, size):
   if not os.access(dir, os.F_OK):
      return
   srcDir = os.path.join(dir, size)
   if not os.access(srcDir, os.F_OK):
      return
   
   for parent,dirnames, filenames in os.walk(srcDir):
      for filename in filenames:
         shutil.copy(os.path.join(parent, filename), dir) 

def stopSdbcm():
   print "stop sdbcm ..."
   return
   (status,output) = commands.getstatusoutput('service sdbcm stop')
   if status != 0:
      print output
      sys.exit(1)

def startSdbcm():
   print "start sdbcm..." 
   return 
   (status,output) = commands.getstatusoutput('service sdbcm start')
   if status != 0:
      print output
      sys.exit(1)

def getInstallPath():
   defaultInstallPath='/opt/sequoiadb/'
   defaultCfg = '/etc/default/sequoiadb' 
   if not os.access(defaultCfg, os.F_OK):
      return defaultInstallPath
   try:
      file = open(defaultCfg, 'r')
      lines = file.readlines()
      for line in lines:
         parts = line.split('=')
         if len(parts) == 2 and parts[0] == 'INSTALL_DIR':
            return parts[1].strip()
      return defaultInstallPath
   finally:
      file.close()
   
def main():
   if len(sys.argv) != 2:
      print '%s <warehouses 2000|6000>'%sys.argv[0]
      sys.exit(1)  
   datasize = '100g'
   if sys.argv[1] == 6000:
      datasize = '300g'

   installPath = getInstallPath()

   binPath = os.path.join(installPath, 'bin')
   sdbList = os.path.join(binPath, 'sdblist')
   
   (status,output) = commands.getstatusoutput(sdbList + ' -l -r data|sed \'1d;$d\'|awk \'{print $10}\'')
   if status != 0:
      sys.exit(1)

   threads = []
   dirs = output.split('\n')
   for dir in dirs:
      if not os.access(dir, os.F_OK):
         continue
      t = threading.Thread(target=copyData , args=(dir, datasize))
      threads.append(t) 
   stopSdbcm()
   for t in threads:
      t.setDaemon(True)
      t.start()
   for t in threads:
      t.join()
   startSdbcm()
   
main()
