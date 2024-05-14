/*******************************************************************************


   Copyright (C) 2011-2018 SequoiaDB Ltd.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU Affero General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Affero General Public License for more details.

   You should have received a copy of the GNU Affero General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.

   Source File Name = netCompressor.hpp

   Descriptive Name =

   When/how to use: this program may be used on binary and text-motionatted
   versions of PD component. This file contains declare of PD functions.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who         Description
   ====== =========== =========== ==============================================
          01/02/2024  fangjiabin  Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef NET_COMPRESSOR_HPP_
#define NET_COMPRESSOR_HPP_

#include "msg.h"
#include "netDef.hpp"
#include "utilCompression.hpp"
#include "utilPooledObject.hpp"

namespace engine
{
   class _netMsgCompressor : public utilPooledObject
   {
   public:
      _netMsgCompressor() ;
      ~_netMsgCompressor() ;

      INT32 compressNetMsg( const MsgHeader *message, const CHAR* body, UINT32 bodyLen,
                            CHAR** headerDes, CHAR** bodyDes,
                            UINT32 &newHeaderLen, UINT32 &newBodyLen ) ;

      INT32 compressNetMsg( const MsgHeader *message, UINT32 messageLen,
                            CHAR** des, UINT32 &messageLenDes ) ;

      INT32 compressNetMsg( const MsgHeader *message, const netIOVec &iov,
                            CHAR** headerDes, netIOVec &iovDes ) ;

      INT32 decompressNetMsg( const MsgHeader *message, CHAR** des ) ;

      void setCompressor( UTIL_COMPRESSOR_TYPE netCompressor ) ;

      void setNeedReleaseUncompressBuff( BOOLEAN need ) ;

   private:
      INT32 _allocBuff( UINT32 len, CHAR **ppBuff, UINT32 *pRealSize ) ;
      CHAR* _getBuff( UINT32 desLen, CHAR** buff, UINT32 &realLen ) ;

      BOOLEAN _needCompressMsg( const MsgHeader *message ) ;
      BOOLEAN _isCompressedMsg( const MsgHeader *message ) ;

   private:
      CHAR   *_pCompressBuff ;
      UINT32  _compressBuffLen ;

      CHAR   *_pUncompressBuff ;
      UINT32  _uncompressBuffLen ;

      CHAR   *_pTmpCompressBuff ;
      UINT32  _tmpCompressBuffLen ;

      UTIL_COMPRESSOR_TYPE _compressor ;

      BOOLEAN _needReleaseUncompressBuff ;
   } ;

}

#endif