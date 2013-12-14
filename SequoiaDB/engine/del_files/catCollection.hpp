#ifndef CATCOLLECTION_HPP__
#define CATCOLLECTION_HPP__

#include "core.hpp"
#include "cat.hpp"
#include "catNet.hpp"
namespace engine
{
   class _catCollection
   {
   public :
      INT32 findReplicationGroups ( CHAR *pCollectionName,
                                    BSONObj &matcher,
                                    vector<CAT_REPLGROUP_ID> &replGroups ) ;
      INT32 init () ;
      INT32 resync ( BOOLEAN isInitial ) ;
   } ;
   typedef class _catCollection catCollection ;
}

#endif
