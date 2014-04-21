#include <set>
#include "ossTypes.h"
#include <string>
#include <map>
#include "ossUtil.h"
#include "sptCommon.hpp"


using namespace std;

#ifndef PATH_LEN
#define PATH_LEN 255
#endif

class manHelp
{
public:
   static manHelp* createInstance( const CHAR *path ) ;
   static void destroyInstance() ;
   ~manHelp() ;
   INT32 getFileHelp( const CHAR *cmd ) ;
   INT32 getFileHelp( const CHAR *category, const CHAR *cmd ) ;
private:
   typedef std::set<string> sset ;
   typedef std::map<string, string> ssmap ;
   manHelp( const CHAR* path ) ;
   INT32 scanFile() ;
   ssmap& getCategoryMap( const CHAR *category ) ;
   INT32 displayMethod( const CHAR *category ) ;
   INT32 displayManual( const CHAR *category, const CHAR *cmd ) ;

   static manHelp *m_pInstance ;
   sset nset ;
   CHAR filePath[ OSS_MAX_PATHSIZE + 1 ] ;
   ssmap mdb ;
   ssmap mcs ;
   ssmap mcl ;
   ssmap mrg ;
   ssmap mnode ;
   ssmap mcursor ;
   ssmap mclcount;
   ssmap mempty ;

} ;


