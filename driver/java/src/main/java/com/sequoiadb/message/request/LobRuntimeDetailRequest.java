package com.sequoiadb.message.request;

import java.nio.ByteBuffer;

import com.sequoiadb.message.MsgOpCode;

public class LobRuntimeDetailRequest extends LobRequest {

    public LobRuntimeDetailRequest(long contextID) {
        opCode = MsgOpCode.LOB_GETRTDETAIL_REQ;
        this.contextId = contextID;
    }

    @Override
    protected void writeLobBody(ByteBuffer out) {
        // no lob body
    }

    @Override
    protected void encodeWithCharset(String charset) {
        // no data
    }
}