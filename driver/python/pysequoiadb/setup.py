#   Copyright (C) 2012-2014 SequoiaDB Ltd.
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

from distutils.core import setup, Extension
import sys
import os
import platform

pythonpath = sys.executable
pythondir = pythonpath[0:pythonpath.rfind(os.sep)]
pythondir += os.sep

print sys.platform
pwd = os.getcwd()
path = os.path.abspath(os.path.join(pwd, os.pardir, os.pardir, os.pardir)) + os.sep

if sys.platform == 'win32':
   thirdparty = path + "thirdparty" + os.sep
   path = path + "SequoiaDB" + os.sep + "engine" + os.sep
   source = [
      path + "client/clientcpp.cpp",
      path + "oss/oss.cpp",
      path + "oss/ossUtil.cpp",
      path + "oss/ossMem.cpp",
      path + "oss/ossVer.cpp",
      path + "oss/ossSocket.cpp",
      path + "oss/ossPrimitiveFileOp.cpp",
      path + "pd/pd.cpp",
      path + "pd/pdTrace.cpp",
      path + "pd/pdFunctionList.cpp",
      path + "client/bson/numbers.c",
      path + "client/bson/bson.c",
      path + "client/bson/encoding.c",
      path + "client/base64c.c",
      path + "client/cJSON.c",
      path + "client/jstobs.c",
      path + "client/common.c",
      path + "util/fromjson.cpp",
      path + "util/json2rawbson.c",
      path + "util/utilStr.cpp",
      path + "bson/bsonobj.cpp",
      path + "bson/oid.cpp",
      path + "bson/base64.cpp",
      path + "bson/md5.c",
      path + "bson/nonce.cpp", ]

   include = [
      path,
      pythondir + 'include',
      path + 'client',
      path + 'util',
      path + 'bson/util',
      path + 'bson',
      path + 'include',
      path + 'client/bson',
      thirdparty + 'boost/', ]

   lib = [
      pythondir + 'lib',
      thirdparty + 'boost/lib/win64', ]

   compile = [
      ('_CRT_RAND_S', 1),
      ('UNICODE', 1),
      ('_UNICODE', 1),
      ('SDB_DLL_BUILD',1),
      ('SDB_CLIENT',1), ]

   compile_options = [
      '/EHsc',
      '/W3',
      '/MD',
      '/MT',
      '/errorReport:none', ]

   link = [
      'libboost_thread-vc100-mt-gd-1_49',
      'ws2_32',
      'kernel32',
      'advapi32',
      'Psapi', ]

else:
   include = [
      path + 'client/include',
      path + 'SequoiaDB/engine/include', ]

   lib = [
      path + 'client/lib', ]

   source = [
      path + "SequoiaDB/engine/oss/ossVer.cpp", ]

   compile = [
      ('UNICODE', 1),
      ('_UNICODE', 1), ]

   compile_options = [
      '-shared',
      '-fPIC',
      '-ldl',
      '-g', ]

   link = [
      'staticsdbcpp', ]


module1cppfiles = source;
module1cppfiles.append('pyclient.cpp')
module1 = Extension( 'sdbclient',
                     define_macros      = compile,
                     extra_compile_args = compile_options,
                     include_dirs       = include,
                     libraries          = link,
                     library_dirs       = lib,
                     sources            = module1cppfiles )

module2cppfiles=source;
module2cppfiles.append('pycollection.cpp')
module2 = Extension( 'sdbcl',
                     define_macros      = compile,
                     extra_compile_args =compile_options,
                     include_dirs       = include,
                     libraries          = link,
                     library_dirs       = lib,
                     sources            = module2cppfiles )

module3cppfiles=source;
module3cppfiles.append('pycollectionspace.cpp')
module3 = Extension( 'sdbcs',
                     define_macros      = compile,
                     extra_compile_args = compile_options,
                     include_dirs       = include,
                     libraries          = link,
                     library_dirs       = lib,
                     sources            = module3cppfiles )

module4cppfiles=source;
module4cppfiles.append('pyreplicagroup.cpp')
module4 = Extension( 'sdbreplicagroup',
                     define_macros      = compile,
                     extra_compile_args = compile_options,
                     include_dirs       = include,
                     libraries          = link,
                     library_dirs       = lib,
                     sources            = module4cppfiles )

module5cppfiles=source;
module5cppfiles.append('pyreplicanode.cpp')
module5 = Extension( 'sdbreplicanode',
                     define_macros      = compile,
                     extra_compile_args = compile_options,
                     include_dirs       = include,
                     libraries          = link,
                     library_dirs       = lib,
                     sources            = module5cppfiles )

module6cppfiles=source;
module6cppfiles.append('pycursor.cpp')
module6 = Extension( 'sdbcursor',
                     define_macros      = compile,
                     extra_compile_args = compile_options,
                     include_dirs       = include,
                     libraries          = link,
                     library_dirs       = lib,
                     sources            = module6cppfiles )

ext_modules = [
            module1,
            module2,
            module3,
            module4,
            module5,
            module6, ]

extra_opts = {}
extra_opts['ext_modules'] = ext_modules
setup(name = 'sdbclient',
      version = '1.0',
      author = 'SequoiaDB Inc.',
      description = 'This is a sequoiadb python driver use adapter package',
      url = 'http://www.sequoiadb.com',
      long_description = '''
           use sequoiadb python driver must install this module.
           ''',
      **extra_opts)
