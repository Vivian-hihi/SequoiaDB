#ifndef RTN_OPERATOR_FACTORY__
#define RTN_OPERATOR_FACTORY__

#include "rtnOperator.hpp"
#include "rtnQueryOptions.hpp"
#include "../bson/bsonobj.h"

namespace engine
{
   enum _rtnQueryType
   {
      RTN_QUERY_NORMAL = 1,
      RTN_QUERY_TEXT
   } ;
   typedef enum _rtnQueryType rtnQueryType ;

   // Factory to produce operators such as rtnTSQueryOperator.
   // It will analyze the message, and decide which kind of operator to produce.
   class _rtnOperatorFactory : public SDBObject
   {
      public:
         _rtnOperatorFactory() ;
         ~_rtnOperatorFactory() ;

         INT32 create( const rtnQueryOptions &options,
                       rtnOperator **pOperator ) ;
         void release( rtnOperator **pOperator ) ;
      private:
         INT32 _getQueryType( const BSONObj &query, rtnQueryType &qType ) ;
   } ;
   typedef _rtnOperatorFactory rtnOperatorFactory ;

   rtnOperatorFactory* rtnGetOperatorFactory() ;
}

#endif /* RTN_OPERATOR_FACTORY__ */

