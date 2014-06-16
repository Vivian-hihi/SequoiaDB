"""
   Copyright (C) 2012-2014 SequoiaDB Ltd.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
"""

from distutils.core import setup, Extension
import sys
import os
import platform

pythonpath = sys.executable
pythondir = pythonpath[0:pythonpath.rfind( os.sep)]
pythondir += os.sep

print sys.platform
curdir=os.getcwd()
path = curdir + os.sep + ".." + os.sep \
       + ".." + os.sep + ".." + os.sep
       
if sys.platform == 'win32':
    thridpartypath = path + "thirdparty"+os.sep
    path = path + "SequoiaDB" + os.sep + "engine" + os.sep
    ModuleCPPLibFiles = [
        path + "client/clientcpp.cpp",
        path + "oss/oss.cpp",
        path + "oss/ossUtil.cpp",
        path + "oss/ossMem.cpp",
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
        path +  "util/utilStr.cpp",
        path + "bson/bsonobj.cpp",
        path + "bson/oid.cpp",
        path + "bson/base64.cpp",
        path + "bson/md5.c",
        path + "bson/nonce.cpp",
        'pyclient.cpp',
        'pycollection.cpp',
        'pycollectionspace.cpp',
        'pycursor.cpp',
        'pyreplicagroup.cpp',
        'pyreplicanode.cpp',]

    incdirs=[pythondir + 'include',
             path,
             path +'client',
             path + 'util',
             path+'bson/util',
             path+'bson',
             path + 'include',
             path+'client/bson',
             thridpartypath + 'boost/',]

    libdirs= [pythondir + 'lib',
              thridpartypath + 'boost/lib/win64',]
    precompilemacros = [('_CRT_RAND_S', 1),
                        ('UNICODE', 1),
                        ('_UNICODE', 1),
                        ('SDB_DLL_BUILD',1),
                        ('SDB_CLIENT',1),]
    compileopt= ['/EHsc',
                 '/W3',
                 '/MD',
                  '/MT',
                 '/errorReport:none']
    linklibs=['libboost_thread-vc100-mt-gd-1_49',
              'ws2_32',
              'kernel32',
              'advapi32',
              'Psapi']

else:
    path = path + "client" + os.sep + "lib"
    incdirs=[pythondir + 'include',]
    libdirs= [pythondir + 'lib',]
    ModuleCPPLibFiles = [ 'pyclient.cpp',
                          'pycollection.cpp',
                          'pycollectionspace.cpp',
                          'pycursor.cpp',
                          'pyreplicagroup.cpp',
                          'pyreplicanode.cpp',]
    precompilemacros = [('UNICODE', 1),
                        ('_UNICODE', 1),]
    compileopt= ['-shared',
                 '-fPIC',]
    linklibs=['staticsdbcpp',]

module1 = Extension('sequoiadbClient',
                    define_macros = precompilemacros,
                    extra_compile_args=compileopt,
                    include_dirs = incdirs,
                    libraries = linklibs,
                    library_dirs = libdirs,
                    sources = ModuleCPPLibFiles )

setup (name = 'sequoiadbClient',
       version = '1.0',
       description = 'This is a sequoiadb python driver use adapter package',
       url = 'http://www.sequoiadb.com',
       long_description = '''
              use sequoiadb python driver must install this module.
              ''',
       ext_modules = [module1])
