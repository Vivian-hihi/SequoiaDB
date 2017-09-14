# @decription: test aggregate
# @testlink:   seqDB-12480
# @interface:  aggregate(self,aggregate_options)
# @author:     liuxiaoxuan 2017-8-30

from bson.son import SON
from lib import testlib
from pysequoiadb.error import (SDBBaseError, SDBEndOfCursor, SDBError)

insert_nums = 10
class TestAggregate12480(testlib.SdbTestBase):
   def setUp(self):
      self.create_cs_cl()
      self.insert_datas()

   def testAggregate12480(self):
      match = SON({'$match': {'name': {'$exists': 1}}})
      group = SON({'$group': {'_id': '$major', 'avg_age': {'$avg': '$age'}, 'major': {'$first': '$major'}}})
      sort = SON({'$sort': {'avg_age': 1}})
      skip = {'$skip': 1}
      limit = {'$limit': 2}

      list_aggr_options = [match, group, sort, skip, limit]
      tuple_aggr_options = (match, group, sort, skip, limit)

      list_cursor = self.cl.aggregate(list_aggr_options)
      tuple_cursor = self.cl.aggregate(list_aggr_options)

      list_actResult = testlib.get_all_records(list_cursor)
      tuple_actResult = testlib.get_all_records(tuple_cursor)

      expectResult = [{'avg_age': 20.0, 'major': 'major5'}, \
                      {'avg_age': 21.0, 'major': 'major1'}]

      self.assertListEqualUnordered(expectResult,list_actResult)
      self.assertListEqualUnordered(expectResult,tuple_actResult)

   def tearDown(self):
      if self.should_clean_env():
         self.drop_cs()

   def insert_datas(self):
      for i in range(0, insert_nums):
         try:
            self.cl.insert({"_id": i, "name": "test" + str(i), "major": "major" + str(i % 10), "age": 20 + i % 5})
         except SDBError as e:
            self.fail('insert fail: ' + e.detail)

