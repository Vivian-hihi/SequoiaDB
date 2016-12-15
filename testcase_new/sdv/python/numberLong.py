# @decription: seqDB-9460:python_插入小于整型、插入大于整型的数
#              seqDB-7825:python_字符串插入和数值型插入记录
#              seqDB-7826:python_numberLong参数校验
#              seqDB-7827:python_numberLong边界值校验
#              seqDB-9461:python_查询最大安全整数
# @author:     Ting YU 2016-8-23

import pysequoiadb
from pysequoiadb import client
from pysequoiadb import collectionspace
from pysequoiadb import collection
from pysequoiadb import cursor
from pysequoiadb import const
from pysequoiadb.error import (SDBTypeError, SDBBaseError, SDBEndOfCursor)
from bson.json_util import loads 
from bson.json_util import dumps 
import sys,getopt
import traceback

hostname=None
service=None

# parase the args
def parse_option():

   if len( sys.argv ) < 2:
      usage()
      sys.exit(1)
   
   try:  
      opts, args = getopt.getopt( sys.argv[1:], 'hH:p:', ['help'] )
   except getopt.GetoptError, err:
      print str( err )
      usage()
      sys.exit(1)
   
   global hostname, service
   for op, value in opts:
      if op == '-H':
         hostname = value
      elif op == '-p':
         service = int(value)
      elif op in ('-h', '--help'):
         usage()
         sys.exit()
      else:
         print 'arguments error'
         usage()
         sys.exit(1)
         
def usage():
   print 'Command options:'
   print '-h,--help  help'
   print '-H   arg   hostname'
   print '-p   arg   coord_port'

def createCL( cs_name, cl_name):
   print '---begin to drop cs in ready'
   try:
      db.drop_collection_space(cs_name)
   except SDBBaseError, e:
      if ( -34 != e.code ):            
         raise e
                   
   print '---begin to create cs cl'
   cs = db.create_collection_space(cs_name)     
   cl = cs.create_collection(cl_name, {"ReplSize":0})
   
   return cl

def clean( cs_name ):
   print '---begin to drop cs in finally'
   try:
      db.drop_collection_space(cs_name)
   except SDBBaseError, e:
      if ( -34 != e.code ): 
         pysequoiadb._print(e.detail)
         raise e
            
def insert_boundary_max( cl ):   
   print '---begin to insert numberLong max: string "9223372036854775807" '
   
   cl.delete()
   cl.insert( loads('{"a":{"$numberLong":"9223372036854775807"},"_id":1}') )
   
   condition = {'a':{'$type':18}}
   rec = cl.query(condition=condition).next()
   cnt = cl.get_count(condition=condition) 
   
   if( cnt != 1 ):
      print 'get_count, expect: 1, actual: %d' % ( cnt )
      raise  Exception( 'COUNT_ERROR' )
      
   if( rec['a'] != 9223372036854775807 ):
      print 'rec["a"], expect: 9223372036854775807, actual: %d' % ( rec['a'] )     
      raise  Exception( 'CHECK_VALUE_ERROR' )
         
   if( dumps(rec) != '{"a": {"$numberLong": "9223372036854775807"}, "_id": 1}' ):
      print 'dumps(rec), expect: {"a": {"$numberLong": "9223372036854775807"}, "_id": 1}, actual: %s' % ( dumps(rec) )
      raise  Exception( 'CHECK_JSON_ERROR' ) 
 

def insert_boundary_min( cl ):
   print '---begin to insert numberLong min: number -9223372036854775808 '
   
   cl.delete()
   cl.insert( loads('{"a":{"$numberLong":-9223372036854775808},"_id":1}') )
   
   condition = {'a':{'$type':18}}
   rec = cl.query(condition=condition).next()
   cnt = cl.get_count(condition=condition) 
   
   if( cnt != 1 ):
      print 'get_count, expect: 1, actual: %d' % ( cnt )
      raise  Exception( 'COUNT_ERROR' )
      
   if( rec['a'] != -9223372036854775808 ):
      print 'rec["a"], expect: -9223372036854775808, actual: %d' % ( rec['a'] )     
      raise  Exception( 'CHECK_VALUE_ERROR' )
         
   if( dumps(rec) != '{"a": {"$numberLong": "-9223372036854775808"}, "_id": 1}' ):
      print 'dumps(rec), expect: {"a": {"$numberLong": "-9223372036854775808"}, "_id": 1}, actual: %s' % ( dumps(rec) )
      raise  Exception( 'CHECK_JSON_ERROR' ) 

def insert_int32( cl ):    
   print '---begin to insert record: value is int32 '
   
   cl.delete()
   cl.insert( loads('{"a":{"$numberLong":-9}}') )

   condition = {'a':{'$type':16}}
   cnt = cl.get_count(condition=condition)
   
   if( cnt != 1 ):
      print 'get_count, expect: 1, actual: %d' % ( cnt )
      raise  Exception( 'COUNT_ERROR' )
         
   condition = {'a':{'$type':18}}
   cnt = cl.get_count(condition=condition)
   
   if( cnt != 0 ):
      print 'get_count, expect: 0, actual: %d' % ( cnt )
      raise  Exception( 'COUNT_ERROR' )


def query_js_max_safe_integet( cl ):       
   print '---begin to check js max safe integer'
      
   cl.delete()
   cl.insert( loads('{"a":{"$numberLong":-9007199254740992},"_id":1}') )
      
   rec = cl.query().next()
         
   if( dumps(rec) != '{"a": {"$numberLong": "-9007199254740992"}, "_id": 1}' ):
      print 'dumps(rec), expect: {"a": {"$numberLong": "-9007199254740992"}, "_id": 1}, actual: %s' % ( dumps(rec) )
      raise  Exception( 'CHECK_JSON_ERROR' )
      
   cl.delete()
   cl.insert( loads('{"a":{"$numberLong":9007199254740992},"_id":1}') )
      
   rec = cl.query().next()
         
   if( dumps(rec) != '{"a": {"$numberLong": "9007199254740992"}, "_id": 1}' ):
      print 'dumps(rec), expect: {"a": {"$numberLong": "9007199254740992"}, "_id": 1}, actual: %s' % ( dumps(rec) )
      raise  Exception( 'CHECK_JSON_ERROR' )

   cl.delete()
   cl.insert( loads('{"a":{"$numberLong":-9007199254740991},"_id":1}') )
      
   rec = cl.query().next()
         
   if( dumps(rec) != '{"a": -9007199254740991, "_id": 1}' ):
      print 'dumps(rec), expect: {"a": -9007199254740991, "_id": 1}, actual: %s' % ( dumps(rec) )
      raise  Exception( 'CHECK_JSON_ERROR' )   
               
   cl.delete()
   cl.insert( loads('{"a":{"$numberLong":9007199254740991},"_id":1}') )
      
   rec = cl.query().next()
         
   if( dumps(rec) != '{"a": 9007199254740991, "_id": 1}' ):
      print 'dumps(rec), expect: {"a": 9007199254740991, "_id": 1}, actual: %s' % ( dumps(rec) )
      raise  Exception( 'CHECK_JSON_ERROR' )  
      
def insert_out_boundary( cl ):
   print '---begin to test out of boundary of numberlong '
   
   cl.delete()
   
   try:
      cl.insert( loads('{"a":{"$numberLong":"-9223372036854775809"}}') )
      raise  Exception( 'NOT_THROW_ERROR' )
   except OverflowError,e:
      print '   %s' % e
      
   try:
      cl.insert( loads('{"a":{"$numberLong":9223372036854775808}}') )
      raise  Exception( 'NOT_THROW_ERROR' )
   except OverflowError,e:
      print '   %s' % e
         
   cnt = cl.get_count()  
   if( cnt != 0 ):
      print 'get_count, expect: 0, actual: %d' % ( cnt )
      raise  Exception( 'COUNT_ERROR' )

def insert_parameter_check( cl ):
   print '---begin to check parameter '
   
   cl.delete()
   
   try:
      cl.insert( loads('{"a":{"$numberLong":"9.9"}}') )
      raise  Exception( 'NOT_THROW_ERROR' )
   except ValueError,e:
      print '   %s' % e
         
   cnt = cl.get_count()  
   if( cnt != 0 ):
      print 'get_count, expect: 0, actual: %d' % ( cnt )
      raise  Exception( 'COUNT_ERROR' )         
         
if __name__ == "__main__":   
   try:    
      parse_option()
      
      cs_name = "pydriver_numberLong"
      cl_name = "pydriver_numberLong"
            
      db = client( hostname, service )      
      cl = createCL( cs_name, cl_name)
      
      # main
      insert_boundary_max( cl )
      insert_boundary_min( cl )
      insert_int32( cl )     
      query_js_max_safe_integet( cl )
      insert_out_boundary( cl )
      insert_parameter_check( cl )
      
   except SDBBaseError, e:
      pysequoiadb._print(e.detail)
      raise e  
            
   finally:  
      if( locals().has_key('db') ):                    
         clean( cs_name )     
         db.disconnect()
         del db 