# -- coding: utf-8 --
# @decription: lob opeartion
# @testlink:   seqDB-12478
# @author:     LaoJingTang 2017-8-
30
import sys

from pysequoiadb import SDBInvalidArgument
from pysequoiadb import SDBTypeError
from pysequoiadb.lob import LOB_WRITE
from lib import testlib
from lob import util

class LobRandoWrite(testlib.SdbTestBase):
   def setUp(self):
      testlib.drop_cs(self.db, self.cs_name, ignore_not_exist=True)
      self.cs = self.db.create_collection_space(self.cs_name)
      self.cl = self.cs.create_collection(self.cl_name)
      self.data = util.random_str(1024)
      self.expect_data = self.data.encode()

   def tearDown(self):
      if self.should_clean_env():
         self.db.drop_collection_space(self.cs_name)

   def test_seek_write_lob_13403(self):
      lob = self.cl.create_lob()
      lob_id = lob.get_oid()
      lob.seek(10, 0)
      lob.write(self.data, 1024)
      lob.close()

      lob = self.cl.open_lob(lob_id, LOB_WRITE)
      lob.seek(2000, 0)
      lob.write(self.data, 1024)
      lob.close()

      lob = self.cl.open_lob(lob_id)
      lob.seek(10, 0)
      actual = lob.read(1024)
      self.assertEqual(actual, self.expect_data)

      lob.seek(2000, 0)
      actual = lob.read(1024)
      self.assertEqual(actual, self.expect_data)

   def test_lock_write_lob_13404(self):
      oid = self.__create_empty_lob()

      lob = self.cl.open_lob(oid, LOB_WRITE)
      lob.lock(0, 1024)
      lob.write(self.data, 1024)
      lob.close()

      lob = self.cl.open_lob(oid, LOB_WRITE)
      lob.lock_and_seek(1024, 1024)
      lob.write(self.data, 1024)
      lob.close()

      lob = self.cl.open_lob(oid)
      actual = lob.read(1024)
      self.assertEqual(actual, self.expect_data)
      actual = lob.read(1024)
      self.assertEqual(actual, self.expect_data)

   def test_lob_truncate_13409(self):
      oid = self.__create_and_write_lob(self.data)
      self.cl.truncate_lob(oid, 0)
      lob = self.cl.open_lob(oid)
      self.assertEqual(lob.get_size(), 0)
      lob.close()

      oid = self.__create_and_write_lob(self.data)
      self.cl.truncate_lob(oid, 100)
      self.assertEqual(self.__read_lob(oid), (self.data[0:100]).encode())

      oid = self.__create_and_write_lob(self.data)
      self.cl.truncate_lob(oid, 10000)
      self.assertEqual(self.__read_lob(oid), self.expect_data)


   def test_lob_oid(self):
      with self.assertRaises(SDBInvalidArgument):
         self.cl.truncate_lob("xxx",10)
      with self.assertRaises(SDBInvalidArgument):
         self.cl.remove_lob("xxx")
      with self.assertRaises(SDBInvalidArgument):
         self.cl.open_lob("xxx")
      with self.assertRaises(SDBInvalidArgument):
         self.cl.get_lob("xxx")
      with self.assertRaises(SDBTypeError):
         self.cl.create_lob("xxx")



   def test_lock(self):
      lob = self.cl.create_lob()
      lob.lock(0, 1024)
      lob.lock(sys.maxsize, 1024)
      lob.lock(0, -1);

      with self.assertRaises(SDBInvalidArgument):
         lob.lock(0, -2);

      with self.assertRaises(SDBInvalidArgument):
         lob.lock(0, 0)

      with self.assertRaises(SDBTypeError):
         lob.lock("1", "1")

   def test_lockandseek(self):
      lob = self.cl.create_lob()
      lob.lock_and_seek(0, -1)
      lob.lock_and_seek(sys.maxsize, -1)
      with self.assertRaises(SDBInvalidArgument):
         lob.lock_and_seek(-1, -1)
      with self.assertRaises(SDBTypeError):
         lob.lock_and_seek("1", -1)

      lob.lock_and_seek(0, 1)
      with self.assertRaises(SDBInvalidArgument):
         lob.lock_and_seek(0, 0)
      with self.assertRaises(SDBInvalidArgument):
         lob.lock_and_seek(0, -2)
      with self.assertRaises(SDBTypeError):
         lob.lock_and_seek(0, "1")

   def test_get_modification_time(self):
      lob = self.cl.create_lob()
      lob.write(self.data, 1024)
      lob.close()
      t1 = lob.get_modification_time()

      oid = lob.get_oid()
      import time
      time.sleep(1)
      lob = self.cl.open_lob(oid, LOB_WRITE)
      lob.seek(0, 0)
      lob.write(self.data, 100)
      lob.close()
      t2 = lob.get_modification_time()

      self.assertTrue(t2 > t1)

   def __create_empty_lob(self):
      lob = self.cl.create_lob()
      lob.close()
      return lob.get_oid()

   def __create_and_write_lob(self,lob_data):
      lob = self.cl.create_lob()
      lob.write(lob_data, len(lob_data))
      lob.close()
      return lob.get_oid()

   def __read_lob(self, oid):
      lob = self.cl.open_lob(oid)
      return lob.read(lob.get_size())
