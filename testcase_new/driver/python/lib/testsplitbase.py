from lib import testlib
from pysequoiadb import SDBError


class TestSplitBase(testlib.TestDataOprtBase):
   data_group = []
   groups = []

   def is_standalone(self):
      try:
         self.db.list_replica_groups()
         return False
      except SDBError as e:
         if e.code == -159:
            return True
         else:
            raise e

   def get_groups(self):
      if len(self.groups) > 0:
         return self.groups
      cur = self.db.list_replica_groups()
      r = self.get_records(cur)
      self.groups.extend(r)
      return self.groups

   def get_data_group(self):
      if len(self.data_group) > 0:
         return self.data_group
      if len(self.groups) == 0:
         self.get_groups()

      for x in self.groups:
         if x["GroupName"] != "SYSCatalogGroup" and x["GroupName"] != "SYSCoord":
            self.data_group.append(x)
      return self.data_group
