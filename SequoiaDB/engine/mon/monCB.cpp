/******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = monCB.cpp

   Descriptive Name = Monitor Control Block

   When/how to use: this program may be used on binary and text-formatted
   versions of monitoring component. This file contains structure for
   database, application and context snapshot.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          09/14/2012  TW  Initial Draft

   Last Changed =

*******************************************************************************/
#include "monCB.hpp"
#include "pmd.hpp"
namespace engine
{
   _monAppCB::_monAppCB()
   {
      reset () ;
      mondbcb = pmdGetKRCB()->getMonDBCB() ;
   }
   _monAppCB &_monAppCB::operator= ( const _monAppCB &rhs )
   {
      mondbcb                   = pmdGetKRCB()->getMonDBCB() ;
      totalDataRead             = rhs.totalDataRead ;
      totalIndexRead            = rhs.totalIndexRead ;
      totalDataWrite            = rhs.totalDataWrite ;
      totalIndexWrite           = rhs.totalIndexWrite ;

      totalUpdate               = rhs.totalUpdate ;
      totalDelete               = rhs.totalDelete ;
      totalInsert               = rhs.totalInsert ;
      totalSelect               = rhs.totalSelect ;
      totalRead                 = rhs.totalRead ;

      totalReadTime             = rhs.totalReadTime ;
      totalWriteTime            = rhs.totalWriteTime ;
      _connectTimestamp.time    = rhs._connectTimestamp.time;
      _connectTimestamp.microtm = rhs._connectTimestamp.microtm ;
      _connectTimeStampTick     = rhs._connectTimeStampTick ;

      return *this ;
   }
}
