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
