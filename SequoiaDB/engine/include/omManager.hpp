/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = omManager.hpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          04/15/2014  XJH Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef OM_MANAGER_HPP__
#define OM_MANAGER_HPP__

#include "omDef.hpp"
#include "clsObjBase.hpp"
#include "ossLatch.hpp"
#include "pmdRestSession.hpp"
#include "restAdaptor.hpp"

#include <vector>
#include <string>
#include <map>

using namespace std ;

namespace engine
{

   /*
      _omManager define
   */
   class _omManager : public _clsObjBase
   {
      DECLARE_OBJ_MSG_MAP()

      public:
         _omManager() ;
         virtual ~_omManager() ;

         virtual INT32    initialize() ;
         virtual INT32    active () ;
         virtual INT32    deactive() ;
         virtual INT32    final() ;

         CHAR*       allocFixBuf() ;
         INT32       getFixBufSize() const { return _fixBufSize ; }
         void        releaseFixBuf( CHAR *pBuff ) ;

         restSessionInfo*  attachSessionInfo( const string &id ) ;
         void              detachSessionInfo( restSessionInfo *pSessionInfo ) ;
         void              invalidSessionInfo( restSessionInfo *pSessionInfo ) ;

         restSessionInfo*  newSessionInfo( const string &userName,
                                           UINT32 localIP ) ;

         restAdaptor*      getRestAdptor() { return &_restAdptor ; }

      protected:

      private:
         vector< CHAR* >                        _vecFixBuf ;
         const INT32                            _fixBufSize ;

         map<string, restSessionInfo*>          _mapSessions ;
         map<string, vector<restSessionInfo*> > _mapUser2Sessions ;

         ossSpinSLatch                          _omLatch ;

         restAdaptor                            _restAdptor ;

         // configure info
         INT32                                  _maxRestBodySize ;
         INT32                                  _restTimeout ;

   } ;
   typedef _omManager omManager ;

   /*
      get the global om manager object point
   */
   omManager *sdbGetOMManager() ;

}

#endif // OM_MANAGER_HPP__

