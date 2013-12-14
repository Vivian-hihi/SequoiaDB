/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = spdSession.hpp

   Descriptive Name =

   When/how to use: this program may be used on binary and text-formatted
   versions of Replication component. This file contains structure for
   replication control block.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          09/14/2012  YW  Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef SPDSESSION_HPP_
#define SPDSESSION_HPP_

#include "core.hpp"
#include "../bson/bson.h"

using namespace bson ;

namespace engine
{
   class _spdFuncDownloader ;
   class _pmdEDUCB ;
   class _spdFMPMgr ;
   class _spdFMP ;

   class _spdSession : public SDBObject
   {
   public:
      _spdSession() ;
      virtual ~_spdSession() ;

   public:
      INT32 eval( const BSONObj &procedures,
                  _spdFuncDownloader *downloader,
                  _pmdEDUCB *cb ) ;

      INT32 next( BSONObj &obj ) ;

      const BSONObj &getErrMsg() { return _errmsg ; }
      const BSONObj &getRetMsg() { return _resmsg ; }
      INT32 resType() const { return _resType ; }
   private:
      INT32 _eval( const BSONObj &procedures,
                   _spdFuncDownloader *downloader ) ;

      INT32 _resIsOk( const BSONObj &res ) ;

   private:
      BSONObj _resmsg ;
      BSONObj _errmsg ;
      INT32 _resType ;
      _spdFMPMgr *_fmpMgr ;
      _pmdEDUCB *_cb ;
      _spdFMP *_fmp ;
   } ;

   typedef class _spdSession spdSession ;
}

#endif

