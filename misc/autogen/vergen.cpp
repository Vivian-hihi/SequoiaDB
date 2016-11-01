#include "vergen.h"
#include "ossVer.h"
#include <fstream>
#include <iostream>
#include <sstream>

using namespace std ;

#define VER_PATH     "../../doc_new/config/version.json"
#define VER_MAJOR    "major"
#define VER_MINOR    "minor"

VerGen::VerGen()
{
}

VerGen::~VerGen()
{
}

void VerGen::run()
{
   ofstream fout( VER_PATH ) ;

   fout << "{" << endl ;
   fout << "    " << "\"" << VER_MAJOR << "\": " << SDB_ENGINE_VERISON_CURRENT << "," << endl ;
   fout << "    " << "\"" << VER_MINOR << "\": " << SDB_ENGINE_SUBVERSION_CURRENT << endl ;
   fout << "}" << endl ;
}

