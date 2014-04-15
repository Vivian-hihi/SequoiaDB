/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = spdFMPMgr.cpp

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

#include "spdFMPMgr.hpp"
#include "spdFMP.hpp"
#include "ossProc.hpp"
#include "ossUtil.hpp"
#include "fmpDef.hpp"

#define SPD_POOL_HIGH_WATERMARK 5

namespace engine
{
   _spdFMPMgr::_spdFMPMgr()
   :_startBuf(NULL),
    _allocated( 0 )
   {

   }

   _spdFMPMgr::~_spdFMPMgr()
   {
      while ( TRUE )
      {
         _mtx.get() ;
         INT32 allocated = _allocated ;
         if ( 0 == allocated )
         {
            _mtx.release() ;
            break ;
         }
         else  if ( (UINT32)allocated != _pool.size() )
         {
            _mtx.release() ;
            PD_LOG( PDINFO, "not all fmp are returned:%d", allocated ) ;
            ossSleepmillis(500) ;
         }
         else
         {
            while ( !_pool.empty() )
            {
               _spdFMP *fmp = _pool.back() ;
               _pool.pop_back() ;
               SAFE_OSS_DELETE( fmp ) ;
               --_allocated ;
            }
            _mtx.release() ;
            break ;
         }
      }

      SDB_ASSERT( 0 == _allocated, "impossible" )

      if ( NULL != _startBuf )
      {
         SDB_OSS_FREE( _startBuf ) ;
      }
   }

   INT32 _spdFMPMgr::init()
   {
      INT32 rc = SDB_OK ;
      CHAR path[OSS_MAX_PATHSIZE + 1] = { 0 } ;
      UINT32 pathLen = 0 ;
      UINT32 appendLen = 0 ;

      rc = ossGetEWD( path, OSS_MAX_PATHSIZE ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to get file path:%d",rc ) ;
         goto error ;
      }

      appendLen = ossStrlen(SPD_PROCESS_NAME) + 1 ;
      pathLen = ossStrlen( path ) ;
      if ( OSS_MAX_PATHSIZE < appendLen + pathLen )
      {
         PD_LOG( PDERROR, "file path is too long:%s", path ) ;
         rc = SDB_SYS ;
         goto error ;
      }

      _startBuf = (CHAR *)SDB_OSS_MALLOC(pathLen + appendLen + 1) ; // +1 for '\0'
      if ( NULL == _startBuf )
      {
         PD_LOG( PDERROR, "failed to allocate mem.") ;
         rc = SDB_OOM ;
         goto error ;
      }

      ossMemcpy( _startBuf, path, pathLen ) ;
      ossMemcpy( _startBuf + pathLen, OSS_FILE_SEP, 1 ) ; // append '/'
      ossMemcpy( _startBuf + pathLen + 1, SPD_PROCESS_NAME,
                 appendLen ) ; // include '\0'

      PD_LOG( PDEVENT, "sub process path:%s", _startBuf ) ;

   done:
      return rc ;
   error:
      goto done ;
   }

   BOOLEAN _spdFMPMgr::isProcedureUsr( const CHAR *usr )
   {
      BOOLEAN rc = FALSE ;
      _mtx.get() ;
      std::set<std::string>::iterator itr = _usrTable.find( usr ) ;
      if ( _usrTable.end() == itr )
      {
         /// do noting.
      }
      else
      {
         _usrTable.erase( itr ) ;
         rc = TRUE ;
      }
      _mtx.release() ;
      return rc ;
   }

   INT32 _spdFMPMgr::getFMP( _spdFMP *&fmp )
   {
      INT32 rc = SDB_OK ;
      _spdFMP *got = NULL ;
      _mtx.get() ;
      if ( _pool.empty() )
      {
         _mtx.release() ;

         rc = _createNewFMP( got ) ;
         if ( SDB_OK != rc )
         {
            PD_LOG( PDERROR, "failed to create new fmp:%d",rc ) ;
            goto error ;
         }
         _mtx.get() ;
         ++_allocated ;
//         ++got->_useTimes ;
//         _usrTable.insert( got->getTmpUsr() ) ;
         _mtx.release() ;
      }
      else
      {
         got = _pool.back() ;
//         ++got->_useTimes ;
//         _usrTable.insert( got->getTmpUsr() ) ;
         _pool.pop_back() ;
         _mtx.release() ;
      }

      fmp = got ;
   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _spdFMPMgr::_createNewFMP( _spdFMP *&fmp )
   {
      INT32 rc = SDB_OK ;
      ossResultCode result ;
      _spdFMP *fmpNode =  SDB_OSS_NEW _spdFMP() ;
      if ( NULL == fmpNode )
      {
         PD_LOG( PDERROR, "failed to allocate mem." ) ;
         rc = SDB_OOM ;
         goto error ;
      }

      rc = ossExec ( _startBuf, _startBuf, NULL,
                     0, fmpNode->_id,
                     result, &( fmpNode->_in), &( fmpNode->_out) ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to start fmp node:%d", rc ) ;
         goto error ;
      }

      fmp = fmpNode ;
   done:
      return rc ;
   error:
      if ( NULL != fmpNode )
      {
         SDB_OSS_DEL fmpNode ;
      }
      goto done ;
   }

   INT32 _spdFMPMgr::returnFMP( _spdFMP *fmp, _pmdEDUCB *cb )
   {
      INT32 rc = SDB_OK ;
      INT32 poolSize = 0 ;
      INT32 allocateSize = 0 ;
      SDB_ASSERT( NULL != fmp && -1 != fmp->id(), "impossible" )

       _mtx.get() ;
      if ( fmp->discarded() ||
           SPD_POOL_HIGH_WATERMARK <= _pool.size() )
      {
         --_allocated ;
         poolSize = _pool.size() ;
         allocateSize = _allocated ;
         SDB_ASSERT( 0 <= _allocated, "impossible" )
         _mtx.release() ;
         rc = fmp->quit( cb ) ;
         if ( SDB_OK != rc )
         {
            PD_LOG( PDERROR, "failed to quit fmp:%d",rc ) ;
         }
         SAFE_OSS_DELETE( fmp ) ;
      }
      else
      {
         _mtx.release() ;
         rc = fmp->reset( cb ) ;
         if ( SDB_OK != rc )
         {
            PD_LOG( PDERROR, "failed to reset fmp:%d",rc ) ;
            _mtx.get() ;
            --_allocated ;
            poolSize = _pool.size() ;
            allocateSize = _allocated ;
            _mtx.release() ;
            SAFE_OSS_DELETE( fmp ) ;
            goto error ;
         }
         else
         {
            _mtx.get() ;
            _pool.push_back( fmp ) ;
            poolSize = _pool.size() ;
            allocateSize = _allocated ;
            _mtx.release() ;
         }
      }

      PD_LOG( PDDEBUG, "pool size:%d, allocate size:%d",
                          poolSize, allocateSize ) ;
   done:
      return rc ;
   error:
      goto done ;
   }
}
