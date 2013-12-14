/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = authCB.hpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          07/12/2012  YW  Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef AUTHCB_HPP_
#define AUTHCB_HPP_

#include "core.hpp"
#include "oss.hpp"
#include "../bson/bson.h"

using namespace bson ;

namespace engine
{
   class _pmdEDUCB ;

   class _authCB : public SDBObject
   {
   public:
      _authCB() ;
      ~_authCB() ;

   public:
      INT32 init( _pmdEDUCB *cb ) ;

      INT32 createUsr( BSONObj &obj,
                       _pmdEDUCB *cb ) ;

      INT32 removeUsr( BSONObj &obj,
                       _pmdEDUCB *cb ) ;

      INT32 authenticate( BSONObj &obj,
                          _pmdEDUCB *cb ) ;

      INT32 checkNeedAuth( _pmdEDUCB *cb, BOOLEAN forcecheck = FALSE ) ;

      BOOLEAN needAuthenticate()
      {
         return _needAuth ;
      }

   private:
      INT32 _initAuthentication( _pmdEDUCB *cb ) ;
      INT32 _createUsr( BSONObj &obj,
                        _pmdEDUCB *cb ) ;
      INT32 _valid( BSONObj &obj, BOOLEAN notEmpty ) ;
   private:
      BOOLEAN _needAuth ;
   } ;

   typedef class _authCB SDB_AUTHCB ;
}

#endif

