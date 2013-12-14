/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = spdCoordDownloader.hpp

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

#ifndef SPDCOORDDOWNLOADER_HPP_
#define SPDCOORDDOWNLOADER_HPP_

#include "spdFuncDownloader.hpp"
#include "rtnContext.hpp"

namespace engine
{
   class rtnCoordCommand ;
   class _SDB_RTNCB ;

   class _spdCoordDownloader : public _spdFuncDownloader
   {
   public:
      _spdCoordDownloader( rtnCoordCommand *command,
                           _pmdEDUCB *cb ) ;
      virtual ~_spdCoordDownloader() ;

   public:
      virtual INT32 next( BSONObj &func ) ;
      virtual INT32 download( const BSONObj &matcher ) ;

   private:
      _rtnContextBuf _context ;
      SINT64 _contextID ;
      rtnCoordCommand *_command ;
      _pmdEDUCB *_cb ;
      _SDB_RTNCB *_rtnCB ;
   } ;

   typedef class _spdCoordDownloader spcCoordDownloader ;
}

#endif

