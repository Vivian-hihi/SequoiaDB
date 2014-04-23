#include "qgmOptiMthMatchSelect.hpp"

using namespace bson;

namespace engine
{
   qgmOptiMthMatchSelect::qgmOptiMthMatchSelect( _qgmPtrTable *pTable,
                                                _qgmParamTable *pParam )
   :_qgmOptiSelect( pTable, pParam )
   {
   }

   qgmOptiMthMatchSelect::~qgmOptiMthMatchSelect()
   {
   }

   BOOLEAN qgmOptiMthMatchSelect::isEmpty()
   {
      return FALSE;
   }

   INT32 qgmOptiMthMatchSelect::fromBson( const BSONObj &matcher )
   {
      _matcher = matcher;
      return SDB_OK;
   }

   INT32 qgmOptiMthMatchSelect::_extend( _qgmOptiTreeNode *&exNode )
   {
      INT32 rc = SDB_OK;
      rc = this->qgmOptiSelect::_extend( exNode );
      PD_RC_CHECK( rc, PDERROR,
                  "extend failed(rc=%d)", rc );
      _type = QGM_OPTI_TYPE_SCAN == _type ?
               QGM_OPTI_TYPE_MTHMCHSCAN : QGM_OPTI_TYPE_MTHMCHFILTER ;
   done:
      return rc;
   error:
      goto done;
   }
}
