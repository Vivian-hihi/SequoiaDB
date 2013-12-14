/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = qgmPlMthMatcherScan.hpp

   Descriptive Name =

   When/how to use: this program may be used on binary and text-formatted
   versions of PMD component. This file contains declare for QGM operators

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          08/29/2013  JHL  Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef QGMPLMTHMATCHERSCAN_HPP__
#define QGMPLMTHMATCHERSCAN_HPP__

#include "qgmPlScan.hpp"

namespace engine
{
   class qgmPlMthMatcherScan : public _qgmPlScan
   {
   public:
      qgmPlMthMatcherScan( const qgmDbAttr &collection,
                           const qgmOPFieldVec &selector,
                           const bson::BSONObj &orderby,
                           const bson::BSONObj &hint,
                           INT64 numSkip,
                           INT64 numReturn,
                           const qgmField &alias,
                           const bson::BSONObj &matcher );

   private:
      virtual INT32 _execute( _pmdEDUCB *eduCB ) ;
   };
}

#endif
