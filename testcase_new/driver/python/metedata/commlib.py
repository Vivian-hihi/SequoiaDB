from pysequoiadb import client
from pysequoiadb import collectionspace
from pysequoiadb.error import (SDBTypeError, SDBBaseError, SDBEndOfCursor)
import unittest

def get_data_groups( db ):
   cursor = db.list_replica_groups()
   data_groups = []
   while True:
      try:
         record = cursor.next()
         group_name = record['GroupName']
         if(group_name != 'SYSCoord' and group_name != 'SYSCatalogGroup'):
            data_groups.append(group_name)       
      except SDBEndOfCursor:
         break
   return data_groups
   
