#include "qgmPlMthMatcherScan.hpp"
#include "pdTrace.hpp"
#include "qgmTrace.hpp"

using namespace bson;

namespace engine
{
   qgmPlMthMatcherScan::qgmPlMthMatcherScan( const qgmDbAttr &collection,
                                             const qgmOPFieldVec &selector,
                                             const bson::BSONObj &orderby,
                                             const bson::BSONObj &hint,
                                             INT64 numSkip,
                                             INT64 numReturn,
                                             const qgmField &alias,
                                             const bson::BSONObj &matcher )
   : _qgmPlScan( collection, selector, orderby, hint, numSkip, numReturn,
               alias, NULL )
   {
      _condition = matcher.copy();
   }

   PD_TRACE_DECLARE_FUNCTION( SDB__QGMPLMTHMATCHERSCAN__EXEC, "qgmPlMthMatcherScan::_execute" )
   INT32 qgmPlMthMatcherScan::_execute( _pmdEDUCB *eduCB )
   {
      PD_TRACE_ENTRY( SDB__QGMPLMTHMATCHERSCAN__EXEC ) ;
      INT32 rc = SDB_OK;
      SDB_ASSERT ( _input.size() == 0, "impossible" );

      _invalidPredicate = FALSE ;
      _contextID = -1 ;

      rc = SDB_ROLE_COORD == _dbRole ?
           _executeOnCoord( eduCB ) : _executeOnData( eduCB ) ;

      if ( SDB_RTN_INVALID_PREDICATES == rc )
      {
         rc = SDB_OK ;
         _invalidPredicate = TRUE ;
      }
      else if ( SDB_OK != rc )
      {
         goto error ;
      }
      else
      {
         /// do nothing.
      }

   done:
      PD_TRACE_EXITRC( SDB__QGMPLMTHMATCHERSCAN__EXEC, rc ) ;
      return rc ;
   error:
      goto done ;
   }
}
