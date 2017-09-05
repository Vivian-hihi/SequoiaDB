from datetime import datetime
from lib import sdbconfig
from pysequoiadb import client

config = sdbconfig.SdbConfig()

def default_db():
   return client(config.host_name, config.service)

def print_setup_msg(self):
   print(str(self.__class__.__name__)+" setup: " + str(datetime.now()))

def print_teardown_msg(self):
   print(str(self.__class__.__name__)+" teardown: " + str(datetime.now()))
