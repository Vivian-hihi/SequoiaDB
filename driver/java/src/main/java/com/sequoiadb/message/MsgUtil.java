/*
 * Copyright 2017 SequoiaDB Inc.
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

package com.sequoiadb.message;

import com.sequoiadb.exception.BaseException;
import com.sequoiadb.exception.SDBError;
import org.bson.*;

import java.io.ByteArrayInputStream;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.security.MessageDigest;

import static com.sequoiadb.util.Helper.bsonEndianConvert;

/**
 * @since 2.9
 */
public final class MsgUtil {
    private MsgUtil() {
    }

    public static final int ALIGN_SIZE = 4;

    public static String md5(String str) {
        MessageDigest md5;
        try {
            md5 = MessageDigest.getInstance("MD5");
        } catch (Exception e) {
            throw new BaseException(SDBError.SDB_SYS, e);
        }

        char[] charArray = str.toCharArray();
        byte[] byteArray = new byte[charArray.length];
        for (int i = 0; i < charArray.length; i++) {
            byteArray[i] = (byte) charArray[i];
        }

        byte[] md5Bytes = md5.digest(byteArray);
        StringBuilder hexValue = new StringBuilder();
        for (byte md5Byte : md5Bytes) {
            int val = ((int) md5Byte) & 0xff;
            if (val < 16) {
                hexValue.append("0");
            }
            hexValue.append(Integer.toHexString(val));
        }

        return hexValue.toString();
    }

    public static int alignedSize(int original, int alignSize) {
        int size = original + alignSize - 1;
        size -= size % alignSize;
        return size;
    }

    public static int alignedSize(int original) {
        return alignedSize(original, ALIGN_SIZE);
    }

    public static byte[] encodeBSONObj(BSONObject obj) {
        BSONEncoder encoder = new BasicBSONEncoder();
        return encoder.encode(obj);
    }

    public static BSONObject decodeBSONBytes(byte[] bytes) {
        BSONDecoder decoder = new BasicBSONDecoder();
        return decoder.readObject(bytes);
    }

    public static BSONObject decodeBSONObject(ByteBuffer in) {
        int position = in.position();
        int length = in.getInt(position);

        if (in.order() == ByteOrder.BIG_ENDIAN) {
            bsonEndianConvert(in.array(), position, length, false);
        }

        BSONObject obj;
        try {
            BSONDecoder decoder = new BasicBSONDecoder();
            obj = decoder.readObject(new ByteArrayInputStream(in.array(), position, length));
        } catch (IOException e) {
            throw new BaseException(SDBError.SDB_INVALIDARG, e);
        }

        if (length < in.remaining()) {
            int alignedSize = MsgUtil.alignedSize(length);
            in.position(alignedSize + position);
        } else {
            in.position(length + position);
        }

        return obj;
    }

    public static byte[] decodeBSONBytes(ByteBuffer in) {
        int position = in.position();
        int length = in.getInt(position);

        if (in.order() == ByteOrder.BIG_ENDIAN) {
            bsonEndianConvert(in.array(), position, length, false);
        }

        byte[] bytes = new byte[length];
        in.get(bytes);

        if (length < in.remaining()) {
            int alignedSize = MsgUtil.alignedSize(length);
            in.position(alignedSize + position);
        } else {
            in.position(length + position);
        }

        return bytes;
    }
}
