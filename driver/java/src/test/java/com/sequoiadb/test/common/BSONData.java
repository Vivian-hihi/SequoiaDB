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

   Source File Name = BSONData.java

   Descriptive Name = N/A

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who         Description
   ====== =========== =========== =============================================
          23/07/2025  fangjiabin  Initial Draft

   Last Changed =

*******************************************************************************/
package com.sequoiadb.test.common;

import org.bson.BSON;
import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import org.bson.types.*;
import org.bson.util.DateInterceptUtil;
import org.junit.Assert;

import java.nio.charset.StandardCharsets;
import java.util.Date;
import java.util.regex.Pattern;

public class BSONData {

    public static BSONObject getAllTypeBSONObj(String baseStr) {
        return getAllTypeBSONObj(baseStr, true);
    }

    public static BSONObject getAllTypeBSONObj(String baseStr, boolean hasOID) {
        Date date = DateInterceptUtil.interceptDate(new Date(), "yyyy-MM-dd");
        BSONDate bsonDate = new BSONDate(date.getTime());

        BSONObject list = new BasicBSONList();
        list.put("1", 123);
        list.put("2", 12345L);
        list.put("3", 123.456);
        list.put("4", baseStr);
        list.put("5", new Binary(BSON.B_GENERAL, baseStr.getBytes(StandardCharsets.UTF_8)));
        if (hasOID) {
            list.put("6", new ObjectId());
        }
        list.put("7", true);
        list.put("8", date);
        list.put("9", bsonDate);
        list.put("10", null);
        list.put("11", Pattern.compile("^a", BSON.regexFlags("i")));
        list.put("12", new Code(baseStr));
//        list.put("13", new Symbol(BASE_STR));
        list.put("14", new CodeWScope("hello", new BasicBSONObject("a", baseStr)));
        list.put("15", new BSONTimestamp((int) (date.getTime() / 1000), 1234));
        list.put("16", new BSONDecimal("12345678901234567890.09876543210987654321"));
        list.put("17", new MaxKey());
        list.put("18", new MinKey());

        BSONObject obj = new BasicBSONObject();
        obj.put("int", 123);
        obj.put("long", 12345L);
        obj.put("double", 123.456);
        obj.put("string", baseStr);
        obj.put("binary", new Binary(BSON.B_GENERAL, baseStr.getBytes(StandardCharsets.UTF_8)));
        if (hasOID) {
            obj.put("oid", new ObjectId());
        }
        obj.put("true", true);
        obj.put("date", date);
        obj.put("bsonDate", bsonDate);
        obj.put("null", null);
        obj.put("regex", Pattern.compile("^a", BSON.regexFlags("i")));
        obj.put("code", new Code(baseStr));
//        obj.put("symbol", new Symbol(BASE_STR));
        obj.put("codewscope", new CodeWScope("hello", new BasicBSONObject("a", baseStr)));
        obj.put("timestamp", new BSONTimestamp((int) (date.getTime() / 1000), 1234));
        obj.put("decimal", new BSONDecimal("12345678901234567890.09876543210987654321"));
        obj.put("maxKey", new MaxKey());
        obj.put("minKey", new MinKey());

        BSONObject data = new BasicBSONObject();
        data.put("int", 123);
        data.put("long", 12345L);
        data.put("double", 123.456);
        data.put("string", baseStr);
        data.put("object", obj);
        data.put("array", list);
        data.put("binary", new Binary(BSON.B_GENERAL, baseStr.getBytes(StandardCharsets.UTF_8)));
        if (hasOID) {
            data.put("oid", new ObjectId());
        }
        data.put("true", true);
        data.put("date", date);
        data.put("bsonDate", bsonDate);
        data.put("null", null);
        data.put("regex", Pattern.compile("^a", BSON.regexFlags("i")));
        data.put("code", new Code(baseStr));
//        data.put("symbol", new Symbol(BASE_STR));
        data.put("codewscope", new CodeWScope("hello", new BasicBSONObject("a", baseStr)));
        data.put("timestamp", new BSONTimestamp((int) (date.getTime() / 1000), 1234));
        data.put("decimal", new BSONDecimal("12345678901234567890.09876543210987654321"));
        data.put("maxKey", new MaxKey());
        data.put("minKey", new MinKey());

        return data;
    }

    public static void checkBSONObj(BSONObject expected, BSONObject actual) {
        String errorMsg = "expected: " + expected.toString();
        errorMsg += ", actual" + actual.toString();

        Assert.assertEquals(errorMsg, expected.keySet().size(), actual.keySet().size());

        for (String k: expected.keySet()) {
            errorMsg = "error key: " + k;

            Object v1 = expected.get(k);
            Object v2 = actual.get(k);

            if (v1 instanceof Date && v2 instanceof  Date) {
                Date d1 = (Date) v1;
                Date d2 = (Date) v2;

                Assert.assertEquals(d1.getTime(), d2.getTime());
            } else if (v1 instanceof BSONObject && v2 instanceof BSONObject) {
                checkBSONObj((BSONObject)v1, (BSONObject)v2);
            } else if (v1 != null && v2 != null) {
                Assert.assertEquals(errorMsg, v1.toString(), v2.toString());
            } else {
                Assert.assertEquals(errorMsg, v1, v2);
            }
        }
    }
}
