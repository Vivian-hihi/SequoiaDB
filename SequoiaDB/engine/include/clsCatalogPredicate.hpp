#ifndef CLSCATALOGPREDICATE_HPP_
#define CLSCATALOGPREDICATE_HPP_

#include "rtnPredicate.hpp"

namespace engine
{
   class clsCatalogPredicateTree;
   class _clsCatalogItem;

   typedef std::map< std::string , rtnStartStopKey * >            MAP_CLSCATAPREDICATEFIELD;
   typedef std::vector< clsCatalogPredicateTree * >                VEC_CLSCATAPREDICATESET;

   typedef enum _CLS_CATA_LOGIC_TYPE
   {
      CLS_CATA_LOGIC_INVALID=0,
      CLS_CATA_LOGIC_AND=1,
      CLS_CATA_LOGIC_OR,
   }CLS_CATA_LOGIC_TYPE;

   class clsCatalogPredicateTree : public SDBObject
   {
   public:
      clsCatalogPredicateTree( bson::BSONObj shardingKey );
      ~clsCatalogPredicateTree();
      void upgradeToUniverse();
      BOOLEAN isUniverse();
      CLS_CATA_LOGIC_TYPE getLogicType();
      void setLogicType( CLS_CATA_LOGIC_TYPE type );
      void addChild( clsCatalogPredicateTree *pChild );
      INT32 addPredicate( const CHAR *pFieldName, bson::BSONElement beField );
      void adjustByShardingKey();
      void clear();
      INT32 matches( _clsCatalogItem * pCatalogItem, BOOLEAN & result );
   private:
      clsCatalogPredicateTree( clsCatalogPredicateTree &right ){}   // forbid copy constructor
   private:
      VEC_CLSCATAPREDICATESET       _children;
      rtnPredicateSet               _predicateSet;
      CLS_CATA_LOGIC_TYPE           _logicType;
      bson::BSONObj                 _shardingKey;
   };
}

#endif
