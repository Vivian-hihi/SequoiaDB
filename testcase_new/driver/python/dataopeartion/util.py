from lib.config import *
from pysequoiadb import client

def get_default_client():
   config=Config()
   return client( config.host_name, config.service )

def check_result(list1,list2):
   for x in list1:
      if x not in list2:
         print("actually: "+str(list1))
         print("expect: "+str(list2))
         return False
   for x in list2:
      if x not in list1:
         print("actually: "+str(list1))
         print("expect: "+str(list2))
         return False
   return True
