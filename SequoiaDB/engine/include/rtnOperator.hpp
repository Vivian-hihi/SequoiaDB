#ifndef RTN_OPERATOR_HPP__
#define RTN_OPERATOR_HPP__

#include "oss.hpp"
#include "pmdEDU.hpp"
#include "rtnCB.hpp"
#include "rtnQueryOptions.hpp"

namespace engine
{
   // Base class for coord Operators.
   class _rtnOperator : public SDBObject
   {
      public:
         _rtnOperator() ;
         virtual ~_rtnOperator() ;
      public:
         virtual INT32 init( const rtnQueryOptions &options,
                             pmdEDUCB *eduCB,
                             SDB_RTNCB *rtnCB,
                             INT64 &contextID,
                             rtnContextBase **ppContext,
                             BOOLEAN enablePrefetch ) = 0 ;
         virtual INT32 execute() = 0 ;
   } ;
   typedef _rtnOperator rtnOperator ;
}

#endif /* RTN_OPERATOR_HPP__ */

