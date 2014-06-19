class enum(object):
   '''
   >>> Sex = enum(((1,"FEMALE"),
    (2, "MALE")))
   >>> Sex.FEMALE
   1
   >>> tuple(Sex.choice_tuples())
   ((1,'FEMALE',(2,'MALE'))
   >>> tuple(Sex.available_options())
   (1,2)
   '''

   def __init__(self, config):
      self.config = config
      for item in config:
         setattr(self, item[1], item[0])

   def choice_tuples(self):
      return ((item[0],item[1]) for item in self.config)

   def available_options(self):
      return (item[0] for item in self.config)
