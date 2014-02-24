/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = clsCataHashMatcher.hpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          2014-02-13  JHL  Initial Draft

   Last Changed =

*******************************************************************************/
#ifndef CLS_CATA_HASH_MATCHER_HPP_
#define CLS_CATA_HASH_MATCHER_HPP_

#include <vector>
#include <string>
#include "ossTypes.h"
#include "clsCatalogPredicate.hpp"

namespace engine
{

   class clsCataHashPredTree : public SDBObject
   {
   typedef std::vector< clsCataHashPredTree * > VEC_CLSCATAHASHPREDSET ;
   typedef std::map< std::string, bson::BSONElement > MAP_CLSCATAHASHPREDFIELDS ;
   public:
      clsCataHashPredTree( bson::BSONObj shardingKey );

      ~clsCataHashPredTree();

      void upgradeToUniverse();

      BOOLEAN isUniverse();

      BOOLEAN isNull();

      CLS_CATA_LOGIC_TYPE getLogicType();

      void setLogicType( CLS_CATA_LOGIC_TYPE type );

      void addChild( clsCataHashPredTree *&pChild );

      INT32 addPredicate( const CHAR *pFieldName, bson::BSONElement beField );

      void clear();

      INT32 generateHashPredicate( UINT32 partitionBit );

      INT32 matches( _clsCatalogItem* pCatalogItem,
                     BOOLEAN &result );

   private:
      clsCataHashPredTree( clsCataHashPredTree &right ){}   // forbid copy constructor
   private:
      VEC_CLSCATAHASHPREDSET        _children;
      MAP_CLSCATAHASHPREDFIELDS     _fieldSet;
      CLS_CATA_LOGIC_TYPE           _logicType;
      bson::BSONObj                 _shardingKey;
      INT32                         _hashVal;
      BOOLEAN                       _hasPred;
      BOOLEAN                       _isNull;
   };

   class clsCataHashMatcher : public SDBObject
   {
   typedef enum _CLS_CATA_PREDICATE_OBJ_TYPE
   {
      PREDICATE_OBJ_TYPE_OP_EQ = 0,
      PREDICATE_OBJ_TYPE_OP_NOT_EQ,
      PREDICATE_OBJ_TYPE_NOT_OP
   }CLS_CATA_PREDICATE_OBJ_TYPE ;
   public:
      // note: don't delete shardingkey before delete clsCataHashMatcher
      clsCataHashMatcher( const bson::BSONObj &shardingKey );

      virtual ~clsCataHashMatcher(){};

      INT32 loadPattern( const bson::BSONObj &matcher,
                        UINT32 partitionBit );

      INT32 matches( _clsCatalogItem* pCatalogItem,
                     BOOLEAN &result );

   private:
      INT32 parseAnObj( const bson::BSONObj &matcher,
                        clsCataHashPredTree &predicateSet );

      INT32 parseCmpOp( const bson::BSONElement &beField,
                        clsCataHashPredTree &predicateSet );

      INT32 parseLogicOp( const bson::BSONElement &beField,
                        clsCataHashPredTree &predicateSet );

      INT32 checkOpObj( const bson::BSONObj obj,
                        INT32 &result );

   private:
      clsCataHashPredTree        _predicateSet;
      bson::BSONObj              _shardingKey;
      bson::BSONObj              _matcher;
   };
}

#endif