/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = dmsTempCB.cpp

   Descriptive Name = Data Management Service Temp Table Control Block

   When/how to use: this program may be used on binary and text-formatted
   versions of data management component. This file contains code logic for
   temporary table creation and release.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          09/14/2012  YW  Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef DPSMESSAGEBLOCK_H_
#define DPSMESSAGEBLOCK_H_

#include "core.hpp"
#include "oss.hpp"
#include "pd.hpp"


namespace engine
{
   const UINT32 DPS_MSG_BLOCK_DEF_LEN = 1024 ;

   class _dpsMessageBlock : public SDBObject
   {
   private:
      CHAR *_start;
      CHAR *_write;
      CHAR *_read;
      // size is the size of buffer
      UINT32 _size;
      // length is the size with active user data
      UINT32 _length;

   public:
      _dpsMessageBlock( UINT32 size );
      _dpsMessageBlock( const _dpsMessageBlock &mb ) ;
      _dpsMessageBlock &operator=(const _dpsMessageBlock &mb ) ;
      ~_dpsMessageBlock();
   public:
      inline UINT32 size() const
      {
         return _size;
      }

      inline UINT32 idleSize() const
      {
         return _size - _length;
      }

      inline UINT32 length() const
      {
         return _length;
      }

      inline CHAR *writePtr() const
      {
         return _write;
      }

      inline void writePtr( UINT32 offset )
      {
         _write = _start + offset;
         _length = offset;
         SDB_ASSERT( _length <= _size , "out of mem!" )
         return;
      }

      inline const CHAR *readPtr() const
      {
         return _read;
      }

      inline void readPtr( INT32 offset )
      {
         _read = _start + offset;
      }

      inline CHAR *offset( UINT32 offset )const
      {
         return _start + offset;
      }

      inline void clear()
      {
         _write = _start;
         _read = _start;
         _length = 0;
      }

      inline const CHAR *startPtr() const
      {
         return _start ;
      }

      INT32 extend( UINT32 len );
   };
   typedef class _dpsMessageBlock dpsMessageBlock;
}

#endif
