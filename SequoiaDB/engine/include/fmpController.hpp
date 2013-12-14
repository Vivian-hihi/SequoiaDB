/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = fmpController.hpp

   Descriptive Name =

   When/how to use:

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          06/19/2013  YW  Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef FMPCONTROLLER_HPP_
#define FMPCONTROLLER_HPP_

#include "core.hpp"
#include "ossIO.hpp"
#include "../bson/bson.h"

using namespace bson ;

class _fmpVM ;

class _fmpController
{
public:
   _fmpController() ;
   virtual ~_fmpController() ;

public:
   INT32 run() ;

private:
   INT32 _runLoop() ;

   INT32 _handleOneLoop( const BSONObj &obj,
                         INT32 step ) ;

   INT32 _readMsg( BSONObj &msg ) ;

   INT32 _writeMsg( const BSONObj &msg ) ;

   INT32 _createVM( SINT32 type ) ;

   void _clear() ;

private:
   OSSFILE _in ;
   OSSFILE _out ;
   _fmpVM *_vm ;
   CHAR *_inBuf ;
   UINT32 _inBufSize ;
   INT32  _step ;
} ;

typedef class _fmpController fmpController ;

#endif

