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
import org.bson.BSONObject;

import java.io.UnsupportedEncodingException;
import java.nio.ByteBuffer;

public class QueryRequest extends SdbRequest {
    private static final int FIXED_LENGTH = 88;
    private static final int version = 1;
    private static final short w = 0;
    private static final short padding = 0;
    private final int flag;
    private final long skipNum;
    private final long returnedNum;
    private byte[] clNameBytes;
    private byte[] matcherBytes;
    private byte[] selectorBytes;
    private byte[] orderBytes;
    private byte[] hintBytes;
    private final String collectionName;
    private final BSONObject matcher;
    private final BSONObject selector;
    private final BSONObject orderBy;
    private final BSONObject hint;

    public QueryRequest(String collectionName,
                        BSONObject matcher, BSONObject selector, BSONObject orderBy, BSONObject hint,
                        long skipNum, long returnedNum, int flag) {
        opCode = MsgOpCode.QUERY_REQ;
        length = FIXED_LENGTH;

        this.flag = flag;
        this.skipNum = skipNum;
        if (returnedNum < 0) {
            returnedNum = -1;
        }
        this.returnedNum = returnedNum;

        if (collectionName == null || collectionName.length() == 0) {
            throw new BaseException(SDBError.SDB_INVALIDARG, "Collection name or command is null or empty");
        }
        this.collectionName = collectionName;

        this.matcher = matcher;
        this.selector = selector;
        this.orderBy = orderBy;
        this.hint = hint;
    }

    @Override
    protected void encodeWithCharset(String charset) {
        try {
            this.clNameBytes = collectionName.getBytes(charset);
            length += Helper.alignedSize(this.clNameBytes.length + 1);
        }catch (UnsupportedEncodingException e) {
            throw new BaseException(SDBError.SDB_INVALIDARG, e);
        }

        if (matcher == null) {
            matcherBytes = EMPTY_BSON_BYTES.clone();
            length += ALIGNED_EMPTY_BSON_LENGTH;
        } else {
            matcherBytes = Helper.encodeBSONObj(matcher, charset);
            length += Helper.alignedSize(matcherBytes.length);
        }

        if (selector == null) {
            selectorBytes = EMPTY_BSON_BYTES.clone();
            length += ALIGNED_EMPTY_BSON_LENGTH;
        } else {
            selectorBytes = Helper.encodeBSONObj(selector, charset);
            length += Helper.alignedSize(selectorBytes.length);
        }

        if (orderBy == null) {
            orderBytes = EMPTY_BSON_BYTES.clone();
            length += ALIGNED_EMPTY_BSON_LENGTH;
        } else {
            orderBytes = Helper.encodeBSONObj(orderBy, charset);
            length += Helper.alignedSize(orderBytes.length);
        }

        if (hint == null) {
            hintBytes = EMPTY_BSON_BYTES.clone();
            length += ALIGNED_EMPTY_BSON_LENGTH;
        } else {
            hintBytes = Helper.encodeBSONObj(hint, charset);
            length += Helper.alignedSize(hintBytes.length);
        }
    }

    @Override
    protected void writeMsgBody(ByteBuffer out) {
        out.putInt(version);
        out.putShort(w);
        out.putShort(padding);
        out.putInt(flag);
        out.putInt(clNameBytes.length);
        out.putLong(skipNum);
        out.putLong(returnedNum);
        out.put(clNameBytes);
        out.put((byte) 0);
        int length = clNameBytes.length + 1;
        int paddingLen = Helper.alignedSize(length) - length;
        Helper.fillZero(out, paddingLen);
        writeBSONBytes(matcherBytes, out);
        writeBSONBytes(selectorBytes, out);
        writeBSONBytes(orderBytes, out);
        writeBSONBytes(hintBytes, out);
    }
}
