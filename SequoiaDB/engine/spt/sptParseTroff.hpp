#include <set>
#include "ossTypes.h"
#include <string>
#include "ossUtil.h"
#include "sdbcommon.hpp"


using namespace std;

#ifndef PATH_LEN
#define PATH_LEN 255
#endif

namespace engine
{

class manHelp
{
public:
   static manHelp* createInstance( const CHAR *path );
   static void destroyInstance();
   ~manHelp();
   INT32 getFileHelp( const CHAR* name );
private:
   manHelp( const CHAR* path );
   INT32 scan();
   static manHelp *m_pInstance;
   typedef std::set<string> sset;
   sset nset;
   CHAR filePath[PROG_PATH_LEN];
} ;

}
