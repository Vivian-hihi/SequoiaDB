from lib import testlib


class TestSplitBase(testlib.TestDataOprtBase):
   data_group= []
   groups = []
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
      if len(self.groups)==0:
         self.get_groups()

      for x in self.groups:
         if x["GroupName"] != "SYSCatalogGroup" and x["GroupName"] != "SYSCoord":
            self.data_group.append(x)
      return self.data_group
