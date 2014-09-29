
#ifndef OSSPATH_HPP__
#define OSSPATH_HPP__

#include "ossPath.h"
#include <string>
#include <map>
#include <vector>
#include "oss.h"

using namespace std ;

INT32 ossEnumFiles( const string &dirPath,
                    map<string, string> &mapFiles,
                    const CHAR *filter = NULL,
                    UINT32 deep = 1 ) ;

INT32 ossEnumFiles2( const string &dirPath,
                     map<string, string> &mapFiles,
                     const CHAR *filter = NULL,
                     OSS_MATCH_TYPE type = OSS_MATCH_ALL,
                     UINT32 deep = 1 ) ;

INT32 ossEnumSubDirs( const string &dirPath,
                      vector< string > &subDirs,
                      UINT32 deep = 1 ) ;


#endif // OSSPATH_HPP__

