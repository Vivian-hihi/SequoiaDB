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

from distutils.core import Extension, setup
import sys
import os
import platform

pythonpath = sys.executable
pythondir = pythonpath[0:pythonpath.rfind(os.sep)]
pythondir += os.sep

pwd = os.getcwd()
path = os.path.abspath(os.path.join(pwd, os.pardir, os.pardir)) + os.sep

windows = False

if sys.platform == 'win32':
   windows = True
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
      path + 'client/CPP/include',
      path + 'SequoiaDB/engine',
      path + 'SequoiaDB/engine/include',
      path + 'SequoiaDB/engine/client',
      path + 'SequoiaDB/engine/bson',
      path + 'SequoiaDB/engine/util',
      path + 'thirdparty/boost', ]

   lib = [
      path + 'client/lib',
      path + 'thirdparty/boost/lib', ]

   source = [
      path + "SequoiaDB/engine/oss/ossVer.cpp", ]

   compile = [
      ('UNICODE', 1),
      ('_UNICODE', 1), ]

   compile_options = [
      '-shared',
      '-fPIC',
      '-O3', ]

   link = [
      'staticsdbcpp', ]


module1cppfiles = source;
module1cppfiles.append('pysequoiadb/pyclient.cpp')
module1 = Extension( 'pysequoiadb.sdbclient',
                     define_macros      = compile,
                     extra_compile_args = compile_options,
                     include_dirs       = include,
                     libraries          = link,
                     library_dirs       = lib,
                     sources            = module1cppfiles )

module2cppfiles=source;
module2cppfiles.append('pysequoiadb/pycollection.cpp')
module2 = Extension( 'pysequoiadb.sdbcl',
                     define_macros      = compile,
                     extra_compile_args = compile_options,
                     include_dirs       = include,
                     libraries          = link,
                     library_dirs       = lib,
                     sources            = module2cppfiles )

module3cppfiles=source;
module3cppfiles.append('pysequoiadb/pycollectionspace.cpp')
module3 = Extension( 'pysequoiadb.sdbcs',
                     define_macros      = compile,
                     extra_compile_args = compile_options,
                     include_dirs       = include,
                     libraries          = link,
                     library_dirs       = lib,
                     sources            = module3cppfiles )

module4cppfiles=source;
module4cppfiles.append('pysequoiadb/pyreplicagroup.cpp')
module4 = Extension( 'pysequoiadb.sdbreplicagroup',
                     define_macros      = compile,
                     extra_compile_args = compile_options,
                     include_dirs       = include,
                     libraries          = link,
                     library_dirs       = lib,
                     sources            = module4cppfiles )

module5cppfiles=source;
module5cppfiles.append('pysequoiadb/pyreplicanode.cpp')
module5 = Extension( 'pysequoiadb.sdbreplicanode',
                     define_macros      = compile,
                     extra_compile_args = compile_options,
                     include_dirs       = include,
                     libraries          = link,
                     library_dirs       = lib,
                     sources            = module5cppfiles )

module6cppfiles=source;
module6cppfiles.append('pysequoiadb/pycursor.cpp')
module6 = Extension( 'pysequoiadb.sdbcursor',
                     define_macros      = compile,
                     extra_compile_args = compile_options,
                     include_dirs       = include,
                     libraries          = link,
                     library_dirs       = lib,
                     sources            = module6cppfiles )

module_bson = Extension( 'bson._cbson',
                         include_dirs       = ['bson'],
                         sources            = ['bson/_cbsonmodule.c',
                                               'bson/time64.c',
                                               'bson/buffer.c',
                                               'bson/encoding_helpers.c'] )

ext_modules = [
            module1,
            module2,
            module3,
            module4,
            module5,
            module6,
            module_bson, ]

extra_opts = {}
extra_opts['packages'] = [ 'bson', 'pysequoiadb']
extra_opts['package_dir']={ 'pysequoiadb':'pysequoiadb', 'bson':'bson'}
extra_opts['package_data'] = { 'pysequoiadb':['err.prop'],
                               'bson':[ 'buffer.h',
                                        'buffer.c',
                                        '_cbsonmodule.h',
                                        '_cbsonmodule.c',
                                        'encoding_helpers.h',
                                        'encoding_helpers.c',
                                        'time64.h',
                                        'time64.c',
                                        'time64_config.h',
                                        'time64_limits.h', ],}
extra_opts['ext_modules'] = ext_modules
setup(name = 'pysequoiadb',
      version = '1.0',
      author = 'SequoiaDB Inc.',
      license = 'AGPL',
      description = 'This is a sequoiadb python driver use adapter package',
      url = 'http://www.sequoiadb.com',
      **extra_opts)
