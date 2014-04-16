/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = omManager.cpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          04/15/2014  XJH Initial Draft

   Last Changed =

*******************************************************************************/

#include "omManager.hpp"

namespace engine
{

   /*
      mapping message map
   */
   BEGIN_OBJ_MSG_MAP( _omManager, _clsObjBase )

   END_OBJ_MSG_MAP()

   /*
      implement om manager
   */
   _omManager::_omManager()
   :_fixBufSize( SDB_PAGE_SIZE )
   {
      _maxRestBodySize     = OM_REST_MAX_BODY_SIZE ;
      _restTimeout         = REST_TIMEOUT ;
   }

   _omManager::~_omManager()
   {
      SDB_ASSERT( _vecFixBuf.size() == 0, "Fix buff catch must be empty" ) ;
   }

   INT32 _omManager::initialize ()
   {
      return _restAdptor.init( _fixBufSize, _maxRestBodySize, _restTimeout ) ;
   }

   INT32 _omManager::active ()
   {
      return SDB_OK ;
   }

   INT32 _omManager::deactive ()
   {
      return SDB_OK ;
   }

   INT32 _omManager::final ()
   {
      // release fix buff catch
      _omLatch.get() ;
      for ( UINT32 i = 0 ; i < _vecFixBuf.size() ; ++i )
      {
         SDB_OSS_FREE( _vecFixBuf[i] ) ;
      }
      _vecFixBuf.clear() ;
      _omLatch.release() ;

      // release session info
      restSessionInfo *pSessionInfo = NULL ;
      map<string, restSessionInfo*>::iterator it = _mapSessions.begin() ;
      while( it != _mapSessions.end() )
      {
         pSessionInfo = it->second ;
         pSessionInfo->releaseMem() ;
         SDB_OSS_DEL pSessionInfo ;
         ++it ;
      }
      _mapSessions.clear() ;
      _mapUser2Sessions.clear() ;

      return SDB_OK ;
   }

   CHAR* _omManager::allocFixBuf()
   {
      CHAR *pBuff = NULL ;

      // if fix buff catch is not empty, get from catch
      _omLatch.get() ;
      if ( _vecFixBuf.size() > 0 )
      {
         pBuff = _vecFixBuf.back() ;
         _vecFixBuf.pop_back() ;
      }
      _omLatch.release() ;

      if ( pBuff )
      {
         goto done ;
      }

      // alloc
      pBuff = ( CHAR* )SDB_OSS_MALLOC( OM_FIX_PTR_SIZE( _fixBufSize ) ) ;
      if ( !pBuff )
      {
         PD_LOG( PDERROR, "Alloc fix buff failed, size: %d",
                 OM_FIX_PTR_SIZE( _fixBufSize ) ) ;
         goto error ;
      }
      OM_FIX_PTR_HEADER( pBuff ) = _fixBufSize ;
      pBuff = OM_FIX_PTR_TO_BUFF( pBuff ) ;

   done:
      return pBuff ;
   error:
      goto done ;
   }

   void _omManager::releaseFixBuf( CHAR * pBuff )
   {
      SDB_ASSERT( pBuff, "Buff can't be NULL" ) ;
      SDB_ASSERT( OM_FIX_BUFF_HEADER( pBuff ) == _fixBufSize,
                  "Buff is not alloc by fix buff" ) ;

      // if fix buff catch is not full, push to catch
      _omLatch.get() ;
      if ( _vecFixBuf.size() < OM_FIX_BUFF_CATCH_NUMBER )
      {
         _vecFixBuf.push_back( pBuff ) ;
         pBuff = NULL ;
      }
      _omLatch.release() ;

      SDB_OSS_FREE( OM_FIX_BUFF_TO_PTR( pBuff ) ) ;
   }

   restSessionInfo* _omManager::attachSessionInfo( const string &id )
   {
      restSessionInfo *pSessionInfo = NULL ;

      _omLatch.get_shared() ;
      map<string, restSessionInfo*>::iterator it = _mapSessions.find( id ) ;
      if ( it != _mapSessions.end() )
      {
         pSessionInfo = it->second ;
         pSessionInfo->_inNum.inc() ;
      }
      _omLatch.release_shared() ;

      return pSessionInfo ;
   }

   void _omManager::detachSessionInfo( restSessionInfo * pSessionInfo )
   {
      SDB_ASSERT( pSessionInfo, "Session can't be NULL" ) ;
      pSessionInfo->_inNum.dec() ;
   }

   void _omManager::invalidSessionInfo( restSessionInfo *pSessionInfo )
   {
      SDB_ASSERT( pSessionInfo, "Session can't be NULL" ) ;
      pSessionInfo->_activeTime = 0 ;
   }

   restSessionInfo* _omManager::newSessionInfo( const string &userName,
                                                UINT32 localIP )
   {
      return NULL ;
   }

   /*
      get the global om manager object point
   */
   omManager* sdbGetOMManager()
   {
      static omManager s_omManager ;
      return &s_omManager ;
   }

}


