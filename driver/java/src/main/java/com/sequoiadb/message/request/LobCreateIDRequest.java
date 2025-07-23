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

   Source File Name = LobCreateIDRequest.java

   Descriptive Name = N/A

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who         Description
   ====== =========== =========== =============================================
          23/07/2025  fangjiabin  Initial Draft

   Last Changed =

*******************************************************************************/
package com.sequoiadb.message.request;

import java.nio.ByteBuffer;

import org.bson.BSONObject;
import org.bson.BasicBSONObject;

import com.sequoiadb.message.MsgOpCode;
import com.sequoiadb.util.Helper;

public class LobCreateIDRequest extends LobRequest {
    private byte[] bsonBytes;
    private final BSONObject obj;

    public LobCreateIDRequest(BSONObject obj) {
        opCode = MsgOpCode.LOB_CREATEID_REQ;

        if (obj == null) {
            obj = new BasicBSONObject();
        }
        this.obj = obj;
    }

    @Override
    protected void writeLobBody(ByteBuffer out) {
        writeBSONBytes(bsonBytes, out);
    }

    @Override
    protected void encodeWithCharset(String charset) {
        bsonBytes = Helper.encodeBSONObj(obj, charset);
        bsonLength = bsonBytes.length;
        length += Helper.alignedSize(bsonBytes.length);
    }
}
