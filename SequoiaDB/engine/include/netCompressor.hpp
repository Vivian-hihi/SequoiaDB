/*******************************************************************************

   Copyright (C) 2011-Present SequoiaDB Ltd.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

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