/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = bps.hpp

   Descriptive Name = Buffer Pool Service Header

   When/how to use: this program may be used on binary and text-formatted
   versions of bufferpool service component. This file is created for further
   extension purpose. It does not contains any meaningful logic at the moment.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          09/14/2012  TW  Initial Draft

   Last Changed =

*******************************************************************************/
#ifndef BPS_HPP_
#define BPS_HPP_

#include "core.hpp"
#include "ossLatch.hpp"
#include "oss.hpp"
#include "ossQueue.hpp"
#include "bpsPrefetch.hpp"
#include "ossAtomic.hpp"

namespace engine
{

   /*
      _bpsCB define
   */
   class _bpsCB : public SDBObject
   {
      private :
         ossQueue<_bpsPreLoadReq*>  _requestQueue ;
         ossQueue<_bpsPreLoadReq*>  _dropBackQueue ;
         ossQueue< bpsDataPref >    _dataPrefetchQueue ;
         INT32                      _numPreLoad ;
         UINT32                     _maxPrefPool ;
         std::vector<EDUID>         _preLoaderList ;

      public:
         ossAtomic32                _curPrefAgentNum ;
         ossAtomic32                _idlePrefAgentNum ;

      private :
         INT32    _addPreLoader () ;

      public :
         OSS_INLINE ossQueue<_bpsPreLoadReq*> *getReqQueue ()
         {
            return &_requestQueue ;
         }
         OSS_INLINE ossQueue<_bpsPreLoadReq*> *getDropQueue ()
         {
            return &_dropBackQueue ;
         }
         OSS_INLINE ossQueue< bpsDataPref > *getPrefetchQueue ()
         {
            return &_dataPrefetchQueue ;
         }
         OSS_INLINE BOOLEAN isPreLoadEnabled () const
         {
            return _preLoaderList.size() > 0 ;
         }
         OSS_INLINE BOOLEAN isPrefetchEnabled () const
         {
            return _maxPrefPool > 0 ? TRUE : FALSE ;
         }
         OSS_INLINE void setNumPreLoads ( UINT32 numPreLoad )
         {
            _numPreLoad = (INT32)numPreLoad ;
         }
         OSS_INLINE void setMaxPrefPool( UINT32 maxPrefPool )
         {
            _maxPrefPool = maxPrefPool ;
         }

      public :
         _bpsCB () :
         _numPreLoad(0), _maxPrefPool(0), _curPrefAgentNum(0),
         _idlePrefAgentNum(0)
         {}

         ~_bpsCB ()
         {
            destroy () ;
         }

         INT32 init () ;

         void destroy () ;

         INT32 sendPreLoadRequest ( const bpsPreLoadReq &request ) ;

         INT32 sendPrefechReq ( const bpsDataPref &request,
                                BOOLEAN inPref = FALSE ) ;

   } ;
   typedef class _bpsCB SDB_BPSCB ;

}

#endif /* BPS_HPP_ */

