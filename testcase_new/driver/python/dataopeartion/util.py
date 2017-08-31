from lib.config import *
from pysequoiadb import client

def get_default_client():
   config=Config()
   return client( config.host_name, config.service )

