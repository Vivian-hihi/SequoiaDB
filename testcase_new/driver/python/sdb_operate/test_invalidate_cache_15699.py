# @decription: invalidate cache
# @testlink:   seqDB-15699
# @interface:  invalidate_cache(self,options=None)
# @author:     liuxiaoxuan 2018-09-26

from lib import testlib
from pysequoiadb.error import (SDBBaseError, SDBEndOfCursor)

class TestInvalidateCache15699(testlib.SdbTestBase):
   def setUp(self):
      # skip standalone
      if testlib.is_standalone():
         self.skipTest('run mode is standalone')
      
   def test_invalidateCache_15699(self):
      
      # invalidate coord and all nodes cache without options 
      self.db.invalidate_cache();

      # invalidate coord cache 
      self.db.invalidate_cache({"Groups" : None});

      # get groups 
      groups = testlib.get_data_groups()
      data_groupnames = list()
      for grp in groups:
         data_groupnames.append(grp['GroupName'])
			
	   # invalidate group string
      self.db.invalidate_cache({"Groups" : data_groupnames[0]});
		
		# invalidate group array
      self.db.invalidate_cache({"Groups" : data_groupnames});

   def tearDown(self):
      pass