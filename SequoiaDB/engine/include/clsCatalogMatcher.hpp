
#ifndef CLSCATALOGMATCHER_HPP_
#define CLSCATALOGMATCHER_HPP_

#include "core.hpp"
#include "oss.hpp"
#include "../bson/bsonobj.h"
#include "clsCatalogPredicate.hpp"

namespace engine
{
   class clsCatalogPredicateTree ;
   class _clsCatalogItem ;

   /*
      clsCatalogMatcher define
   */
   class clsCatalogMatcher : public SDBObject
   {
   public:
      clsCatalogMatcher( const bson::BSONObj &shardingKey );

      INT32 loadPattern( const bson::BSONObj &matcher );

      INT32 matches( _clsCatalogItem* pCatalogItem,
                     BOOLEAN &result );

   private:
      INT32 parseAnObj( const bson::BSONObj &matcher,
                        clsCatalogPredicateTree &predicateSet );

      INT32 parseCmpOp( const bson::BSONElement &beField,
                        clsCatalogPredicateTree &predicateSet );

      INT32 parseLogicOp( const bson::BSONElement &beField,
                          clsCatalogPredicateTree &predicateSet ) ;
      BOOLEAN isOpObj( const bson::BSONObj obj ) ;

   private:
      clsCatalogPredicateTree    _predicateSet;
      bson::BSONObj              _shardingKey;
      bson::BSONObj              _matcher;
   } ;

}

#endif // CLSCATALOGMATCHER_HPP_

