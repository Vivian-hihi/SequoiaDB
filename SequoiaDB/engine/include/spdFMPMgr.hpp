/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = spdFMPMgr.hpp

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

#ifndef SPDFMPMGR_HPP_
#define SPDFMPMGR_HPP_

#include "ossLatch.hpp"

#include <list>
#include <set>

namespace engine
{
   class _pmdEDUCB ;
   class _spdFMP ;

   class _spdFMPMgr : public SDBObject
   {
   public:
      _spdFMPMgr() ;
      virtual ~_spdFMPMgr() ;

   public:
      INT32 init() ;
      INT32 getFMP( _spdFMP *&fmp ) ;
      INT32 returnFMP( _spdFMP *fmp, _pmdEDUCB *cb ) ;
      BOOLEAN isProcedureUsr( const CHAR *usr ) ;

   private:
      INT32 _createNewFMP( _spdFMP *&fmp ) ;

   private:
      std::list<_spdFMP *> _pool ;
      std::set<std::string> _usrTable ;
      ossSpinXLatch _mtx ;
      CHAR *_startBuf ;
      INT32 _allocated ;
   } ;

   typedef class _spdFMPMgr spdFMPMgr ;
}

#endif

