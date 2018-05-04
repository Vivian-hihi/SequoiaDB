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
        length += this.message.length() + 1;
    }

    @Override
    protected void encodeBody(ByteBuffer out) {
        try {
            out.put(message.getBytes("UTF-8"));
            out.put((byte) 0); // end of string
            int length = message.length() + 1;
        } catch (UnsupportedEncodingException e) {
            throw new BaseException(SDBError.SDB_INVALIDARG, e);
        }
    }
}
