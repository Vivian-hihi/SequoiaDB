#ifndef RTN_QUERY_OPERATOR__
#define RTN_QUERY_OPERATOR__

#include "rtnOperator.hpp"

namespace engine
{
   // Operator for query with text search.
   class _rtnTSQueryOperator : public _rtnOperator
   {
      public:
         _rtnTSQueryOperator() ;
         virtual ~_rtnTSQueryOperator() ;

         INT32 init( const rtnQueryOptions &options,
                     pmdEDUCB *eduCB,
                     SDB_RTNCB *rtnCB,
                     INT64 &contextID,
                     rtnContextBase **ppContext,
                     BOOLEAN enablePrefetch ) ;
         INT32 execute() ;
      private:
         // rtnSEAdptSession _adptSession ;
   } ;
   typedef _rtnTSQueryOperator rtnTSQueryOperator ;
}

#endif /* RTN_QUERY_OPERATOR__ */

