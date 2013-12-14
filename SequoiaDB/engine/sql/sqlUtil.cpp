#include "sqlUtil.hpp"
#include "../bson/bson.h"
#include "pdTrace.hpp"
#include "sqlTrace.hpp"

using namespace bson ;

namespace engine
{
   static void dump( const SQL_CONTAINER &c,
                          INT32 indent )
   {
      for ( SQL_CONTAINER::const_iterator itr = c.begin();
            itr != c.end();
            itr++ )
      {
         cout << string( indent * 4, ' ') << "|--(value:" <<
                 string( itr->value.begin(), itr->value.end())
                 << " id:"<< itr->value.id().to_long()<< ")" << endl ;
         dump( itr->children, indent + 1) ;
      }
   }


   void sqlDumpAst( const SQL_CONTAINER &c )
   {
      dump( c, 0 ) ;
   }
}
