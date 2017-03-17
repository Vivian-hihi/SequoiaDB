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

package com.sequoiadb.util;

import com.sequoiadb.exception.BaseException;
import com.sequoiadb.exception.SDBError;
import org.bson.BSON;
import org.bson.types.BSONDecimal;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;

public class Helper {

    public static int byteToInt(byte[] byteArray, boolean endianConvert) {
        if (byteArray == null) {
            throw new BaseException(SDBError.SDB_INVALIDARG);
        }

        ByteBuffer bb = ByteBuffer.wrap(byteArray);
        if (endianConvert) {
            bb.order(ByteOrder.LITTLE_ENDIAN);
        } else {
            bb.order(ByteOrder.BIG_ENDIAN);
        }

        return bb.getInt();
    }

    public static int byteToInt(byte[] array, int begin) {
        if (array == null)
            throw new BaseException(SDBError.SDB_INVALIDARG);
        ByteBuffer bb = ByteBuffer.allocate(4);
        int count = 0;
        while (count < 4) {
            bb.put(array[begin + count]);
            ++count;
        }
        return bb.getInt(0);
    }

    // l2r means "local to remote", when we build a message
    // for sending to remote, we set l2r to be true; when we
    // receive message from remote, and we need to handle endian,
    // we set l2r to be false
    public static void bsonEndianConvert(byte[] inBytes, int offset,
                                         int objSize, boolean l2r) {
        int begin = offset;
        arrayReverse(inBytes, offset, 4);
        offset += 4;
        byte type;
        while (offset < inBytes.length) {
            // get bson element type
            type = inBytes[offset];
            // move offset to next to skip type
            ++offset;
            if (type == BSON.EOO)
                break;
            // skip element name: '...\0'
            offset += getStrLength(inBytes, offset) + 1;
            switch (type) {
                case BSON.NUMBER:
                    arrayReverse(inBytes, offset, 8);
                    offset += 8;
                    break;
                case BSON.STRING:
                case BSON.CODE:
                case BSON.SYMBOL: {
                    // the first 4 bytes indicate the length of the string
                    // the length of the string is the real length plus 1('\0')
                    int length = Helper.byteToInt(inBytes, offset);
                    arrayReverse(inBytes, offset, 4);
                    int newLength = Helper.byteToInt(inBytes, offset);
                    offset += (l2r ? newLength : length) + 4;
                    break;
                }
                case BSON.OBJECT:
                case BSON.ARRAY: {
                    int length = getBsonLength(inBytes, offset, l2r);
                    bsonEndianConvert(inBytes, offset, length, l2r);
                    offset += length;
                    break;
                }
                case BSON.BINARY: {
                    int length = Helper.byteToInt(inBytes, offset);
                    arrayReverse(inBytes, offset, 4);
                    int newLength = Helper.byteToInt(inBytes, offset);
                    offset += (l2r ? newLength : length) + 5;
                    break;
                }
                case BSON.UNDEFINED:
                case BSON.NULL:
                case BSON.MAXKEY:
                case BSON.MINKEY:
                    break;
                case BSON.OID:
                    offset += 12;
                    break;
                case BSON.BOOLEAN:
                    offset += 1;
                    break;
                case BSON.DATE:
                    arrayReverse(inBytes, offset, 8);
                    offset += 8;
                    break;
                case BSON.REGEX:
                    // string regex
                    offset += getStrLength(inBytes, offset) + 1;
                    // string option
                    offset += getStrLength(inBytes, offset) + 1;
                    break;
                case BSON.REF: {
                    offset += 12;
                    // 4 bytes length + string + 12 bytes
                    int length = Helper.byteToInt(inBytes, offset);
                    arrayReverse(inBytes, offset, 4);
                    int newLength = Helper.byteToInt(inBytes, offset);
                    offset += (l2r ? newLength : length) + 4;
                    offset += 12;
                    break;
                }
                case BSON.CODE_W_SCOPE: {
                    // 4 bytes
                    arrayReverse(inBytes, offset, 4);
                    offset += 4;
                    // string
                    int length = Helper.byteToInt(inBytes, offset);
                    arrayReverse(inBytes, offset, 4);
                    int newLength = Helper.byteToInt(inBytes, offset);
                    offset += (l2r ? newLength : length) + 4;
                    // then object
                    int objLength = getBsonLength(inBytes, offset, l2r);
                    bsonEndianConvert(inBytes, offset, objLength, l2r);
                    offset += objLength;
                    break;
                }
                case BSON.NUMBER_INT:
                    arrayReverse(inBytes, offset, 4);
                    offset += 4;
                    break;
                case BSON.TIMESTAMP:
                    // 2 4-bytes
                    arrayReverse(inBytes, offset, 4);
                    offset += 4;
                    arrayReverse(inBytes, offset, 4);
                    offset += 4;
                    break;
                case BSON.NUMBER_LONG:
                    arrayReverse(inBytes, offset, 8);
                    offset += 8;
                    break;
                case BSON.NUMBER_DECIMAL: {
                    // size(4) + typemod(4) + dscale(2) + weight(2) + digits(2x)
                    // size
                    int length = Helper.byteToInt(inBytes, offset);
                    arrayReverse(inBytes, offset, 4);
                    int newLength = Helper.byteToInt(inBytes, offset);
                    offset += 4;
                    // typemod
                    arrayReverse(inBytes, offset, 4);
                    offset += 4;
                    // dscale
                    arrayReverse(inBytes, offset, 2);
                    offset += 2;
                    // weight
                    arrayReverse(inBytes, offset, 2);
                    offset += 2;
                    // digits
                    int ndigits = ((l2r ? newLength : length) -
                        BSONDecimal.DECIMAL_HEADER_SIZE) / (Short.SIZE / Byte.SIZE);
                    for (int i = 0; i < ndigits; i++) {
                        arrayReverse(inBytes, offset, 2);
                        offset += 2;
                    }
                    break;
                }

            }
        }
        if (offset - begin != objSize) {
            throw new BaseException(SDBError.SDB_INVALIDSIZE);
        }
    }

    private static int getBsonLength(byte[] inBytes, int offset,
                                     boolean endianConvert) {
        byte[] tmp = new byte[4];
        for (int i = 0; i < 4; i++)
            tmp[i] = inBytes[offset + i];

        return Helper.byteToInt(tmp, endianConvert);
    }

    private static void arrayReverse(byte[] array, int begin, int length) {
        int i = begin;
        int j = begin + length - 1;
        byte tmp;
        while (i < j) {
            tmp = array[i];
            array[i] = array[j];
            array[j] = tmp;
            ++i;
            --j;
        }
    }

    private static int getStrLength(byte[] array, int begin) {
        int length = 0;
        while (array[begin] != '\0') {
            ++length;
            ++begin;
        }
        return length;
    }
}
