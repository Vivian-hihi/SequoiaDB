#   Copyright (C) 2012-2016 SequoiaDB Ltd.
#
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.

"""decimal supported for Python driver of SequoiaDB
"""
import bson
from bson.py3compat import binary_type as bstr
try:
   import bsondecimal as decimal
except:
   raise Exception("failed to import extension: decimal")

class Decimal(object):
   def __init__(self, precision = None, scale = None):
      if ( precision is None and scale is not None ) or ( precision is not None and scale is None ):
         raise TypeError("precision and scale should be set both or neither")
      if precision is not None and not isinstance(precision, int):
         raise TypeError("precision must be an instance of int")
      if scale is not None and not isinstance(scale, int):
         raise TypeError("scale must be an instance of int")

      _, self.__decimal = decimal.create()
      if _ != 0:
         raise Exception("cannot create decimal entity, out of memory")

      if precision is None and scale is None:
         _ = decimal.init(self.__decimal)
      else:
         _ = decimal.init2(self.__decimal, precision, scale)
      if 0 != _:
         raise Exception("invalid parameter, code: %d" % _)
      
   def __del__(self):
      decimal.destroy(self.__decimal)

   def __str__(self):
      return self.__to_json_string()

   def __repr__(self):
      return self.__to_json_string()

   def set_zero(self):
      _ = decimal.setZero(self.__decimal)
      if 0 != _:
         raise Exception("invalid parameter, code: %d" % _)

   def is_zero(self):
      _, zero_ = decimal.isZero(self.__decimal)
      if _ != 0:
         raise Exception("invalid parameter, code: %d" % _)
      return zero_

   def set_min(self):
      _ = decimal.setMin(self.__decimal)
      if _ != 0:
         raise Exception("invalid parameter, code: %d" % _)

   def is_min(self):
      _, min_ = decimal.isMin(self.__decimal)
      if _ != 0:
         raise Exception("invalid parameter, code: %d" % _)
      return min_

   def set_max(self):
      _ = decimal.setMax(self.__decimal)
      if _ != 0:
         raise Exception("invalid parameter, code: %d" % _)

   def is_max(self):
      _, max_, decimal.isMax(self.__decimal)
      if _ != 0:
         raise Exception("invalid parameter, code: %d" % _)
      return max_

   def __from_int(self, value):
      _ = decimal.fromInt(self.__decimal, value)
      if _ != 0:
         raise Exception("invalid parameter, code: %d" % _)

   def to_int(self):
      _, v = decimal.toInt(self.__decimal)
      if _ != 0:
         raise Exception("invalid parameter, code: %d" % _)
      return v

   def __from_float(self, value):
      _ = decimal.fromDouble(self.__decimal, value)
      if _ != 0:
         raise Exception("invalid parameter, code: %d" % _)

   def to_float(self):
      _, v = decimal.toFloat(self.__decimal)
      if _ != 0:
         raise Exception("invalid parameter, code: %d" % _)
      return v

   def __from_string(self, value):
      _ = decimal.fromString(self.__decimal, value)
      if _ != 0:
         raise Exception("invalid parameter, code: %d" % _)

   def to_string(self):
      _, v = decimal.toString(self.__decimal)
      if _ != 0:
         raise Exception("invalid parameter, code: %d" % _)
      return v

   def parse(self, value):
      if isinstance(value, int):
         return self.__from_int(value)
      elif isinstance(value, float):
         return self.__from_float(value)
      elif isinstance(value, bstr):
         return self.__from_string(value)
      else:
         raise TypeError("invalid value to parse")

   def __to_json_string(self):
      _, v = decimal.toJsonString(self.__decimal)
      if _ != 0:
         raise Exception("invalid parameter, code: %d" % _)
      return v

   def parse_from_bson_string(self, value):
      _, l = decimal.fromBsonValue(self.__decimal, value)
      if _ != 0:
         raise Exception("invalid parameter, code: %d" % _)
      return l

   def compare(self, rhs):
      if isinstance(rhs, int):
         _ = decimal.compareInt(self.__decimal, rhs)
      elif isinstance(rhs, Decimal):
         _ = decimal.compare(self.__decimal, rhs)
      else:
         raise TypeError('invalid comparation: between Decimal and %s' % type(rhs))

      if _ not in (-1, 0, 1):
         raise Exception("invalid return value or process failed, code %d" % _)
      return _

   def to_bson_element_value(self):
      _, s = decimal.toBsonElement(self.__decimal)
      if _ != 0:
         raise Exception("invalid parameter, code: %d" % _)
      return s
