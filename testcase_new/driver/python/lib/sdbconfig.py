import json
import os
import os.path

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
      """
      print(self.hostName)
      print(self.service)
      print(self.changedPrefix)
      print(self.rsrvPortBegin)
      print(self.rsrvPortEnd)
      print(self.rsrvNodeDir)
      print(self.workDir)
      """
      fp.close()





"""
if __name__ == "__main__":
   print str(id(default_db))
   print str(id(default_db))
   print str(id(default_db))
"""
