# -*- mode: python; -*-
# build file for SequoiaDB
# this requires scons
# you can get from http://www.scons.org
# then just type scons

# some common tasks
#   build 64-bit mac and pushing to s3
#      scons --64 s3dist

# This file, SConstruct, configures the build environment, and then delegates to
# several, subordinate SConscript files, which describe specific build rules.

EnsureSConsVersion( 1, 1, 0 )

import platform
import os
import sys
import imp
import types
import re
import shutil
import urllib
import urllib2
import buildscripts
import buildscripts.bb
import stat
from buildscripts import utils
from os.path import join, dirname, abspath
import libdeps
root_dir = dirname(File('SConstruct').rfile().abspath)
client_dir = join(root_dir,'client')
clientlib_dir = join(client_dir,'lib')
db_dir = join(root_dir,'SequoiaDB')
engine_dir = join(db_dir,'engine')
boost_dir = join(root_dir, 'boost')
boost_lib_dir = join(boost_dir, 'lib')
parser_dir = join(root_dir, 'parser' )
sm_dir = join(parser_dir, 'sm')
js_dir = join(sm_dir, 'js')
pcre_dir = join(engine_dir,'pcre')
crypto_dir = join(root_dir, 'crypto')
ssl_dir = join(crypto_dir, 'openssl-1.0.1c')
curl_dir = join(root_dir, 'curl')
gtest_dir = join(engine_dir,'gtest')
driver_dir = join(db_dir,'driver')
buildscripts.bb.checkOk()
# --- options ----

options = {}

options_topass = {}

def GuessOS():
	id = platform.system()
        print('id=' + id ) ;
	if id == 'Linux':
		return 'linux'
	elif id == 'Windows' or id == 'Microsoft':
		return 'win32'
	else:
		return None


def GuessArch():
	id = platform.machine()
	id = id.lower()
	if (not id) or (not re.match('(x|i[3-6])86$', id) is None):
		return 'ia32'
	elif id == 'i86pc':
		return 'ia32'
	elif id == 'x86_64':
		return 'ia64'
	elif id == 'amd64':
		return 'ia64'
        elif id == 'ppc64':
                return 'ppc64'
	else:
		return None

guess_os = GuessOS()
guess_arch = GuessArch()

def add_option( name, help , nargs , contibutesToVariantDir , dest=None ):

    if dest is None:
        dest = name

    AddOption( "--" + name , 
               dest=dest,
               type="string",
               nargs=nargs,
               action="store",
               help=help )

    options[name] = { "help" : help ,
                      "nargs" : nargs , 
                      "contibutesToVariantDir" : contibutesToVariantDir ,
                      "dest" : dest } 

def get_option( name ):
    return GetOption( name )

def _has_option( name ):
    x = get_option( name )
    if x is None:
        return False

    if x == False:
        return False

    if x == "":
        return False

    return True

def has_option( name ):
    x = _has_option(name)

    if name not in options_topass:
        # if someone already set this, don't overwrite
        options_topass[name] = x

    return x


def get_variant_dir():
    
    a = []
    
    for name in options:
        o = options[name]
        if not has_option( o["dest"] ):
            continue
        if not o["contibutesToVariantDir"]:
            continue
        
        if o["nargs"] == 0:
            a.append( name )
        else:
            x = get_option( name )
            x = re.sub( "[,\\\\/]" , "_" , x )
            a.append( name + "_" + x )
            
    s = "#build/${PYSYSPLATFORM}/"

    if len(a) > 0:
        a.sort()
        s += "/".join( a ) + "/"
    else:
        s += "normal/"
    return s

# installation options
add_option( "all", "install all", 0, False)
add_option( "engine", "install engine", 0, False)
add_option( "tool", "install tools", 0, False)
add_option( "testcase", "install testcases", 0, False)
add_option( "shell", "install shell", 0, False)
add_option( "client", "install client environment", 0, False)
add_option( "fmp", "install fmp", 0, False)


add_option( "language" , "description language" , 1 , False )
	
# installation/packaging
add_option( "prefix" , "installation prefix" , 1 , False )
add_option( "distname" , "dist name (0.8.0)" , 1 , False )
add_option( "distmod", "additional piece for full dist name" , 1 , False )
add_option( "nostrip", "do not strip installed binaries" , 0 , False )

add_option( "sharedclient", "build a libmongoclient.so/.dll" , 0 , False )
add_option( "full", "include client and headers when doing scons install", 0 , False )

# linking options
add_option( "release" , "release build" , 0 , True )
add_option( "static" , "fully static build" , 0 , True )

# base compile flags
add_option( "64" , "whether to force 64 bit" , 0 , True , "force64" )
add_option( "32" , "whether to force 32 bit" , 0 , True , "force32" )

add_option( "cxx", "compiler to use" , 1 , True )
add_option( "cc", "compiler to use for c" , 1 , True )

add_option( "cpppath", "Include path if you have headers in a nonstandard directory" , 1 , True )
add_option( "libpath", "Library path if you have libraries in a nonstandard directory" , 1 , True )

add_option( "extrapath", "comma separated list of add'l paths  (--extrapath /opt/foo/,/foo) static linking" , 1 , True )
add_option( "extrapathdyn", "comma separated list of add'l paths  (--extrapath /opt/foo/,/foo) dynamic linking" , 1 , True )
add_option( "extralib", "comma separated list of libraries  (--extralib js_static,readline" , 1 , True )
add_option( "staticlib", "comma separated list of libs to link statically (--staticlib js_static,boost_program_options-mt,..." , 1 , True )
add_option( "staticlibpath", "comma separated list of dirs to search for staticlib arguments" , 1 , True )

add_option( "boost-compiler", "compiler used for boost (gcc41)" , 1 , True , "boostCompiler" )
add_option( "boost-version", "boost version for linking(1_38)" , 1 , True , "boostVersion" )

add_option( "no-glibc-check" , "don't check for new versions of glibc" , 0 , False )

# experimental features
add_option( "mm", "use main memory instead of memory mapped files" , 0 , True )
add_option( "ssl" , "Enable SSL" , 0 , True )

# dev options
add_option( "d", "debug build no optimization, etc..." , 0 , True , "debugBuild" )
add_option( "dd", "debug build no optimization, additional debug logging, etc..." , 0 , True , "debugBuildAndLogging" )
add_option( "noscreenout", "do not send anything to screen", 0, True )
add_option( "durableDefaultOn" , "have durable default to on" , 0 , True )
add_option( "durableDefaultOff" , "have durable default to off" , 0 , True )

# debugging/profiling help

add_option( "gdbserver" , "build in gdb server support" , 0 , True )

add_option("smokedbprefix", "prefix to dbpath et al. for smoke tests", 1 , False )

add_option( "use-system-pcre", "use system version of pcre library", 0, True )

add_option( "use-system-all" , "use all system libraries", 0 , True )

add_option( "use-cpu-profiler",
            "Link against the google-perftools profiler library",
            0, True )       
            
            
# don't run configure if user calls --help
if GetOption('help'):
    Return()

# --- environment setup ---

variantDir = get_variant_dir()
clientCppVariantDir = variantDir + "clientcpp"
clientCVariantDir = variantDir + "clientc"
shellVariantDir = variantDir + "shell"
toolVariantDir = variantDir + "tool"
fmpVariantDir = variantDir + "fmp"
driverDir = variantDir + "driver/"

def removeIfInList( lst , thing ):
    if thing in lst:
        lst.remove( thing )

def printLocalInfo():
    import sys, SCons
    print( "scons version: " + SCons.__version__ )
    print( "python version: " + " ".join( [ `i` for i in sys.version_info ] ) )

printLocalInfo()

boostLibs = [ "thread" , "filesystem", "program_options", "system" ]

nix = False
linux = False
linux64  = False
windows = False
force64 = has_option( "force64" )
msarch = None
if force64:
    msarch = "amd64"

release = True
debugBuild = False
debugLogging = False

force32 = has_option( "force32" ) 
release = has_option( "release" )
static = False

debugBuild = has_option( "debugBuild" ) or has_option( "debugBuildAndLogging" ) 
debugLogging = has_option( "debugBuildAndLogging" )

# if neither release/debugBuild specified, by default using release
# if both release/debugBuild specified, by defaul use debugBuild
if not release and not debugBuild:
	release = True
	debugBuild = False
elif release and debugBuild:
  release = False
  debugBuild = True

env = Environment( BUILD_DIR=variantDir,
                   MSVS_ARCH=msarch ,
                   tools=["default", "gch", "jsheader", "mergelib" ],
                   PYSYSPLATFORM=os.sys.platform,

                   PCRE_VERSION='7.4',
                   )


libdeps.setup_environment( env )

if env['PYSYSPLATFORM'] == 'linux3':
    env['PYSYSPLATFORM'] = 'linux2'

if os.sys.platform == 'win32':
    env['OS_FAMILY'] = 'win'
else:
    env['OS_FAMILY'] = 'posix'

if has_option( "cxx" ):
    env["CC"] = get_option( "cxx" )
    env["CXX"] = get_option( "cxx" )

if has_option( "cc" ):
    env["CC"] = get_option( "cc" )
	

if env['PYSYSPLATFORM'] == 'linux2':
    env['LINK_LIBGROUP_START'] = '-Wl,--start-group'
    env['LINK_LIBGROUP_END'] = '-Wl,--end-group'
    env['RELOBJ_LIBDEPS_START'] = '--whole-archive'
    env['RELOBJ_LIBDEPS_END'] = '--no-whole-archive'
    env['RELOBJ_LIBDEPS_ITEM'] = ''

env["LIBPATH"] = []

if has_option( "libpath" ):
    env["LIBPATH"] = [get_option( "libpath" )]

if has_option( "cpppath" ):
    env["CPPPATH"] = [get_option( "cpppath" )]

if has_option( "noscreenout" ):
    env.Append( CPPDEFINES=[ "_NOSCREENOUT" ] )

if has_option( "durableDefaultOn" ):
    env.Append( CPPDEFINES=[ "_DURABLEDEFAULTON" ] )

if has_option( "durableDefaultOff" ):
    env.Append( CPPDEFINES=[ "_DURABLEDEFAULTOFF" ] )

hasEngine = has_option( "engine" )
hasClient = has_option( "client" )
hasTestcase = False
hasTool = has_option( "tool" )
hasShell = has_option( "shell" )
hasFmp = has_option("fmp")
hasAll = has_option( "all" )
if hasAll:
   hasEngine = True
   hasClient = True
   hasTestcase = True
   hasTool = True
   hasShell = True
   hasFmp = True
elif not ( hasEngine or hasClient or hasTestcase or hasTool or hasFmp ):
   hasEngine = True
   hasClient = True
   hasShell = True

boostCompiler = GetOption( "boostCompiler" )
if boostCompiler is None:
    boostCompiler = ""
else:
    boostCompiler = "-" + boostCompiler

boostVersion = GetOption( "boostVersion" )
if boostVersion is None:
    boostVersion = ""
else:
    boostVersion = "-" + boostVersion

usesm = True

extraLibPlaces = []

env['EXTRACPPPATH'] = []
env['EXTRALIBPATH'] = []

def addExtraLibs( s ):
    for x in s.split(","):
        env.Append( EXTRACPPPATH=[ x + "/include" ] )
        env.Append( EXTRALIBPATH=[ x + "/lib" ] )
        env.Append( EXTRALIBPATH=[ x + "/lib64" ] )
        extraLibPlaces.append( x + "/lib" )

if has_option( "extrapath" ):
    addExtraLibs( GetOption( "extrapath" ) )

if has_option( "extrapathdyn" ):
    addExtraLibs( GetOption( "extrapathdyn" ) )

if has_option( "extralib" ):
    for x in GetOption( "extralib" ).split( "," ):
        env.Append( LIBS=[ x ] )

class InstallSetup:
    binaries = False
    clientSrc = False
    headers = False
    bannerFiles = tuple()
    headerRoot = "include"

    def __init__(self):
        self.default()
    
    def default(self):
        self.binaries = True
        self.libraries = False
        self.clientSrc = False
        self.headers = False
        self.bannerFiles = tuple()
        self.headerRoot = "include"
        self.clientTestsDir = None

    def justClient(self):
        self.binaries = False
        self.libraries = False
        self.clientSrc = True
        self.headers = True
        self.bannerFiles = [ "#distsrc/client/LICENSE.txt",
                             "#distsrc/client/SConstruct" ]
        self.headerRoot = ""

installSetup = InstallSetup()

if has_option( "full" ):
    installSetup.headers = True
    installSetup.libraries = True

# ---- other build setup -----

platform = os.sys.platform
if "uname" in dir(os):
    processor = os.uname()[4]
else:
    processor = "i386"

if force32:
    processor = "i386"
if force64:
    processor = "x86_64"

env['PROCESSOR_ARCHITECTURE'] = processor

DEFAULT_INSTALL_DIR = "/usr/local"
installDir = DEFAULT_INSTALL_DIR
nixLibPrefix = "lib"

distName = GetOption( "distname" )
dontReplacePackage = False

if has_option( "prefix" ):
    installDir = GetOption( "prefix" )


def findVersion( root , choices ):
    if not isinstance(root, list):
        root = [root]
    for r in root:
        for c in choices:
            if ( os.path.exists( r + c ) ):
                return r + c
    raise RuntimeError("can't find a version of [" + repr(root) + "] choices: " + repr(choices))

def choosePathExist( choices , default=None):
    for c in choices:
        if c != None and os.path.exists( c ):
            return c
    return default

def filterExists(paths):
    return filter(os.path.exists, paths)

# add database include, boost include here
env.Append( CPPPATH=[join(engine_dir,'include'),join(ssl_dir,'include'),join(gtest_dir,'include'),join(curl_dir,'include'),pcre_dir, boost_dir] )
env.Append( CPPDEFINES=["__STDC_LIMIT_MACROS", "HAVE_CONFIG_H"] )
env.Append( CPPDEFINES=[ "SDB_DLL_BUILD" ] )
# specify dependent libraries for javascript engine and boost
if guess_os == "linux":
    linux = True
    platform = "linux"

    env.Append( LIBS=['m'] )
    env.Append( LIBS=['dl'] )
    # 64 bit linux
    if guess_arch == "ia64" and not force32:
        linux64 = True
        nixLibPrefix = "lib64"
        #env.Append( EXTRALIBPATH="/usr/lib64" )
        env.Append( EXTRALIBPATH="/lib64" )
        # use project-related boost library
        env.Append( EXTRALIBPATH=join(boost_lib_dir,'linux64') )
        # use project-related ssl library
        env.Append( EXTRALIBPATH=join(ssl_dir,'lib/linux64') )
        # use project-related curl library
        env.Append( EXTRALIBPATH=join(curl_dir,'lib/linux64') )
        # use project-related spidermonkey library
        if usesm:
            if debugBuild:
                smlib_dir = join(js_dir,'lib/debug/linux64/lib')
                env.Append( CPPPATH=join(js_dir,'lib/debug/linux64/include') )
                env.Append( EXTRALIBPATH=[smlib_dir] )
            else:
                smlib_dir = join(js_dir,'lib/release/linux64/lib')
                env.Append( CPPPATH=join(js_dir,'lib/release/linux64/include') )
                env.Append( EXTRALIBPATH=[smlib_dir] )
        ssllib_dir = join(ssl_dir,'lib/linux64')
        curllib_dir = join(curl_dir,'lib/linux64')
        env.Append( LIBS=["pthread"] )
        force64 = False
    # in case for 32 bit linux
    elif guess_arch == "ia32":
        linux64 = False
        nixLibPrefix = "lib"
        env.Append( EXTRALIBPATH="/usr/lib" )
        env.Append( EXTRALIBPATH="/lib" )
        # we want 32 bit boost library
        env.Append( EXTRALIBPATH=join(boost_lib_dir,'linux32') )
        # use project-related ssl library
        env.Append( EXTRALIBPATH=join(ssl_dir,'lib/linux32') )
        # use project-related curl library
        env.Append( EXTRALIBPATH=join(curl_dir,'lib/linux32') )
        # and 32 bit spidermonkey library
        if usesm:
            if debugBuild:
                smlib_dir = join(js_dir,'lib/debug/linux32/lib')
                env.Append( CPPPATH=join(js_dir,'lib/debug/linux32/include') )
                env.Append( EXTRALIBPATH=[smlib_dir] )
            else:
                smlib_dir = join(js_dir,'lib/release/linux32/lib')
                env.Append( CPPPATH=join(js_dir,'lib/release/linux32/include') )
                env.Append( EXTRALIBPATH=[smlib_dir] )
                # if we are in 64 bit box but want to build 32 bit release
        ssllib_dir = join(ssl_dir,'lib/linux32')
        curllib_dir = join(curl_dir,'lib/linux32')
    elif guess_arch == "ia64" and force32:
        # let's use 32 bit boost library
        env.Append( EXTRALIBPATH=["/usr/lib32", join(boost_lib_dir,'linux32')] )
        env.Append( LIBS=["pthread"] )
        # use project-related ssl library
        env.Append( EXTRALIBPATH=join(ssl_dir,'lib/linux32') )
        #use project-related curl library
        env.Append( EXTRALIBPATH=join(curl_dir,'lib/linux32') )
        # and use 32 bit spider monkey
        if usesm:
            if debugBuild:
                smlib_dir = join(js_dir,'lib/debug/linux32/lib')
                env.Append( CPPPATH=join(js_dir,'lib/debug/linux32/include') )
                env.Append( EXTRALIBPATH=[smlib_dir] )
            else:
                smlib_dir = join(js_dir,'lib/release/linux32/lib')
                env.Append( CPPPATH=join(js_dir,'lib/release/linux32/include') )
                env.Append( EXTRALIBPATH=join(js_dir,'lib/release/linux32/lib') )
        ssllib_dir = join(ssl_dir,'lib/linux32')
        curllib_dir = join(curl_dir,'lib/linux32')
    elif guess_arch == "ppc64" and not force32:
        linux64 = True
        nixLibPrefix = "lib64"
        env.Append( CPPDEFINES=[ "SDB_BIG_ENDIAN" ] )
        #env.Append( EXTRALIBPATH="/usr/lib64" )
        env.Append( EXTRALIBPATH="/lib64" )
        # use project-related boost library
        env.Append( EXTRALIBPATH=join(boost_lib_dir,'ppclinux64') )
        # use project-related ssl library
        env.Append( EXTRALIBPATH=join(ssl_dir,'lib/ppclinux64') )
        # use project-related curl library
        env.Append( EXTRALIBPATH=join(curl_dir,'lib/ppclinux64') )
        # use project-related spidermonkey library
        if usesm:
            if debugBuild:
                smlib_dir = join(js_dir,'lib/debug/ppclinux64/lib')
                env.Append( CPPPATH=join(js_dir,'lib/debug/ppclinux64/include') )
                env.Append( EXTRALIBPATH=[smlib_dir] )
            else:
                smlib_dir = join(js_dir,'lib/release/ppclinux64/lib')
                env.Append( CPPPATH=join(js_dir,'lib/release/ppclinux64/include') )
                env.Append( EXTRALIBPATH=[smlib_dir] )
        ssllib_dir = join(ssl_dir,'lib/ppclinux64')
        curllib_dir = join(curl_dir,'lib/ppclinux64')
        env.Append( LIBS=["pthread"] )
        force64 = False

    if usesm:
        smlib_file = join(smlib_dir, 'libmozjs185.so')
        env.Append( CPPDEFINES=[ "XP_UNIX" ] )
        env.Append( LIBS=['js_static'] )
    env.Append( LIBS=['crypto'] )
    ssllib_file = join(ssllib_dir, 'libcrypto.a')
    ssllib_file1 = join(ssllib_dir, 'libcrypto.a')
    curllib_file = join(curllib_dir, 'libcurl.a')
    nix = True
    if static:
       env.Append( LINKFLAGS=" -static " )

elif "win32" == guess_os:
    
    # when building windows
    windows = True
    #if force64:

    # check VC compiler
    for pathdir in env['ENV']['PATH'].split(os.pathsep):
        if os.path.exists(os.path.join(pathdir, 'cl.exe')):
            print( "found visual studio at " + pathdir )
            break
        else:
	          #use current environment
	          env['ENV'] = dict(os.environ)

    # if we are 64 bit
    if guess_arch == "ia64" and not force32:
        # use 64 bit boost library
        env.Append( EXTRALIBPATH=join(boost_lib_dir,'win64') )
        # use project-related ssl library
        env.Append( EXTRALIBPATH=join(ssl_dir,'lib/win64') )
        # use project-related curl library
        env.Append( EXTRALIBPATH=join(curl_dir,'lib/win64') )
        # use 64 bit spidermonkey
        if usesm:
            if debugBuild:
                smlib_dir = join(js_dir,'lib/debug/win64/lib')
                env.Append( CPPPATH=join(js_dir,'lib/debug/win64/include') )
                env.Append( EXTRALIBPATH=[smlib_dir] )
            else:
                smlib_dir = join(js_dir,'lib/release/win64/lib')
                env.Append( CPPPATH=join(js_dir,'lib/release/win64/include') )
                env.Append( EXTRALIBPATH=[smlib_dir] )
        force64 = False
        ssllib_dir = join(ssl_dir,'lib/win64')
        curllib_dir = join(curl_dir,'lib/win64')
    else:
	      # either we are 32 bit or force 32 bit
        env.Append( EXTRALIBPATH=join(boost_lib_dir,'win32') )
        # use project-related ssl library
        env.Append( EXTRALIBPATH=join(ssl_dir,'lib/win32') )
        # use project-related curl library
        env.Append( EXTRALIBPATH=join(curl_dir,'lib/win32') )
        if usesm:
            if debugBuild:
                smlib_dir = join(js_dir,'lib/debug/win32/lib')
                env.Append( CPPPATH=join(js_dir,'lib/debug/win32/include') )
                env.Append( EXTRALIBPATH=[smlib_dir] )
            else:
                smlib_dir = join(js_dir,'lib/release/win32/lib')
                env.Append( CPPPATH=join(js_dir,'lib/release/win32/include') )
                env.Append( EXTRALIBPATH=[smlib_dir] )
        ssllib_dir = join(ssl_dir,'lib/win32')
        curllib_dir = join(curl_dir,'lib/win32')
    if usesm:
        smlib_file = join(smlib_dir, 'mozjs185-1.0.dll')
        env.Append( CPPDEFINES=[ "XP_WIN" ] )
        env.Append( LIBS=['mozjs185-1.0'] )
        env.Append( CPPDEFINES=["JS_HAVE_STDINT_H"] )
    env.Append( LIBS=['libeay32'] )
    ssllib_file = join(ssllib_dir, 'libeay32.dll')
    ssllib_file1 = join(ssllib_dir, 'ssleay32.dll')
    curllib_file = join(curllib_dir, 'libcurl.dll')
    boostLibs = []

    env.Append( CPPDEFINES=[ "_UNICODE" ] )
    env.Append( CPPDEFINES=[ "UNICODE" ] )
    # find windows SDK
    winSDKHome = findVersion( [ "C:/Program Files/Microsoft SDKs/Windows/", "C:/Program Files (x86)/Microsoft SDKs/Windows/" ] ,
                              [ "v7.1", "v7.0A", "v7.0", "v6.1", "v6.0a", "v6.0" ] )
    print( "Windows SDK Root '" + winSDKHome + "'" )

    env.Append( EXTRACPPPATH=[ winSDKHome + "/Include" ] )

    # consider adding /MP build with multiple processes option.

    # /EHsc exception handling style for visual studio
    # /W3 warning level
    # /WX abort build on compiler warnings
    env.Append( CPPFLAGS=" /EHsc /W3 " ) #  /WX " )

    # some warnings we don't like:
    # c4355
    # 'this' : used in base member initializer list
    #    The this pointer is valid only within nonstatic member functions. It cannot be used in the initializer list for a base class.
    # c4800
    # 'type' : forcing value to bool 'true' or 'false' (performance warning)
    #    This warning is generated when a value that is not bool is assigned or coerced into type bool. 
    # c4267
    # 'var' : conversion from 'size_t' to 'type', possible loss of data
    # When compiling with /Wp64, or when compiling on a 64-bit operating system, type is 32 bits but size_t is 64 bits when compiling for 64-bit targets. To fix this warning, use size_t instead of a type.
    # c4244
    # 'conversion' conversion from 'type1' to 'type2', possible loss of data
    #  An integer type is converted to a smaller integer type.
    env.Append( CPPFLAGS=" /wd4355 /wd4800 /wd4267 /wd4244 /wd4200 " )

    # PSAPI_VERSION relates to process api dll Psapi.dll.
    env.Append( CPPDEFINES=["_CONSOLE","_CRT_SECURE_NO_WARNINGS","PSAPI_VERSION=1","_CRT_RAND_S" ] )

    # this would be for pre-compiled headers, could play with it later  
    #env.Append( CPPFLAGS=' /Yu"pch.h" ' ) 

    # docs say don't use /FD from command line (minimal rebuild)
    # /Gy function level linking
    # /Gm is minimal rebuild, but may not work in parallel mode.
    if release:
        env.Append( CPPDEFINES=[ "NDEBUG" ] )
        env.Append( CPPFLAGS= " /O2 /Gy " )
        env.Append( CPPFLAGS= " /MT /Zi /errorReport:none " )
        # TODO: this has caused some linking problems :
        # /GL whole program optimization
        # /LTCG link time code generation
        env.Append( CPPFLAGS= " /GL " )
        env.Append( LINKFLAGS=" /LTCG " )
        # /DEBUG will tell the linker to create a .pdb file
        # which WinDbg and Visual Studio will use to resolve
        # symbols if you want to debug a release-mode image
        env.Append( LINKFLAGS=" /DEBUG " )
    else:
        # /Od disable optimization
        # /Z7 debug info goes into each individual .obj file -- no .pdb created 
        # /TP it's a c++ file
        # /RTC1: - Enable Stack Frame Run-Time Error Checking; Reports when a variable is used without having been initialized
        env.Append( CPPFLAGS=" /RTC1 /MDd /Z7 /errorReport:none " )

        if debugBuild:
            env.Append( LINKFLAGS=" /debug " )
            env.Append( CPPFLAGS=" /Od " )
            
        if debugLogging:
            env.Append( CPPDEFINES=[ "_DEBUG" ] )

    if guess_arch == "ia64" and not force32:
        env.Append( EXTRALIBPATH=[ winSDKHome + "/Lib/x64" ] )
    else:
        env.Append( EXTRALIBPATH=[ winSDKHome + "/Lib" ] )

    if release:
        #env.Append( LINKFLAGS=" /NODEFAULTLIB:MSVCPRT  /NODEFAULTLIB:MSVCRTD " )
        env.Append( LINKFLAGS=" /NODEFAULTLIB:MSVCPRT  " )
    else:
        env.Append( LINKFLAGS=" /NODEFAULTLIB:MSVCPRT  /NODEFAULTLIB:MSVCRT  " )

    winLibString = "ws2_32.lib kernel32.lib advapi32.lib Psapi.lib"

    if force64:

        winLibString += ""
        #winLibString += " LIBCMT LIBCPMT "

    else:
        winLibString += " user32.lib gdi32.lib winspool.lib comdlg32.lib  shell32.lib ole32.lib oleaut32.lib "
        winLibString += " odbc32.lib odbccp32.lib uuid.lib dbghelp.lib "

    env.Append( LIBS=Split(winLibString) )

    # dm these should automatically be defined by the compiler. commenting out to see if works. jun2010
    #if force64:
    #    env.Append( CPPDEFINES=["_AMD64_=1"] )
    #else:
    #    env.Append( CPPDEFINES=["_X86_=1"] )

    env.Append( EXTRACPPPATH=["#/../winpcap/Include"] )
    env.Append( EXTRALIBPATH=["#/../winpcap/Lib"] )

else:
    print( "No special config for [" + os.sys.platform + "] which probably means it won't work" )

env['STATIC_AND_SHARED_OBJECTS_ARE_THE_SAME'] = 1
if nix:

    # -Winvalid-pch Warn if a precompiled header (see Precompiled Headers) is found in the search path but can't be used. 
    env.Append( CPPFLAGS="-fPIC -fno-strict-aliasing -ggdb -pthread -Wno-write-strings -Wall -Wsign-compare -Wno-unknown-pragmas -Winvalid-pch -Wno-address" )
    # env.Append( " -Wconversion" ) TODO: this doesn't really work yet
    if linux:
        env.Append( CPPFLAGS=" -pipe " )
        env.Append( CPPFLAGS=" -fno-builtin-memcmp " ) # glibc's memcmp is faster than gcc's

    env.Append( CPPDEFINES="_FILE_OFFSET_BITS=64" )
    env.Append( CXXFLAGS=" -Wnon-virtual-dtor " )
    env.Append( LINKFLAGS=" -fPIC -pthread -rdynamic" )
    env.Append( LIBS=[] )

    #make scons colorgcc friendly
    env['ENV']['HOME'] = os.environ['HOME']
    env['ENV']['TERM'] = os.environ['TERM']

    if linux and has_option( "sharedclient" ):
        env.Append( LINKFLAGS=" -Wl,--as-needed -Wl,-zdefs " )

    if debugBuild:
        env.Append( CPPFLAGS=" -O0 -fstack-protector " );
        env['ENV']['GLIBCXX_FORCE_NEW'] = 1; # play nice with valgrind
    else:
        env.Append( CPPFLAGS=" -O3 " )
        #env.Append( CPPFLAGS=" -fprofile-generate" )
        #env.Append( LINKFLAGS=" -fprofile-generate" )
        # then:
        #env.Append( CPPFLAGS=" -fprofile-use" )
        #env.Append( LINKFLAGS=" -fprofile-use" )

    if debugLogging:
        env.Append( CPPFLAGS=" -D_DEBUG" );

    if force64:
        env.Append( CFLAGS="-m64" )
        env.Append( CXXFLAGS="-m64" )
        env.Append( LINKFLAGS="-m64" )

    if force32:
        env.Append( CFLAGS="-m32" )
        env.Append( CXXFLAGS="-m32" )
        env.Append( LINKFLAGS="-m32" )

if "uname" in dir(os):
    hacks = buildscripts.findHacks( os.uname() )
    if hacks is not None:
        hacks.insert( env , { "linux64" : linux64 } )

try:
    umask = os.umask(022)
except OSError:
    pass

moduleFiles = {}
commonFiles = []
serverOnlyFiles = []
scriptingFiles = []

#env.Append( CPPPATH=['$EXTRACPPPATH'],
#            LIBPATH=['$EXTRALIBPATH'] )
env.Append( CPPPATH=env["EXTRACPPPATH"], LIBPATH=env["EXTRALIBPATH"])
env['SEQUOIA_COMMON_FILES'] = commonFiles
env['SEQUOIA_SERVER_ONLY_FILES' ] = serverOnlyFiles
env['SEQUOIA_SCRIPTING_FILES'] = scriptingFiles
env['SEQUOIA_MODULE_FILES'] = moduleFiles
# --- check system ---

def getSysInfo():
    if windows:
        return "windows " + str( sys.getwindowsversion() )
    else:
        return " ".join( os.uname() )

def doConfigure( myenv , shell=False ):
    conf = Configure(myenv)
    myenv["LINKFLAGS_CLEAN"] = list( myenv["LINKFLAGS"] )
    myenv["LIBS_CLEAN"] = list( myenv["LIBS"] )

    if 'CheckCXX' in dir( conf ):
        if  not conf.CheckCXX():
            print( "c++ compiler not installed!" )
            Exit(1)

    #if nix:
     #   if not conf.CheckLib( "stdc++" ):
     #       print( "can't find stdc++ library which is needed" );
     #       Exit(1)

    def myCheckLib( poss , failIfNotFound=False , staticOnly=False):

        if type( poss ) != types.ListType :
            poss = [poss]

        allPlaces = [];
        allPlaces += extraLibPlaces
        #if nix and release:
        if nix:
            allPlaces += myenv.subst( myenv["LIBPATH"] )
            if not force64:
                allPlaces += [ "/usr/lib" , "/usr/local/lib", "/usr/lib64" ]
            for p in poss:
                for loc in allPlaces:
                    fullPath = loc + "/lib" + p + ".a"
                    if os.path.exists( fullPath ):
                        myenv.Append( _LIBFLAGS='${SLIBS}',
                                      SLIBS=" " + fullPath + " " )
                        return True

        if release and not windows and failIfNotFound:
            print( "ERROR: can't find static version of: " + str( poss ) + " in: " + str( allPlaces ) )
            Exit(1)

        res = not staticOnly and conf.CheckLib( poss )
        if res:
            return True

        if failIfNotFound:
            print( "can't find or link against library " + str( poss ) + " in " + str( myenv["LIBPATH"] ) )
            print( "see config.log for more information" )
            if windows:
                print( "use scons --64 when cl.exe is 64 bit compiler" )
            Exit(1)

        return False

    if not conf.CheckCXXHeader( "boost/filesystem/operations.hpp" ):
        print( "can't find boost headers" )
        if shell:
            print( "\tshell might not compile" )
        else:
            Exit(1)

    # this will add it if it exists and works
    #myCheckLib( [ "boost_system" + boostCompiler + "-mt" + boostVersion ,
    #              "boost_system" + boostCompiler + boostVersion ] )

    for b in boostLibs:
        l = "boost_" + b
        myCheckLib( [ l + boostCompiler + "-mt" + boostVersion ,
                      l + boostCompiler + boostVersion ] ,
                      False)
                    #release or not shell)
    if not conf.CheckCXXHeader( "execinfo.h" ):
        myenv.Append( CPPDEFINES=[ "NOEXECINFO" ] )

    myenv["_HAVEPCAP"] = myCheckLib( ["pcap", "wpcap"] )
    removeIfInList( myenv["LIBS"] , "pcap" )
    removeIfInList( myenv["LIBS"] , "wpcap" )

    # Handle staticlib,staticlibpath options.
    staticlibfiles = []
    if has_option( "staticlib" ):
        # FIXME: probably this loop ought to do something clever
        # depending on whether we want to use 32bit or 64bit
        # libraries.  For now, we sort of rely on the user supplying a
        # sensible staticlibpath option. (myCheckLib implements an
        # analogous search, but it also does other things I don't
        # understand, so I'm not using it.)
        if has_option ( "staticlibpath" ):
            dirs = GetOption ( "staticlibpath" ).split( "," )
        else:
            dirs = [ "/usr/lib64", "/usr/lib" ]

        for l in GetOption( "staticlib" ).split( "," ):
            removeIfInList(myenv["LIBS"], l)
            found = False
            for d in dirs:
                f=  "%s/lib%s.a" % ( d, l )
                if os.path.exists( f ):
                    staticlibfiles.append(f)
                    found = True
                    break
            if not found:
                raise RuntimeError("can't find a static %s" % l)

    myenv.Append(LINKCOM=" $STATICFILES")
    myenv.Append(STATICFILES=staticlibfiles)

    return conf.Finish()

clientCppEnv = env.Clone()
clientCppEnv.Append( CPPDEFINES=[ "SDB_DLL_BUILD" ] )
clientCEnv = clientCppEnv.Clone()
clientCppEnv["BUILD_DIR"] = clientCppVariantDir
clientCEnv["BUILD_DIR"] = clientCVariantDir
env = doConfigure( env )

testEnv = env.Clone()
testEnv.Append( CPPPATH=["../"] )

shellEnv = None
shellEnv = env.Clone();

toolEnv = None
toolEnv = env.Clone() ;

fmpEnv = None
fmpEnv = env.Clone() ;

if release and ( linux64 ):
    shellEnv["LINKFLAGS"] = env["LINKFLAGS_CLEAN"]
    shellEnv["LIBS"] = env["LIBS_CLEAN"]
    shellEnv["SLIBS"] = ""

if windows:
    shellEnv.Append( LIBS=["winmm.lib"] )
    #env.Append( CPPFLAGS=" /TP " )

shellEnv = doConfigure( shellEnv , shell=True )

# add engine and client variable
env.Append( CPPDEFINES=[ "SDB_ENGINE" ] )
clientCppEnv.Append( CPPDEFINES=[ "SDB_CLIENT" ] )
clientCEnv.Append( CPPDEFINES=[ "SDB_CLIENT" ] )
# should we use engine or client for test env? not sure, let's put client for now
testEnv.Append( CPPDEFINES=[ "SDB_CLIENT" ] )
shellEnv.Append( CPPDEFINES=[ "SDB_CLIENT" ] )
shellEnv.Append( CPPDEFINES=[ "SDB_SHELL" ] )
shellEnv.Append( LIBPATH=[clientlib_dir] )
shellEnv.Append( LIBS=['sdbc'] )
toolEnv.Append( CPPDEFINES=[ "SDB_CLIENT" ] )
toolEnv.Append( CPPDEFINES=[ "SDB_TOOL" ] )
toolEnv.Append( LIBPATH=[clientlib_dir] )
toolEnv.Append( LIBS=['sdbc','sdbcpp'] )
fmpEnv.Append( CPPDEFINES=[ "SDB_FMP" ] )
fmpEnv.Append( CPPDEFINES=[ "SDB_CLIENT" ] )
fmpEnv.Append( LIBPATH=[clientlib_dir] )
fmpEnv.Append( LIBS=['sdbc'] )

def checkErrorCodes():
    import buildscripts.errorcodes as x
    if x.checkErrorCodes() == False:
        print( "next id to use:" + str( x.getNextCode() ) )
        Exit(-1)

#checkErrorCodes()
#  ---- Docs ----
def build_docs(env, target, source):
    from buildscripts import docs
    docs.main()

env.Alias("docs", [], [build_docs])
env.AlwaysBuild("docs")

#  ---- astyle ----

def doStyling( env , target , source ):

    res = utils.execsys( "astyle --version" )
    res = " ".join(res)
    if res.count( "2." ) == 0:
        print( "astyle 2.x needed, found:" + res )
        Exit(-1)

    files = utils.getAllSourceFiles() 
    files = filter( lambda x: not x.endswith( ".c" ) , files )

    cmd = "astyle --options=mongo_astyle " + " ".join( files )
    res = utils.execsys( cmd )
    print( res[0] )
    print( res[1] )


env.Alias( "style" , [] , [ doStyling ] )
env.AlwaysBuild( "style" )



#  ----  INSTALL -------

def getSystemInstallName():
    n = platform + "-" + processor
    if static:
        n += "-static"
    if has_option("nostrip"):
        n += "-debugsymbols"
    if nix and os.uname()[2].startswith( "8." ):
        n += "-tiger"

    try:
        findSettingsSetup()
        import settings
        if "distmod" in dir( settings ):
            n = n + "-" + str( settings.distmod )
    except:
        pass


    dn = GetOption( "distmod" )
    if dn and len(dn) > 0:
        n = n + "-" + dn

    return n

env['NIX_LIB_DIR'] = nixLibPrefix
env['INSTALL_DIR'] = installDir
if testEnv is not None:
    testEnv['INSTALL_DIR'] = installDir
if shellEnv is not None:
    shellEnv['INSTALL_DIR'] = installDir
if clientCppEnv is not None:
    clientCppEnv['INSTALL_DIR'] = installDir
if clientCEnv is not None:
    clientCEnv['INSTALL_DIR'] = installDir
if fmpEnv is not None:
    fmpEnv['INSTALL_DIR'] = installDir
# The following symbols are exported for use in subordinate SConscript files.
# Ideally, the SConscript files would be purely declarative.  They would only
# import build environment objects, and would contain few or no conditional
# statements or branches.
#
# Currently, however, the SConscript files do need some predicates for
# conditional decision making that hasn't been moved up to this SConstruct file,
# and they are exported here, as well.
Export("env")
Export("shellEnv")
Export("toolEnv")
Export("testEnv")
Export("fmpEnv")
Export("clientCppEnv")
Export("clientCEnv")
Export("installSetup getSysInfo")
Export("usesm")
Export("windows linux nix")
if usesm:
   Export("smlib_file")
Export("ssllib_file")
Export("ssllib_file1")
Export("curllib_file")
Export("hasEngine")
Export("hasTestcase")
Export("hasTool")
Export("driverDir")
Export("guess_os")

# Generating Versioning information
# In order to change the file location, we have to modify both win32 and linux
# ossVer_Autogen.h is NOT in SVN, we have to generate this file by scons before
# actually compling the project
# Thus, we should avoid putting ossVer* files to release package
if guess_os == "win32":
   # In windows platform, we take advantage of SubWCRev
   os.system ("SubWCRev . misc/autogen/ossVer.tmp SequoiaDB/engine/include/ossVer_Autogen.h")
else:
   # In NIX platform, we use svn and sed to send to ossVer_Autogen.h
   os.system("sed \"s/WCREV/$(svn info | grep Revision | awk '{print $2}')/g\" misc/autogen/ossVer.tmp > oss.tmp")
   os.system("sed 's/\$//g' oss.tmp > SequoiaDB/engine/include/ossVer_Autogen.h")

language = get_option ( "language" )
if language is None:
   os.system ( "scons -C misc/autogen" )
else:
   os.system ( "scons -C misc/autogen --language=" + language )

if hasEngine:
   env.SConscript( 'SequoiaDB/SConscript', variant_dir=variantDir, duplicate=False )

# Convert javascript files to a cpp file
print 'Convert js files to cpp'
sys.path.append(join(root_dir, 'misc'))
import jsToCpp
jsToCpp.jsToCpp(engine_dir)

# Always build client before tool/shell/fmp
if hasClient or hasTool or hasShell or hasFmp:
   clientCppEnv.SConscript( 'SequoiaDB/SConscriptClientCpp', variant_dir=clientCppVariantDir, duplicate=False )
   clientCEnv.SConscript ( 'SequoiaDB/SConscriptClientC', variant_dir=clientCVariantDir, duplicate=False )

if hasShell:
   shellEnv.SConscript ( 'SequoiaDB/SConscriptShell', variant_dir=shellVariantDir, duplicate=False )

if hasTool:
   toolEnv.SConscript ( 'SequoiaDB/SConscriptTool', variant_dir=toolVariantDir, duplicate=False )

if hasFmp:
   fmpEnv.SConscript ( 'SequoiaDB/SConscriptFmp', variant_dir=fmpVariantDir, duplicate=False )


