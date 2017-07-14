#include "rtnContextData.hpp"
#include "rtnQueryOperator.hpp"

namespace engine
{
   _rtnTSQueryOperator::_rtnTSQueryOperator()
   {
   }

   _rtnTSQueryOperator::~_rtnTSQueryOperator()
   {

   }

   INT32 _rtnTSQueryOperator::init( const rtnQueryOptions &options,
                                    pmdEDUCB *eduCB,
                                    SDB_RTNCB *rtnCB,
                                    INT64 &contextID,
                                    rtnContextBase **ppContext,
                                    BOOLEAN enablePrefetch )
   {
      INT32 rc = SDB_OK ;
      rtnContextTSData *pContext = NULL ;
      // One context should be allocated and opened.

      // 1. Allocate a context, and pass the options to it. Inside that context,
      // do the logic of communicate with search engine adapter.

      rc = rtnCB->contextNew( RTN_CONTEXT_TS_DATA, (rtnContext **)&pContext,
                              contextID, eduCB ) ;
      PD_RC_CHECK( rc, PDERROR, "Create text search context failed[ %d ]",
                   rc ) ;

      rc = pContext->open( options, eduCB ) ;
      PD_RC_CHECK( rc, PDERROR, "Open text search query context failed[ %d ]",
                   rc ) ;

      // For text search query, as we are gonna rewrite the query, the original
      // options should be stored.
      //rc = _options.getOwned() ;
      PD_RC_CHECK( rc, PDERROR,
                   "Get query options into own buffer failed[ %d ]", rc ) ;

      if ( ppContext )
      {
         *ppContext = pContext ;
      }

      if ( enablePrefetch )
      {
         pContext->enablePrefetch( eduCB ) ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _rtnTSQueryOperator::execute()
   {
      return SDB_OK ;
   }
}

