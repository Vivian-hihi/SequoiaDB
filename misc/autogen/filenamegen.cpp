#include "core.hpp"
#include "ossUtil.h"
#include "filenamegen.h"
#include <stdio.h>
#include "boost/filesystem.hpp"
#include "boost/filesystem/operations.hpp"
#include "boost/filesystem/path.hpp"
namespace fs = boost::filesystem ;
using namespace std ;
const CHAR *fileNameSuffix[] = {
".cpp",
".c",
".h",
".hpp",
".C"
} ;

static BOOLEAN isSourceFile ( const CHAR *pFile )
{
   INT32 fileNameLen = sizeof(fileNameSuffix) / sizeof(CHAR*) ;
   for ( INT32 i = 0 ; i < fileNameLen; ++i )
   {
      if ( ossStrlen(pFile) > ossStrlen(fileNameSuffix[i]) &&
           ossStrncasecmp ( &pFile[strlen(pFile)-strlen(fileNameSuffix[i])],
                            fileNameSuffix[i], strlen(fileNameSuffix[i]) ) == 0)
         return TRUE ;
   }
   return FALSE ;
}

void FileNameGen::genList ()
{
   try
   {
      ofstream fout ( FILENAMEPATH ) ;
      if ( fout == NULL )
      {
         cout << "can not open file: " << FILENAMEPATH << endl ;
         exit(0) ;
      }
      string comment =
        "/** \\file ossErr.h\n"
        "    \\brief The meaning of the error code.\n"
        "*/\n"
        "/*    Copyright 2012 SequoiaDB Inc.\n"
        " *\n"
        " *    Licensed under the Apache License, Version 2.0 (the \"License\");\n"
        " *    you may not use this file except in compliance with the License.\n"
        " *    You may obtain a copy of the License at\n"
        " *\n"
        " *    http://www.apache.org/licenses/LICENSE-2.0\n"
        " *\n"
        " *    Unless required by applicable law or agreed to in writing, software\n"
        " *    distributed under the License is distributed on an \"AS IS\" BASIS,\n"
        " *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.\n"
        " *    See the License for the specific language governing permissions and\n"
        " *    limitations under the License.\n"
        " */\n"
        "/*    Copyright (C) 2011-2014 SequoiaDB Ltd.\n"
        " *    This program is free software: you can redistribute it and/or modify\n"
        " *    it under the term of the GNU Affero General Public License, version 3,\n"
        " *    as published by the Free Software Foundation.\n"
        " *\n"
        " *    This program is distributed in the hope that it will be useful,\n"
        " *    but WITHOUT ANY WARRANTY; without even the implied warrenty of\n"
        " *    MARCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the\n"
        " *    GNU Affero General Public License for more details.\n"
        " *\n"
        " *    You should have received a copy of the GNU Affero General Public License\n"
        " *    along with this program. If not, see <http://www.gnu.org/license/>.\n"
        " */\n";
      fout<<std::left<<comment<<endl;
      string comment =
         "/* This list file is automatically generated, you MUST NOT \
modify this file anyway! */" ;
      fout << comment << endl ;
      _genList ( SOURCEPATH, fout ) ;
      fout.close() ;
   }
   catch ( std::exception &e )
   {
      ossPrintf ( "Failed to gen list: %s"OSS_NEWLINE,
                  e.what() ) ;
   }
}

void FileNameGen::_genList ( const CHAR *pPath, std::ofstream &fout )
{
   const CHAR *pathSep = OSS_FILE_SEP ;
   fs::path directory ( pPath ) ;
   fs::directory_iterator end_iter ;

   if ( fs::exists(directory) && fs::is_directory(directory) )
   {
      for ( fs::directory_iterator dir_iter ( directory );
            dir_iter != end_iter; ++dir_iter )
      {
         if ( fs::is_regular_file ( dir_iter->status() ) )
         {
            const std::string fileName = dir_iter->path().filename().string() ;
            const CHAR *pFileName = fileName.c_str() ;
            if ( isSourceFile ( pFileName ) )
            {
               UINT32 hashCode = ossHashFileName ( pFileName ) ;
               fout << hashCode << " : " << pFileName << endl ;
            }
         }
         else if ( fs::is_directory ( dir_iter->status() ) )
         {
            if ( ossStrncmp ( dir_iter->path().filename().string().c_str(),
                              SKIPPATH, ossStrlen(SKIPPATH)) != 0 )
               _genList ( dir_iter->path().string().c_str(),
                          fout ) ;
         }
      }
   }
}

