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

import java.io.UnsupportedEncodingException;
import java.nio.ByteBuffer;

public class SQLRequest extends SdbRequest {
    private byte[] sqlBytes;
    private final String sql;

    public SQLRequest(String sql) {
        opCode = MsgOpCode.SQL_REQ;
        this.sql = sql;
    }

    @Override
    protected void writeMsgBody(ByteBuffer out) {
        out.put(sqlBytes);
        out.put((byte) 0);
    }

    @Override
    protected void encodeWithCharset(String charset) {
        try {
            this.sqlBytes = sql.getBytes(charset);
        } catch (Exception e) {
            throw new BaseException(SDBError.SDB_INVALIDARG, e);
        }

        length += sqlBytes.length + 1;
    }
}