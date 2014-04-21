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
#include <fstream>
#include "sptCommon.hpp"

#define MFILE_SUFFIX ".cli"

#define DB_CATEGORY "db"
#define CS_CATEGORY "cs"
#define CL_CATEGORY "cl"
#define RG_CATEGORY "rg"
#define NODE_CATEGORY "node"
#define CURSOR_CATEGORY "cursor"
#define CLCOUNT_CATEGORY "count"

#define READ_CHARACTOR_NUM 1024
#define CMD_NAME_LEN  255

#define READ_CUTLINE_BEGIN ".SH \"NAME\""
#define READ_CUTLINE_END  ".SH \"SYNOPSIS\""
#define READ_SYN_BEGIN READ_CUTLINE_END
#define READ_SYN_END ".SH \"CATEGORY\""
#define MARK1 "\\fB"
#define MARK2 "\\fR"
#define MARK3 "("


namespace fs = boost::filesystem;

static INT32 removeMark( CHAR *buffer, INT32 buffer_len, const CHAR *mark ) ;

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
   rc = scanFile();
   if ( rc )
   {
      ossPrintf( "Failed to scan troff files."OSS_NEWLINE );
      ossPanic () ;
   }
}

manHelp::~manHelp() {}

INT32 manHelp::scanFile()
{
   INT32 rc = SDB_OK;
   INT32 tmp_buf_len = READ_CHARACTOR_NUM ;
   INT32 file_buf_len = READ_CHARACTOR_NUM ;
   CHAR *tmp_buffer = (CHAR *)SDB_OSS_MALLOC( tmp_buf_len + 1 ) ;
   CHAR *file_buffer = (CHAR *)SDB_OSS_MALLOC( file_buf_len ) ;
   typedef vector<fs::path> vec;
   vec v;
   const INT32 file_name_len = OSS_PROCESS_NAME_LEN ;
   fs::path p( filePath );
   CHAR *pFileName = NULL ;
   pFileName = (CHAR *)SDB_OSS_MALLOC( file_name_len + 1 );
   if ( !pFileName )
   {
      rc = SDB_OOM ;
      goto error ;
   }

   // i am going to extrace all the .cli file in /opt/sequoiadb/doc/manual
   // in to a vector<fs::path> and extrace what i want from this
   // vector

   // check the path
   if ( !fs::exists(p) || !fs::is_directory(p) )
   {
      rc = SDB_INVALIDARG;
      goto error;
   }
   // scan all the file in manual directory
   std::copy( fs::directory_iterator(p), fs::directory_iterator(),
              std::back_inserter(v) );
   // extract ".cli" file in vector to sset
   for ( vec::const_iterator it(v.begin()), it_end(v.end());
         it != it_end; it++ )
   {
      std::string fPath = (*it).string() ;
      const CHAR *pfPath = fPath.c_str() ;
     // nerver use const CHAR* pTmp = fs::extension(*it).c_str();
      std::string fSuffix = fs::extension(*it);
     const CHAR *pfSuffix = fSuffix.c_str();
      // if it is ".cli" file, save the file name
      // 1: save the file name to sset
      // 2: save synopsis and cutline to ssmap
      if ( ossStrncmp( pfSuffix, MFILE_SUFFIX, ossStrlen(MFILE_SUFFIX) ) == 0 )
      {
         INT32 strLen = 0;
         std::string fileName ;
         const CHAR* pLeaf = NULL;
         CHAR *pSplit = NULL ;
         // get the file name
         // get the file name without the file path
         std::string leaf = (*it).leaf().string();
         pLeaf = leaf.c_str();
         strLen = ossStrlen( pLeaf ) - ossStrlen( MFILE_SUFFIX );
         if ( strLen > file_name_len )
         {
            rc = SDB_INVALIDARG ;
            goto error;
         }
         ossMemcpy( pFileName, pLeaf, strLen );
         *(pFileName+ strLen) = 0;
         fileName =  pFileName ;
         // save the file name without ".cli" to sset
         // the format is: cs.createCL
         nset.insert( fileName );

         // when finishing save this file name to sset
         // i am going go save category and cutline to ssmap
         // split the file name cs.createCL
         pSplit = ossStrrchr( pFileName, '.' ) ;
         if ( !pSplit )
         {
            rc = SDB_INVALIDARG ;
            goto error ;
         }
         *pSplit = '\0' ;
         pSplit++ ;
         if ( ossMemcmp( pFileName, DB_CATEGORY, ossStrlen( pFileName ) ) == 0 ||
              ossMemcmp( pFileName, CS_CATEGORY, ossStrlen( pFileName ) ) == 0 || 
              ossMemcmp( pFileName, CL_CATEGORY, ossStrlen( pFileName ) ) == 0 ||
              ossMemcmp( pFileName, RG_CATEGORY, ossStrlen( pFileName ) ) == 0 ||
              ossMemcmp( pFileName, NODE_CATEGORY, ossStrlen( pFileName ) ) == 0 ||
              ossMemcmp( pFileName, CURSOR_CATEGORY, ossStrlen( pFileName ) ) == 0 ||
              ossMemcmp( pFileName, CLCOUNT_CATEGORY, ossStrlen( pFileName ) ) == 0 )
         {
            // read brief cutline from troff file
            std::ifstream fin ;
            INT32 troff_file_len = 0 ;
            INT32 read_real_len = 0 ;
            INT32 cutline_len = 0 ;
            CHAR* r_beg = NULL ;
            CHAR* r_end = NULL ;
            CHAR* r_pos = NULL ;
            std::string cutline ;
            std::string synopsis ;
            // open file
            fin.open( pfPath ) ;
            // get the troff_file_len of file
            fin.seekg ( 0, fin.end ) ;
            troff_file_len = fin.tellg() ;
            fin.seekg ( 0, fin.beg ) ;
            // check the file_buffer
            if ( file_buf_len < troff_file_len )
            {
               CHAR *pOld = file_buffer ;
               file_buffer= (CHAR *)SDB_OSS_REALLOC( file_buffer, troff_file_len ) ;
               if ( !file_buffer )
               {
                  ossPrintf ( "Failed to allocate %d bytes for read file"OSS_NEWLINE, troff_file_len ) ;
                  rc = SDB_OOM ;
                  file_buffer = pOld ;
                  goto exit ;
               }
               file_buf_len = troff_file_len ;
            }
            // read troff file
            fin.read ( file_buffer, troff_file_len ) ;

            // extract cutline
            // i am going to extract the content between '.SH "NAME"' and ''.SH "SYNOPSIS"'
            // in troff file as the cutline, i will abandon null string and some useless content
            r_beg = ossStrstr( file_buffer, READ_CUTLINE_BEGIN ) ;
            if ( !r_beg )
            {
               ossPrintf( "Failed to deal with file %s, for having no tag \"NAME\""OSS_NEWLINE, pfPath ) ;
               rc = SDB_INVALIDARG ;
               goto exit ;
            }
            r_end = ossStrstr( file_buffer, READ_CUTLINE_END ) ;
            if ( !r_end )
            {
               ossPrintf( "Failed to deal with file %s, for having no tag \"SYNOPSIS\""OSS_NEWLINE, pfPath ) ;
               rc = SDB_INVALIDARG ;
               goto exit ;
            }
            *r_end = '\0' ;
            // find out the right position to get the cutline
            r_pos =  ossStrstr( r_beg, pSplit ) ;
            if ( !r_pos )
            {
               ossPrintf ( "Failed to deal with file %s, for the content of tag \"NAME\" having no short name %s"OSS_NEWLINE, pfPath, pSplit ) ;
               rc = SDB_INVALIDARG ;
               goto exit ;
            }
            // r_pos is the right place to extract cutline
            // begin to extract cutline
            // check the receive buffer
            read_real_len = ossStrlen( r_pos ) ;
            if ( tmp_buf_len < read_real_len )
            {
               CHAR *pOld = tmp_buffer ;
               tmp_buffer= (CHAR *)SDB_OSS_REALLOC( tmp_buffer, read_real_len + 1 ) ;
               if ( !tmp_buffer )
               {
                  ossPrintf ( "Failed to allocate %d bytes for read file"OSS_NEWLINE, troff_file_len ) ;
                  rc = SDB_OOM ;
                  tmp_buffer = pOld ;
                  goto exit ;
               }
               tmp_buf_len = read_real_len + 1 ;
            }
            cutline_len = read_real_len - ossStrlen(pSplit) ;
            ossMemcpy( tmp_buffer, r_pos + ossStrlen(pSplit), cutline_len ) ;
            tmp_buffer[ cutline_len ] = '\0' ;
            cutline = tmp_buffer ;
            // when finishing extract cutline, i am going to extract synopsis
            r_beg = r_end + 1 ;
            r_pos = NULL ;
            r_end = NULL ;
            r_end = ossStrstr( r_beg, READ_SYN_END ) ;
            if ( !r_end )
            {
               ossPrintf( "Failed to deal with file %s, for having no tag \"CATEGORY\""OSS_NEWLINE, pfPath ) ;
               rc = SDB_INVALIDARG ;
               goto exit ;
            }
            *r_end = '\0' ;
            // find out the right position to get the synopsis
            r_pos = ossStrstr ( r_beg, pSplit ) ;
            if ( !r_pos )
            {
               ossPrintf ( "Failed to deal with file %s, for the content of tag \"SYNOPIS\" having no short name %s"OSS_NEWLINE, pfPath, pSplit ) ;
               rc = SDB_INVALIDARG ;
               goto exit ;
            }
            // r_pos is the right place to extract synopsis
            // begin to extract synopsis
            // check the recieve buffer
            read_real_len = ossStrlen( r_pos ) ;
            if ( tmp_buf_len < read_real_len )
            {
               CHAR *pOld = tmp_buffer ;
               tmp_buffer= (CHAR *)SDB_OSS_REALLOC( tmp_buffer, read_real_len + 1 ) ;
               if ( !tmp_buffer )
               {
                  ossPrintf ( "Failed to allocate %d bytes for read file"OSS_NEWLINE, troff_file_len ) ;
                  rc = SDB_OOM ;
                  tmp_buffer = pOld ;
                  goto exit ;
               }
               tmp_buf_len = read_real_len + 1 ;
            }
            ossMemcpy( tmp_buffer, r_pos, read_real_len ) ;
            tmp_buffer[read_real_len] = '\0' ;
            // remove the useless mark
            rc = removeMark( tmp_buffer, read_real_len, MARK1 ) ;
            rc = removeMark( tmp_buffer, read_real_len, MARK2 ) ;
            #if defined ( _WINDOW )
            rc = removeMark( tmp_buffer, read_real_len, "\r\n" ) ;
            #else
            rc = removeMark( tmp_buffer, read_real_len, "\n" ) ;
            #endif
            rc = removeMark( tmp_buffer, read_real_len, "\f" ) ;
            synopsis = tmp_buffer ;

            // put synopsis and cutline in map
            if ( ossMemcmp( pFileName, DB_CATEGORY, ossStrlen( pFileName ) ) == 0 )
            {
               mdb.insert( std::pair<string, string>(synopsis, cutline) ) ;
            }
            else if ( ossMemcmp( pFileName, CS_CATEGORY, ossStrlen( pFileName ) ) == 0 )
            {
               mcs.insert( std::pair<string, string>(synopsis, cutline) ) ;
            }
            else if ( ossMemcmp( pFileName, CL_CATEGORY, ossStrlen( pFileName ) ) == 0 )
            {
               mcl.insert( std::pair<string, string>(synopsis, cutline) ) ;
            }
            else if ( ossMemcmp( pFileName, RG_CATEGORY, ossStrlen( pFileName ) ) == 0 )
            {
               mrg.insert( std::pair<string, string>(synopsis, cutline) ) ;
            }
            else if ( ossMemcmp( pFileName, NODE_CATEGORY, ossStrlen( pFileName ) ) == 0 )
            {
               mnode.insert( std::pair<string, string>(synopsis, cutline) ) ;
            }
            else if ( ossMemcmp( pFileName, CURSOR_CATEGORY, ossStrlen( pFileName ) ) == 0 )
            {
               mcursor.insert( std::pair<string, string>(synopsis, cutline) ) ;
            }
            else if ( ossMemcmp( pFileName, CLCOUNT_CATEGORY, ossStrlen( pFileName ) ) == 0 )
            {
               mclcount.insert( std::pair<string, string>(synopsis, cutline) ) ;
            }
            else
            {
               ossPrintf( "Failed to deal with file %s, for the wrong file name %s"OSS_NEWLINE, pfPath, pFileName ) ;
               rc = SDB_INVALIDARG ;
               goto exit ;
            }
          exit:
            // close file
            fin.close();
            if ( rc )
               goto error ;
            // save category and brief command to ssmap
         }
         else
         {
            ossPrintf( "Failed to deal with file %s, for the wrong file name %s"OSS_NEWLINE, pfPath, pFileName ) ;
            rc = SDB_INVALIDARG ;
            goto error ;
         }
      }
   }
done :
   if ( NULL != file_buffer )
   {
      SDB_OSS_FREE( file_buffer ) ;
      file_buffer = NULL ;
   }
   if ( NULL != tmp_buffer )
   {
      SDB_OSS_FREE( tmp_buffer ) ;
      tmp_buffer = NULL ;
   }
   if ( NULL != pFileName )
   {
      SDB_OSS_FREE( pFileName ) ;
      pFileName = NULL ;
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

INT32 manHelp::getFileHelp( const CHAR* category, const CHAR* cmd )
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
   if ( category == NULL )
   {
      rc = SDB_INVALIDARG ;
      goto error ;
   }
   // in case name == NULL display all the method in this category
   if ( NULL == cmd )
   {
      rc = displayMethod( category ) ;
      if ( rc )
         goto error ;
   }
   // in case name != NULL display method manual
   else
   {
      rc = displayManual( category, cmd ) ;
      if ( rc )
         goto error ;
   }
   /*
   //
   // search the name set to find out the matched file name
   for ( sset::const_iterator it(nset.begin()), it_end(nset.end());
         it != it_end; it++ )
   {
      str = ossStrstr( (*it).c_str(), cmd );
      if ( str != NULL )
      {
         tmp.insert( (*it) );
      }
   }
   // if we not find any matched file name, tell the user directly
   if ( tmp.size() == 0 )
      ossPrintf( "No manual for method %s"OSS_NEWLINE, cmd );
   else if ( tmp.size() > 1 ) // if we get more than 1 file names, let the user fill again
   {
      ossPrintf( "%d methods named \"%s\", please fill in full name: \n",
                 (INT32)tmp.size(), cmd );
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
   */
done :
   return rc;
error :
   goto done;
}

typedef std::map<string, string> ssmap ;


ssmap& manHelp::getCategoryMap( const CHAR *category )
{
    if ( ossMemcmp( category, DB_CATEGORY, ossStrlen( category ) ) == 0 )
    {
       return mdb ;
    }
    else if ( ossMemcmp( category, CS_CATEGORY, ossStrlen( category ) ) == 0 )
    {
       return mcs ;
    }
    else if ( ossMemcmp( category, CL_CATEGORY, ossStrlen( category ) ) == 0 )
    {
       return mcl ;
    }
    else if ( ossMemcmp( category, RG_CATEGORY, ossStrlen( category ) ) == 0 )
    {
       return mrg ;
    }
    else if ( ossMemcmp( category, NODE_CATEGORY, ossStrlen( category ) ) == 0 )
    {
       return mnode ;
    }
    else if ( ossMemcmp( category, CURSOR_CATEGORY, ossStrlen( category ) ) == 0 )
    {
       return mcursor ;
    }
    else if ( ossMemcmp( category, CLCOUNT_CATEGORY, ossStrlen( category ) ) == 0 )
    {
       return mclcount ;
    }
    else
    {
       return mempty ;
    }
}

INT32 manHelp::displayMethod( const CHAR *category )
{
    INT32 rc = SDB_OK ;
    ssmap &cate = getCategoryMap( category ) ;
    if ( cate == mempty )
    {
       rc = SDB_INVALIDARG ;
       goto error ;
    }
    else
    {
       const CHAR *p_first = NULL ;
       const CHAR *p_second = NULL ;
       ssmap::iterator it = cate.begin() ;
       for ( ; it != cate.end(); it++ )
       {
          p_first = (it->first).c_str() ;
          p_second = (it->second).c_str() ;
//          ossPrintf ( "   %s  %s", p_first, p_second ) ;
//          ossPrintf ( "   %s  %s"OSS_NEWLINE, p_first, p_second ) ;
          ossPrintf ( "   %s"OSS_NEWLINE, p_first ) ;
       }
    }
done :
   return rc ;
error :
   goto done ;
}

INT32 manHelp::displayManual( const CHAR *category, const CHAR *cmd )
{
   INT32 rc = SDB_OK ;
   CHAR *p_first = NULL ;
   CHAR *p = NULL ;
   CHAR command[ CMD_NAME_LEN + 2 ] = { 0 } ;
   INT32 commandLen = 0 ;
   INT32 cateLen = 0 ;
   INT32 cmdLen = 0 ;
   // check
   if ( !category || !cmd )
   {
      rc = SDB_INVALIDARG ;
      goto error ;
   }

   cateLen = ossStrlen( category ) ;
   cmdLen = ossStrlen( cmd ) ;
   if ( cateLen + cmdLen > CMD_NAME_LEN )
   {
      rc = SDB_INVALIDARG ;
      goto error ;
   }
   // build the full name of a command e.g. cs.createCL
   // because our troff files are named "category.cmd.cli"
   // but, if user does not offer category, just use cmd to do
   // fuzzy search
   if ( 0 != cateLen )
   {
      ossMemcpy( command, category, cateLen ) ;
      ossMemcpy( command + cateLen, ".", 1 ) ;
      ossMemcpy( command + cateLen + 1, cmd, cmdLen ) ;
      command[ cateLen + 1 + cmdLen ] = '\0' ;
   }
   else
   {
      ossMemcpy( command, cmd, cmdLen ) ;
      command[ cateLen + cmdLen ] = '\0' ;
   }
   // display man page
   rc = getFileHelp ( command ) ;
   if ( rc )
   {
      goto error ;
   }
/*
   ssmap &cate = getCategoryMap( category ) ;
   // can i compare them like this ?????
   // it seem it will never be true
   if ( mempty == cate )
   {
      rc = SDB_INVALIDARG ;
      goto error ;
   }
   ssmap::iterator it = cate.begin() ;
   for ( ; it != cate.end(); it++ )
   {
      p_first = (it->first).c_str() ;
      p = ossStrrchr( p_first, MARK3 ) ;
      if ( !p )
      {
         ossPrintf( "Command %s has wrong format."OSS_NEWLINE, p_first ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }
      cmdLen = p - p_first ;
      ossMemcpy( command, p, cmdLen ) ;
      command[ cmdLen + 1 ] = '\0' ;
      p = ossStrstr( cmdLen, cmd ) ;
      if ( !p )
      {

      }
   }
*/
done :
   return rc ;
error :
   goto done ;
}

static INT32 removeMark( CHAR *buffer, INT32 buffer_len, const CHAR *mark )
{
   INT32 rc = SDB_OK ;
   INT32 strLen = 0 ;
   INT32 moveLen = 0 ;
   // mark the current position of "mark"
   CHAR *pb = NULL ;
   // mark the next position of "mark"
   CHAR *pe = NULL ;
   // destination to move to
   CHAR *pp = NULL ;
   if ( !buffer || !mark || buffer_len <= 0 )
   {
      rc = SDB_INVALIDARG ;
      goto error ;
   }
   buffer[buffer_len] = '\0' ;
   strLen = ossStrlen(mark) ;
   pp = pb = ossStrstr( buffer, mark ) ;
   if ( !pb )
    goto done ;
   while ( true )
   {
      pe = ossStrstr( pb + strLen, mark ) ;
      if ( !pe )
      {
         moveLen = ossStrlen( pb + strLen ) ;
         ossMemmove( pp, pb + strLen, moveLen + 1 ) ;
         break ;
      }
      else
      {
         moveLen = pe - (pb + strLen) ;
         ossMemmove( pp, pb + strLen, moveLen ) ;
         pp += moveLen ;
         pb = pe ;
      }
   }
done :
   return rc ;
error :
   goto done ;
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
