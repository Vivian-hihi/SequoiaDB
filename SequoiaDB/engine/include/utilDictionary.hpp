#ifndef UTIL_DICTIONARY__
#define UTIL_DICTIONARY__

#include "oss.hpp"

namespace engine
{
   class _utilDictionary : public SDBObject
   {
   public:
      _utilDictionary() : _dictSize(0) {}
      ~_utilDictionary() {}
   public:
      int init(unsigned int size);
      /*
      int build_orig( const char *src, unsigned int srcLen,
                 char *dict, unsigned int &maxDictLen ) ;
      */
      int build( const char *src, unsigned int srcLen,
                 char *dict, unsigned int &maxDictLen ) ;
   private:
      unsigned int _dictSize;
   };
   typedef _utilDictionary utilDictionary;
}
#endif /* UTIL_DICTIONARY__ */

