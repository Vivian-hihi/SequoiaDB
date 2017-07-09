#include "rtnOperatorFactory.hpp"
#include "rtnQueryOperator.hpp"

namespace engine
{
   _rtnOperatorFactory::_rtnOperatorFactory()
   {
   }

   _rtnOperatorFactory::~_rtnOperatorFactory()
   {
   }

   INT32 _rtnOperatorFactory::create( const rtnQueryOptions &options,
                                      rtnOperator **pOperator )
   {
      INT32 rc = SDB_OK ;
      rtnOperator *opr = NULL ;
      rtnQueryType type = RTN_QUERY_NORMAL ;

      // Analyze the message, and create the operator.
      rc = _getQueryType( options._query, type ) ;
      PD_RC_CHECK( rc, PDERROR, "Get query type failed[ %d ]", rc ) ;

      if ( RTN_QUERY_TEXT == type )
      {
         opr = SDB_OSS_NEW rtnTSQueryOperator() ;
         if ( !opr )
         {
            PD_LOG( PDERROR, "Create text search query operator failed" ) ;
            rc = SDB_OOM ;
            goto error ;
         }
      }

      *pOperator = opr ;

   done:
      return rc ;
   error:
      goto done ;
   }

   void _rtnOperatorFactory::release( rtnOperator **pOperator )
   {
      if ( pOperator && *pOperator )
      {
         SDB_OSS_DEL (*pOperator) ;
         *pOperator = NULL ;
      }
   }

   INT32 _rtnOperatorFactory::_getQueryType( const BSONObj &query,
                                             rtnQueryType &qType )
   {
      INT32 rc = SDB_OK ;

      qType = RTN_QUERY_NORMAL ;

      BSONElement ele = query.firstElement() ;
      if ( Object == ele.type() )
      {
         BSONElement subEle = ele.Obj().firstElement() ;
         if ( 0 == ossStrcmp( FIELD_NAME_TEXT, subEle.fieldName() ) )
         {
            if ( 1 != query.nFields() )
            {
               rc = SDB_INVALIDARG ;
               PD_LOG( PDERROR, "Only one query condition should be specified "
                       "for text search, actually: %d", query.nFields() ) ;
               goto error ;
            }

            qType = RTN_QUERY_TEXT ;
            if ( ( String != subEle.type() ) && ( Object != subEle.type() ) )
            {
               rc = SDB_INVALIDARG ;
               PD_LOG( PDERROR, "Query conditioin type[%d] for text "
                       "search is wrong", subEle.type() ) ;
               goto error ;
            }
         }
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   rtnOperatorFactory* rtnGetOperatorFactory()
   {
      static rtnOperatorFactory s_factory ;
      return &s_factory ;
   }
}

