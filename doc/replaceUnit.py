
"""Module of replace the symbol in api files.

"""
import codecs
import os
import sys
import platform

# symbol head. all symbol should had.
SDB_SYMBOL_HEAD = "@SDB_SYMBOL_"

# symbol, use '@SDB_SYMBOL_VERSION' in api file rather than 'SDB_SYMBOL_VERSION'.
SDB_SYMBOL_VERSION = SDB_SYMBOL_HEAD + "VERSION"   # @SDB_SYMBOL_VERSION
# add new symbol in here
# new symbol...

# symbol default value
DEFAULT_VALUE = ""

# if add new symbol,  _symbol_map had to add it and you maybe add a new function to get it's value.
_symbol_map = {
    SDB_SYMBOL_VERSION : DEFAULT_VALUE
}


class ReplaceSymbol(object):

    def __init__(self, filepath):
        """init _symbol_map

        """
        # init SDB_SYMBOL_VERSION's value.
        version_value = self.__get_sdb_version(filepath)
        _symbol_map[SDB_SYMBOL_VERSION] = version_value
        # if add new symbol, can init it's value in here.
        # ...

    def replace_in_directory(self, files):
        """replace in a directory.

        :param files:  A directory.
        :return:
        """
        for root, dirs,files in os.walk(files):
            for fileName in files:
                self.replace_in_file(os.path.join(root,fileName))


    def replace_in_file(self, filePath):
        """replace in a file.

        :param filePath:  A file.
        :return:
        """

        if not ( filePath.endswith(".html")):
            return 0
        file_data = ""
        flags = 0
        with codecs.open(filePath, mode='r', encoding='utf-8') as f:
            for line in f:
               if SDB_SYMBOL_HEAD in line:
                   flags = 1
                   line = self.__change_symbol(line)
               file_data +=line
            f.close()
        if ( 1 == flags):
            with codecs.open(filePath , mode='w', encoding='utf-8') as f:
                f.write(file_data)
                f.close()

    def __change_symbol(self, line):
        """change symbol with it's value.

        """
        for symbol in _symbol_map:
            if symbol in line:
                line = line.replace(symbol, _symbol_map.get(symbol))
        return line

    def __get_sdb_version(self, filePath):
        """read file to get sdb version information

        """
        sdb_engine_version_str = None
        sub_version_str = None

        sdb_engine_version = None
        sub_version = None

        with codecs.open(filePath, mode='r', encoding='utf-8') as f:
            for line in f:
                if "SDB_ENGINE_VERISON_CURRENT" in line:
                    list = line.replace("\n","").split(' ')
                    sdb_engine_version_str = "#define " + list[-1]
                if "SDB_ENGINE_SUBVERSION_CURRENT" in line:
                    list = line.replace("\n","").split(' ')
                    sub_version_str = "#define " + list[-1]
            f.close()
        if sdb_engine_version_str != None and sub_version_str != None :
            with codecs.open(filePath, mode='r', encoding='utf-8') as f:
                for line in f:
                    if sdb_engine_version_str in line:
                        list = line.replace("\n","").split(' ')
                        sdb_engine_version = list[-1]
                    if sub_version_str in line:
                        list = line.replace("\n","").split(' ')
                        sub_version = list[-1]
                f.close()
        if ( sub_version == None ):
            version = "000"
        else:
            version = sdb_engine_version + "0" + sub_version
        return version

def GuessOS():
    id = platform.system()
    if id == 'Linux':
        return 'linux'
    elif id == 'Windows' or id == 'Microsoft':
        return 'win32'
    elif id == 'AIX':
        return 'aix'
    else:
        return None

if __name__ == "__main__":
    api_dir = None
    version_file = None
    root_dir = os.path.split( os.path.realpath( sys.argv[0] ) )[0]
    print("root_dir: ",root_dir)
    if GuessOS() == 'win32':
        api_dir =  os.path.join(root_dir, 'build\\output\\api')
        version_file = os.path.join(os.path.dirname(root_dir), 'SequoiaDB\\engine\\include\\ossVer.h')
    else:
        api_dir =  os.path.join(root_dir, 'build/output/api')
        version_file = os.path.join(os.path.dirname(root_dir), 'SequoiaDB/engine/include/ossVer.h')
    replace = ReplaceSymbol(version_file)
    replace.replace_in_directory(api_dir)