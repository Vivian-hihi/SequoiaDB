/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = spdFMP.hpp

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

#ifndef SPDFMP_HPP_
#define SPDFMP_HPP_

#include "core.hpp"
#include "ossNPipe.hpp"
#include "../bson/bson.h"

using namespace bson ;

namespace engine
{
   class _pmdEDUCB ;

   class _spdFMP : public SDBObject
   {
   public:
      _spdFMP() ;
      virtual ~_spdFMP() ;

   public:
      /// timeout is sec.
      /// user should call getOwned if necessary.
      virtual INT32 read( BSONObj &msg, _pmdEDUCB *cb,
                          BOOLEAN ignoreTimeout = TRUE ) ;

      virtual INT32 write( const BSONObj &msg ) ;

      inline const OSSPID &id() const
      {
         return _id ;
      }

      inline void setDiscarded()
      {
         _discarded = TRUE ;
         return ;
      }

      inline BOOLEAN discarded()const
      {
         return _discarded ;
      }

      INT32 reset( _pmdEDUCB *cb ) ;

      INT32 quit( _pmdEDUCB *cb ) ;

      std::string getTmpUsr() ;

   private:
      INT32 _extendReadBuf() ;

      INT32 _extractMsg( BSONObj &msg, BOOLEAN &extracted ) ;

      inline void _clear()
      {
         _totalRead = 0 ;
         _itr = 0 ;
         _expect = 0 ;
         return ;
      }

   protected:
      OSSNPIPE _in ;
      OSSNPIPE _out ;
      OSSPID _id ;
      BOOLEAN _discarded ;
      CHAR *_readBuf ;
      INT32 _readBufSize ;
      INT32 _totalRead ;
      INT32 _itr ;
      INT32 _expect ;
      INT32 _useTimes ;

      friend class _spdFMPMgr ;
   } ;
   typedef class _spdFMP spdFMP ;
}

#endif

