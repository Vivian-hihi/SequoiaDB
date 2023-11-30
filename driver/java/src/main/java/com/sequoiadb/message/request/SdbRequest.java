/*
 * Copyright 2018 SequoiaDB Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.sequoiadb.message.request;

import com.sequoiadb.base.ClientCharsetEnum;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.exception.SDBError;
import com.sequoiadb.message.SdbMsgHeader;
import com.sequoiadb.message.SdbProtocolVersion;
import com.sequoiadb.util.Helper;
import org.bson.BasicBSONObject;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;

import static com.sequoiadb.util.Helper.bsonEndianConvert;

public abstract class SdbRequest extends SdbMsgHeader implements Request {

    protected static final byte[] EMPTY_BSON_BYTES = Helper.encodeBSONObj(new BasicBSONObject(), Helper.ENCODING_TYPE);
    protected static final int ALIGNED_EMPTY_BSON_LENGTH = Helper.alignedSize(EMPTY_BSON_BYTES.length);

    protected SdbRequest() {
        length = HEADER_LENGTH;
    }

    @Override
    public void setRequestId(long requestId) {
        this.requestId = requestId;
    }

    @Override
    public int length() {
        return length;
    }

    @Override
    public int opCode() {
        return opCode;
    }

    @Override
    public void encode(SdbProtocolVersion version, String charset) {
        switch (version) {
            case SDB_PROTOCOL_VERSION_V1:
                // old version only support UTF-8
                encodeWithCharset(ClientCharsetEnum.UTF_8.getName());
                break;
            case SDB_PROTOCOL_VERSION_V2:
                encodeWithCharset(charset);
                break;
            default:
                throw new BaseException(SDBError.SDB_NET_BROKEN_MSG,
                        "Message protocol version error!");
        }
    }

    @Override
    public void writeBuffer(ByteBuffer out, SdbProtocolVersion version) {
        switch (version) {
            case SDB_PROTOCOL_VERSION_V1:
                writeMsgHeaderV1(out);
                break;
            case SDB_PROTOCOL_VERSION_V2:
                writeMsgHeaderV2(out);
                break;
            default:
                throw new BaseException(SDBError.SDB_NET_BROKEN_MSG,
                        "Message protocol version error!");
        }
        writeMsgBody(out);
    }

    protected void writeMsgHeaderV1(ByteBuffer out) {
        int len = length - HEADER_LENGTH + HEADER_LENGTH_V1;
        out.limit( len );
        out.putInt(len);
        out.putInt(opCode);
        out.putInt(tid);
        out.putLong(routeId);
        out.putLong(requestId);
    }

    protected void writeMsgHeaderV2(ByteBuffer out) {
        out.putInt( length );
        out.putInt( eye );
        out.putInt( tid );
        out.putLong( routeId );
        out.putLong( requestId );
        out.putInt( opCode );
        out.putShort( version );
        out.putShort( flags );
        globalID.encode( out );
        out.put( reserve );
    }

    protected abstract void writeMsgBody(ByteBuffer out);

    protected abstract void encodeWithCharset(String charset);

    protected static void writeBSONBytes(byte[] objBytes, ByteBuffer out) {
        if (out.order() == ByteOrder.BIG_ENDIAN) {
            bsonEndianConvert(objBytes, 0, objBytes.length, true);
        }
        out.put(objBytes);

        int paddingLen = Helper.alignedSize(objBytes.length) - objBytes.length;
        Helper.fillZero(out, paddingLen);
    }
}
