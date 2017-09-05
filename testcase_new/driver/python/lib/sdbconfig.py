import os.path
import json
import os
from pysequoiadb import client

class SdbConfig(object):

   def __init__(self):
      self.get_config()
      
   def get_config(self): 
      config_file = os.path.join(os.getcwd(), "config.json")
      fp = open(config_file)
      configs = json.load(fp)
      self.host_name = configs['HOSTNAME']
      self.service = configs['SVCNAME']
      self.changed_prefix = configs['CHANGEDPREFIX']
      self.rsrv_port_begin = configs['RSRVPORTBEGIN']
      self.rsrv_port_end = configs['RSRVPORTEND']
      self.rsrv_node_dir = configs['RSRVNODEDIR']
      self.work_dir = configs['WORKDIR']
      self.break_on_failure = configs['BREAK_ON_FAILURE']
      """
      print(self.host_name)
      print(self.service)
      print(self.changed_prefix)
      print(self.rsrv_port_begin)
      print(self.rsrv_port_end)
      print(self.rsrv_node_dir)
      print(self.work_dir)
      print(self.break_on_failure)
      """
      fp.close()

"""
if __name__ == "__main__":
   print str(id(default_db))
   print str(id(default_db))
   print str(id(default_db))
"""
