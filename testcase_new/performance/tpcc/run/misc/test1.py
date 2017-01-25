#!/bin/env python
import os
from os.path import join, getsize
for root, dirs, files in os.walk('/data/disk6/testdata'):
   a = root
   print root, "consumes",
   print "a=%s"%a
