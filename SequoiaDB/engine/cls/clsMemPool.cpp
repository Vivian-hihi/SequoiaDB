/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = clsMemPool.hpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          27/11/2012  Xu Jianhui  Initial Draft

   Last Changed =

*******************************************************************************/

#include "clsMemPool.hpp"
#include "ossMem.hpp"
#include "pd.hpp"
#include "pdTrace.hpp"
#include "clsTrace.hpp"

namespace engine
{
   _clsMemPool::_clsMemPool()
   :_totalMemSize( 0 )
   {
   }

   _clsMemPool::~_clsMemPool()
   {
   }

   INT32 _clsMemPool::initialize()
   {
      return SDB_OK ;
   }

   INT32 _clsMemPool::final()
   {
      if ( 0 != totalSize() )
      {
         PD_LOG( PDERROR, "MemPool has memory leak: %llu", totalSize() ) ;
      }
      return SDB_OK ;
   }

   void _clsMemPool::clear ()
   {
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__CLSMEMPOL_ALLOC, "_clsMemPool::alloc" )
   CHAR *_clsMemPool::alloc( UINT32 size, UINT32 &assignSize )
   {
      PD_TRACE_ENTRY ( SDB__CLSMEMPOL_ALLOC );
      CHAR *pBuffer = (CHAR *)SDB_OSS_MALLOC ( size ) ;
      if ( pBuffer )
      {
         assignSize = size ;
         _totalMemSize.add( assignSize ) ;
      }
      else
      {
         assignSize = 0 ;
         PD_LOG ( PDERROR, "Failed to allocate memory" ) ;
      }

      PD_TRACE_EXIT ( SDB__CLSMEMPOL_ALLOC );
      return pBuffer ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__CLSMEMPOL_RELEASE, "_clsMemPool::release" )
   void _clsMemPool::release( CHAR* pBuff, UINT32 size )
   {
      PD_TRACE_ENTRY ( SDB__CLSMEMPOL_RELEASE );
      if ( pBuff && size > 0 )
      {
         SDB_OSS_FREE ( pBuff ) ;
         _totalMemSize.sub( size ) ;
      }
      PD_TRACE_EXIT ( SDB__CLSMEMPOL_RELEASE );
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__CLSMEMPOL_REALLOC, "_clsMemPool::realloc" )
   CHAR *_clsMemPool::realloc ( CHAR* pBuff, UINT32 srcSize,
                                UINT32 needSize, UINT32 &assignSize )
   {
      PD_TRACE_ENTRY ( SDB__CLSMEMPOL_REALLOC );
      CHAR *p = NULL ;
      if ( srcSize >= needSize )
      {
         assignSize = srcSize ;
         p = pBuff ;
         goto done ;
      }

      release ( pBuff, srcSize );

      p = alloc ( needSize, assignSize ) ;
   done :
      PD_TRACE_EXIT ( SDB__CLSMEMPOL_REALLOC );
      return p ;
   }
}

