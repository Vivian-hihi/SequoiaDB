package com.sequoiadb.message.request;

import com.sequoiadb.exception.BaseException;
import com.sequoiadb.exception.SDBError;
import com.sequoiadb.message.MsgOpCode;
import com.sequoiadb.util.Helper;

import java.io.UnsupportedEncodingException;
import java.nio.ByteBuffer;

/**
 * Created by tanzhaobo on 2018/4/9.
 */
public class TestRequest extends SdbRequest {
    private String message;

    public TestRequest(String message) {
        length = HEADER_LENGTH;
        opCode = MsgOpCode.MSG_REQ;

        this.message = message == null ? "" : message;
        length += Helper.alignedSize(this.message.length() + 1);
    }

    @Override
    protected void encodeBody(ByteBuffer out) {
        try {
            out.put(message.getBytes("UTF-8"));
            out.put((byte) 0); // end of string
            int length = message.length() + 1;
            int paddingLen = Helper.alignedSize(length) - length;
            if (paddingLen > 0) {
                out.put(new byte[paddingLen]);
            }
        } catch (UnsupportedEncodingException e) {
            throw new BaseException(SDBError.SDB_INVALIDARG, e);
        }
    }
}
