#include "pd.hpp"
#include "utilDictionary.hpp"
#include "utilLZW.hpp"

using namespace std;

namespace engine
{
   int utilDictionary::init(unsigned int size)
   {
      SDB_ASSERT( size > 0, "Invalid parameter value for dictionary size" ) ;
      _dictSize = size;

      return SDB_OK ;
   }

   int _utilDictionary::build( const char *src, unsigned int srcLen,
                               char *dict, unsigned int &maxDictLen )
   {
      INT32 rc = SDB_OK ;

      rc = utilLZWBuildDict( src, srcLen, dict, maxDictLen ) ;
      PD_RC_CHECK( rc, PDERROR, "Failed to build dictionary for lzw, "
                   "rc: %d", rc ) ;

   done:
      return rc ;
   error:
      goto done ;
   }
}

