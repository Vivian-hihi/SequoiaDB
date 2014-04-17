#include "core.h"
#include "ossUtil.h"
#include "sptParseTroff.hpp"
#include "ossErr.h"
#include "ossMem.h"
#include "../mdocml/parseMandocCpp.hpp"
#include "ossTypes.h"
#include <algorithm>
#include <boost/filesystem.hpp>
#include <vector>
#include "sptCommon.hpp"

#define MFILE_SUFFIX ".cli"

namespace fs = boost::filesystem;

namespace engine
{

manHelp* manHelp::m_pInstance = NULL;

manHelp* manHelp::createInstance( const CHAR *path )
{
   if ( !m_pInstance )
   {
      m_pInstance = new manHelp( path );
      if ( !m_pInstance )
         ossPrintf( "Failed to new manHelp."OSS_NEWLINE );
   }
   return m_pInstance;
}

void manHelp::destroyInstance()
{
   if ( m_pInstance )
   {
      delete m_pInstance;
      m_pInstance = NULL;
   }
}

manHelp::manHelp( const CHAR *path )
{
   INT32 rc = SDB_OK;
   INT32 len = ossStrlen( path );
   ossMemcpy( filePath, path , len );
   filePath[len] = 0;
   // scan the .cli files
   rc = scan();
   if ( rc )
   printf("error happen in constructor.\n");
}

manHelp::~manHelp(){}

INT32 manHelp::scan()
{
   INT32 rc = SDB_OK;
   typedef vector<fs::path> vec;
   vec v;
   const CHAR* fname = NULL;
   INT32 strLen = 0;
   const INT32 NAME_LEN = 256;
   CHAR *pStr = (CHAR *)SDB_OSS_MALLOC( NAME_LEN );

   fs::path p( filePath );
   if ( !fs::exists(p) || !fs::is_directory(p) )
   {
      rc = SDB_INVALIDARG;
      goto error;
   }

   std::copy( fs::directory_iterator(p), fs::directory_iterator(),
              std::back_inserter(v) );

   for ( vec::const_iterator it(v.begin()), it_end(v.end());
         it != it_end; it++ )
   {
      // if it is ".cli" file, save the file name
      if ( ossStrncmp( fs::extension(*it).c_str(), MFILE_SUFFIX, ossStrlen(MFILE_SUFFIX) ) == 0 )
      {
         // get the file name
         fname = (*it).leaf().c_str();
         strLen = ossStrlen( fname ) - ossStrlen( MFILE_SUFFIX );
         if ( strLen > NAME_LEN - 1 )
         {
            rc = SDB_INVALIDARG ;
            goto error;
         }
         ossMemcpy( pStr, fname, strLen );
         *(pStr + strLen) = 0;
         std::string n( pStr );
         // save the file name without ".cli"
         nset.insert( n );
      }
   }
done :
   if ( pStr != NULL )
   {
      SDB_OSS_FREE( pStr );
      pStr = NULL;
   }
   return rc;
error :
   goto done;
}

INT32 manHelp::getFileHelp( const CHAR* name )
{
   INT32 rc = SDB_OK;
   sset tmp;
   const CHAR *str = NULL;
   CHAR fPath[ OSS_MAX_PATHSIZE + 1 ] = { 0 };
   // get a instance use to parse troff file
   engine::parseMandoc *md = engine::parseMandoc::createInstance();
   if ( md == NULL )
   {
      rc = SDB_OOM ;
      goto error ;
   }
   // check argument
   if ( name == NULL || ossStrcmp(name, "") == 0 )
   {
      rc = SDB_INVALIDARG ;
      goto error ;
   }
   // search the name set to find out the matched file name
   for ( sset::const_iterator it(nset.begin()), it_end(nset.end());
         it != it_end; it++ )
   {
      str = ossStrstr( (*it).c_str(), name );
      if ( str != NULL )
      {
         tmp.insert( (*it) );
      }
   }
   // if we not find any matched file name, tell the user directly
   if ( tmp.size() == 0 )
      ossPrintf( "No manual for method %s"OSS_NEWLINE, name );
   else if ( tmp.size() > 1 ) // if we get more than 1 file names, let the user fill again
   {
      ossPrintf( "%d methods named \"%s\", please fill in full name: \n",
                 (INT32)tmp.size(), name );
      for ( sset::const_iterator it(tmp.begin()), it_end(tmp.end());
            it != it_end; it++ )
      {
         ossPrintf( "    %s"OSS_NEWLINE, (*it).c_str() );
      }
   }
   else // if we get one, parse it
   {
      sset::const_iterator it(tmp.begin());
      ossMemcpy( fPath, filePath, ossStrlen(filePath) );
      ossStrncat( fPath, (*it).c_str(), ossStrlen((*it).c_str()) );
      ossStrncat( fPath, MFILE_SUFFIX, ossStrlen(MFILE_SUFFIX) );
      // parse
      rc = md->parse ( fPath ) ;
      if ( rc != SDB_OK )
      {
         goto error ;
      }
   }

done :
   return rc;
error :
   goto done;
}
}
/*
int main()
{
   INT32 rc = 0;
   const CHAR* path = "/home/users/tanzhaobo/sequoiadb/doc/manual/";
   manHelp scf(path);
   rc = scf.getFileHelp("createCL");
   if ( rc )
   {
      printf("wrong!\n");
   }
   else
   {
      printf("OK!\n");
   }
   return 0;
}
*/
