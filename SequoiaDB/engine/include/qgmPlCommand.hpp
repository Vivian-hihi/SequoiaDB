/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = qgmPlCommand.hpp

   Descriptive Name =

   When/how to use: this program may be used on binary and text-formatted
   versions of PMD component. This file contains declare for QGM operators

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          04/09/2013  YW  Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef QGMPLCOMMAND_HPP_
#define QGMPLCOMMAND_HPP_

#include "qgmPlan.hpp"
#include "msg.h"
#include "msgDef.h"
#include "rtnCoordCommands.hpp"

namespace engine
{
   class _qgmPlCommand : public _qgmPlan
   {
   public:
      _qgmPlCommand( INT32 type,
                     const qgmDbAttr &fullName,
                     const qgmField &indexName,
                     const qgmOPFieldVec &indexColumns,
                     const BSONObj &partition,
                     BOOLEAN uniqIndex ) ;

      virtual ~_qgmPlCommand() ;

   public:
      virtual void close() ;

      virtual string toString() const ;

   private:
      virtual INT32 _execute( _pmdEDUCB *eduCB ) ;

      virtual INT32 _fetchNext( qgmFetchOut &next ) ;

      INT32 _executeOnData( _pmdEDUCB *eduCB ) ;

      INT32 _executeOnCoord( _pmdEDUCB *eduCB ) ;

      void _killContext() ;

   private:
      INT32 _commandType ;
      INT64 _contextID ;
      qgmDbAttr _fullName ;
      qgmField _indexName ;
      qgmOPFieldVec _indexColumns ;
      BSONObj _partition ;
      BOOLEAN _uniqIndex ;

   } ;

   typedef class _qgmPlCommand qgmPlCommand ;
}

#endif

