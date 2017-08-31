from pysequoiadb import client
from pysequoiadb import collectionspace
from pysequoiadb.error import (SDBTypeError, SDBBaseError, SDBEndOfCursor)

def check_Result( cl, condition, selector, expect_result, expect_type, check_id ):
      actual_record_num = cl.get_count( condition = condition )
      expect_record_num = len(expect_result)
      if(actual_record_num != expect_record_num):
         print(actual_record_num)
         raise Exception( 'RECORD_COUNT_ERROR' )
         
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
      
      expect_type_num = len(expect_type)
      if(actual_record_num != expect_type_num):
         print(actual_record_num)
         raise Exception( 'TYPE_COUNT_ERROR' )
               
      cursor = cl.query( condition = condition, selector = selector )
      while True:
         try:
            record = cursor.next()
            if(not check_id):
               del record['_id']
            if( not (record in expect_type)):
               print(record)
               raise Exception( 'CHECK_TYPE_ERROR' )   
         except SDBEndOfCursor:
            break