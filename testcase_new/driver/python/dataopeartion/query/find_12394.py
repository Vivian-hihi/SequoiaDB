# @decription: find records with option FLAGS
# @author:     liuxiaoxuan 2017-8-29

import unittest
import datetime
from pysequoiadb import client
from pysequoiadb import collectionspace
from pysequoiadb.error import (SDBTypeError, SDBBaseError, SDBEndOfCursor,SDBError)
from lib.config import *

cs_name = "cs_12394"
cl_name = "cl_12394"
insert_nums = 100
class TestFind12394(unittest.TestCase):
    def setUp(self):
        try:
            print(datetime.datetime.now())
            config = Config()
            self.db = client( config.host_name, config.service )
            self.create_cl()
            self.insertDatas()                 
        except SDBBaseError as e:
            print(e.detail)
            raise e                     

    def testqueryAll(self):
        try:
            print("---begin to check data---") 
            i = 1
            cursor = self.cl.query(order_by={"_id":1} , flags=1)
            flag = True
            while True:
                try:
                    rec = cursor.next()
                    exprec = {"_id":i,"a":"test" + str(i)}              
                    if(rec != exprec):
                        flag = False
                        print(rec)  
                        print(exprec)
                        break
                    i = i + 1                       
                except SDBEndOfCursor:
                    break             
            self.assertEqual( flag,True)           

        except SDBBaseError as e:
            print(e.detail)
            assert False

    def testqueryOne(self):
        try:
            print("---begin to check data---")   
            rec = self.cl.query_one(condition={"_id":{'$gt':5}},flags=10)
            exprec = {"_id":6,"a":"test" + str(6)}
            self.assertEqual( rec,exprec) 

        except (SDBTypeError, SDBBaseError) as e:
            print(e)
            assert False

        except SDBBaseError as e:
            print(e.detail) 
            assert False           

    def tearDown(self):
        try:
            print(datetime.datetime.now())
            self.db.drop_collection_space(cs_name)
            self.db.disconnect()
        except SDBBaseError as e:
            if(-34 != e.code):
                print(e.detail)
                raise e        
    def create_cl(self):
        try:
            print( '---begin to create cs---')
            self.cs = self.db.create_collection_space(cs_name)            

            self.cl = self.cs.create_collection(cl_name)
            print( '---create cl success---' )   
        except SDBError as e:
            print(e.detail) 
            raise e    

    def insertDatas(self):   
        print( '---begin to insert records---' )
        for i in range(1,insert_nums):
            try:
                self.cl.insert({"_id":i,"a":"test" + str(i)})  
            except SDBError as e:
                print(e.detail) 
                raise e 
				
if __name__ == "__main__":
    unittest.main() 