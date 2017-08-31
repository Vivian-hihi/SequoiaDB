from pysequoiadb import client
from pysequoiadb import collectionspace
from pysequoiadb.error import (SDBTypeError, SDBBaseError, SDBEndOfCursor)

def check_Result( cl, condition, expect_result, check_id ):
      actual_record_num = cl.get_count( condition = condition )
      expect_record_num = len(expect_result)
      if(actual_record_num != expect_record_num):
         print(actual_record_num)
         raise Exception( 'COUNT_ERROR' )
         
      cursor = cl.query( condition = condition )
      while True:
         try:
            record = cursor.next()
            if(not check_id):
               del record['_id']
            if( not (record in expect_result)):
               print(record)
               raise Exception( 'CHECK_RECORD_ERROR' )       
         except SDBEndOfCursor:
            break