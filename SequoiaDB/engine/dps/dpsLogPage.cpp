/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = dpsLogPage.cpp

   Descriptive Name = Data Protection Services Log Page

   When/how to use: this program may be used on binary and text-formatted
   versions of DPS component. This file contains code logic for log page
   operations

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          09/14/2012  TW  Initial Draft

   Last Changed =

*******************************************************************************/
#include "dpsLogPage.hpp"
#include "pdTrace.hpp"
#include "dpsTrace.hpp"
namespace engine
{
   _dpsLogPage::_dpsLogPage()
   {
   //   _dpsLogPage( DPS_DEFAULT_PAGE_SIZE );
      // delete in destructor
      _mb = SDB_OSS_NEW _dpsMessageBlock( DPS_DEFAULT_PAGE_SIZE );
      if ( NULL == _mb )
      {
         pdLog(PDERROR, __FUNC__, __FILE__, __LINE__,
               "new _dpsMessageBlock failed!");
      }
      _startPage = DPS_LSN_START_FROM_HEAD;
   }

   _dpsLogPage::_dpsLogPage( UINT32 size )
   {
      // delete in destructor
      _mb = SDB_OSS_NEW _dpsMessageBlock( size );
      if ( NULL == _mb )
      {
         pdLog(PDERROR, __FUNC__, __FILE__, __LINE__,
               "new _dpsMessageBlock failed!");
      }

      _startPage = DPS_LSN_START_FROM_HEAD;
   }

   _dpsLogPage::~_dpsLogPage()
   {
      lock();
      if ( _mb )
         SDB_OSS_DEL _mb;
      _mb = NULL;
      unlock();
   }
/*
   // insert something into the log page
   INT32 _dpsLogPage::insert( const CHAR *src, UINT32 len )
   {
      INT32 rc = SDB_OK;
      UINT64 offset = 0;
      // preallocate space from the page
      rc = allocate( len, offset );
      if ( rc )
      {
         pdLog ( PDERROR, __FUNC__, __FILE__, __LINE__,
                 "Failed to allocate %d from offset %lld, rc = %d",
                 len, offset, rc ) ;
         goto error;
      }
      // fill up the data into given offset
      rc = fill( offset, src, len );
      if ( rc )
      {
         pdLog ( PDERROR, __FUNC__, __FILE__, __LINE__,
                 "Failed to fill, rc = %d", rc ) ;
         goto error ;
      }
   done :
      return rc ;
   error :
      goto done ;
   }
*/
   // fill up data into log page
   PD_TRACE_DECLARE_FUNCTION ( SDB__DPSLGPAGE, "_dpsLogPage::fill" )
   INT32 _dpsLogPage::fill( UINT32 offset, const CHAR *src, UINT32 len )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB__DPSLGPAGE );
      if ( !_mb )
      {
         _mb = SDB_OSS_NEW _dpsMessageBlock( DPS_DEFAULT_PAGE_SIZE );
         if ( NULL == _mb )
         {
            pdLog(PDERROR, __FUNC__, __FILE__, __LINE__,
                  "new _dpsMessageBlock failed!");
            rc = SDB_OOM ;
            goto error ;
         }
      }
      ossMemcpy ( _mb->offset( offset ), src, len ) ;
   done :
      PD_TRACE_EXITRC ( SDB__DPSLGPAGE, rc );
      return rc ;
   error :
      goto done ;
   }

   // reserve space from a given page
   PD_TRACE_DECLARE_FUNCTION ( SDB__DPSLGPAGE2, "_dpsLogPage::allocate" )
   INT32 _dpsLogPage::allocate( UINT32 len )
   {
      INT32 rc = SDB_OK;
      PD_TRACE_ENTRY ( SDB__DPSLGPAGE2 );
      if ( !_mb )
      {
         _mb = SDB_OSS_NEW _dpsMessageBlock( DPS_DEFAULT_PAGE_SIZE );
         if ( NULL == _mb )
         {
            pdLog(PDERROR, __FUNC__, __FILE__, __LINE__,
                  "new _dpsMessageBlock failed!");
            rc = SDB_OOM ;
            goto error ;
         }
      }
      // make sure we get enough space
      SDB_ASSERT ( getLastSize() >= len, "len is greater than buffer size" )
      // change current writing position
      _mb->writePtr( len + _mb->length() );

   done :
      PD_TRACE_EXITRC ( SDB__DPSLGPAGE2, rc );
      return rc ;
   error :
      goto done ;
   }

   // allocate and return the offset
   INT32 _dpsLogPage::allocate( UINT32 len, UINT64 &offset )
   {
      offset = getLength();

      return allocate(len);
   }
}

