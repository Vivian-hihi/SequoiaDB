# find python3 C API include directory
import sys
import platform
import os

def GuessOS():
    id = platform.system()
    if id == 'Linux':
        return 'linux'
    elif id == 'Windows' or id == 'Microsoft':
        return 'win32'
    else:
        return None

guess_os = GuessOS()

if sys.version_info[0] != 3:
    raise Exception("not python3")

if guess_os == 'linux':
        ver = sys.version_info[0]
        sub = sys.version_info[1]
        python_include = sys.prefix + '/include/' + ("python%d.%d" % (ver, sub))
        if not os.path.exists(python_include):
            python_include += "m"
            if not os.path.exists(python_include):
                raise Exception("can't find python3 include directory")
        python_lib = sys.prefix + '/lib/' + ("python%d.%d" % (ver, sub))
elif guess_os == 'win32':
        python_include = sys.prefix + "/include/"
        python_lib = sys.prefix + '/libs/'

print(python_include)
print(python_lib)