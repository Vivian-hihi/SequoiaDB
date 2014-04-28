
#ifndef OSSPATH_HPP__
#define OSSPATH_HPP__

#include "ossPath.h"
#include <string>
#include <map>

using namespace std ;

INT32 ossEnumFiles( const string &dirPath,
                    map<string, string> &mapFiles,
                    const CHAR *filter = NULL,
                    UINT32 deep = 1 ) ;


#endif // OSSPATH_HPP__

