/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = ossTypes.hpp

   Descriptive Name = Operating System Services Types Header

   When/how to use: this program may be used on binary and text-formatted
   versions of OSS component. This file contains declare for data types used in
   SequoiaDB.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          09/14/2012  TW  Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef DPSLOGPAGE_HPP_
#define DPSLOGPAGE_HPP_

#include "core.hpp"
#include "oss.hpp"
#include "ossUtil.hpp"
#include "dpsMessageBlock.hpp"
#include "ossRWMutex.hpp"
#include "dpsLogDef.hpp"
namespace engine
{
   const INT8 DPS_LSN_START_FROM_HEAD = 0;
   const INT8 DPS_LSN_NOT_START_FROM_HEAD = -1;

   class _dpsLogPage : public SDBObject
   {
      private:
         ossRWMutex _mtx;
         _dpsMessageBlock *_mb;
         UINT32 _pageNumber;
         INT8 _startPage;
         DPS_LSN _beginLSN ;

      public:
         _dpsLogPage();

         _dpsLogPage( UINT32 size );

         ~_dpsLogPage();

      public:
         inline UINT32 getLastSize() const
         {
            return _mb->idleSize();
         }

         inline UINT32 getBufSize() const
         {
            return _mb->size();
         }

         inline UINT32 getLength() const
         {
            return _mb->length();
         }

         inline void clear()
         {
            _startPage = DPS_LSN_START_FROM_HEAD ;
            _mb->clear();
            return ;
         }

         inline void setStartFlag( INT8 flag )
         {
            _startPage = flag;
            return;
         }

         inline INT8 getStartFlag() const
         {
            return _startPage;
         }

         inline void setLsn( const DPS_LSN &lsn , UINT32 offset )
         {
            ossMemcpy( _mb->offset( offset ), ( CHAR * )&lsn,
                       sizeof(DPS_LSN) );
            return;
         }

         inline DPS_LSN getLsn( UINT32 offset )
         {
            return *( (DPS_LSN *)_mb->offset( offset ) );
         }

         inline void setBeginLSN ( const DPS_LSN &lsn )
         {
            _beginLSN = lsn  ;
         }
         inline DPS_LSN getBeginLSN ()
         {
            return _beginLSN ;
         }

         inline _dpsMessageBlock *mb()
         {
            return _mb;
         }

         inline void setNumber( UINT32 number )
         {
            _pageNumber = number;
            return;
         }

         inline UINT32 getNumber()
         {
            return _pageNumber;
         }

         inline void lockShared()
         {
            _mtx.lock_r();
            return ;
         }

         inline void unlockShared()
         {
            _mtx.release_r() ;
            return ;
         }

         inline void lock()
         {
            _mtx.lock_w() ;
            return ;
         }

         inline void unlock()
         {
            _mtx.release_w() ;
            return ;
         }

      public:
         //INT32 insert( const CHAR *src, UINT32 len );

         INT32 fill( UINT32 offset, const CHAR *src, UINT32 len );

         INT32 allocate( UINT32 len, UINT64 &offset );

         INT32 allocate( UINT32 len );
   };

   typedef class _dpsLogPage dpsLogPage;
}
#endif

