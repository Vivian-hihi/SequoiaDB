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

   Source File Name = BSONCharsetTest.java

   Descriptive Name = N/A

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who         Description
   ====== =========== =========== =============================================
          23/07/2025  fangjiabin  Initial Draft

   Last Changed =

*******************************************************************************/
package com.sequoiadb.test.bson;

import com.sequoiadb.base.ClientCharsetEnum;
import com.sequoiadb.test.common.BSONData;
import org.bson.*;
import org.bson.types.*;
import org.junit.Assert;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;

public class BSONCharsetTest {

    private static BasicBSONEncoder encoder;
    private static NewBSONDecoder decoder;
    private final static String BASE_STR = "你好，world!";

    @BeforeClass
    public static void setConnBeforeClass() throws Exception {
        encoder = new BasicBSONEncoder();
        decoder = new NewBSONDecoder();
    }

    @Before
    public void setUp() throws Exception {
        // clean charset
        encoder.setCharset(null);
        decoder.setCharset(null);
    }

    @Test
    public void charsetNameTest() {
        BSONObject obj = new BasicBSONObject();
        obj.put("a", BASE_STR);

        byte[] expectedBytes = BSON.encode(obj, null, ClientCharsetEnum.UTF_8.getName());
        byte[] actualBytes;
        BSONObject actualObj;

        // case 1: null
        actualBytes = BSON.encode(obj, null, null);
        Assert.assertArrayEquals(expectedBytes, actualBytes);

        actualObj = BSON.decode(actualBytes, null);
        Assert.assertEquals(obj, actualObj);


        // case 2: empty string
        actualBytes = BSON.encode(obj, null, "");
        Assert.assertArrayEquals(expectedBytes, actualBytes);

        actualObj = BSON.decode(actualBytes, "");
        Assert.assertEquals(obj, actualObj);

        // case 3: normal charset
        actualBytes = BSON.encode(obj, null, "GB18030");
        actualObj = BSON.decode(actualBytes, "GB18030");
        Assert.assertEquals(obj, actualObj);

        // case 4: error charset
        try {
            BSON.encode(obj, null, "abc");
            Assert.fail("BSON.encode() should fail");
        } catch (BSONException e) {
            Assert.assertTrue(e.getMessage().startsWith("conversion encoding failed"));
        }

        try {
            BSON.decode(actualBytes, "abc");
            Assert.fail("BSON.decode() should fail");
        } catch (BSONException e) {
            Assert.assertTrue(e.getMessage().startsWith("unsupported encoding"));
        }
    }

    @Test
    public void testBSONSymbol() {
        BSONObject symbolObj = new BasicBSONObject();
        symbolObj.put("symbol", new Symbol(BASE_STR));
        testBSONValue(symbolObj);
    }


    @Test
    public void testBSONKey() {
        String baseStr = "你好，world!";
        BSONObject obj = new BasicBSONObject();

        // case 1: chinese key
        obj.put("你好", baseStr);
        // case 2: mix key
        obj.put(baseStr, baseStr);

        testBSONValue(obj);
    }

    private void testBSONValue(BSONObject data) {
        // case 1: UTF-8
        testBSONValue(data, ClientCharsetEnum.UTF_8);

        // case 2: GB18030
        testBSONValue(data, ClientCharsetEnum.GB18030);
    }

    private void testBSONValue(BSONObject data, ClientCharsetEnum charset) {
        encoder.setCharset(charset.getName());
        decoder.setCharset(charset.getName());

        byte[] bytes = encoder.encode(data);
        BSONObject obj = decoder.readObject(bytes);
        Assert.assertEquals(data, obj);
    }

    @Test
    public void testAllBSONType() {
        BSONObject data = BSONData.getAllTypeBSONObj(BASE_STR);

        // case 1: UTF-8
        byte[] utf8Bytes = BSON.encode(data, null, ClientCharsetEnum.UTF_8.getName());
        BSONObject utf8Obj = BSON.decode(utf8Bytes, ClientCharsetEnum.UTF_8.getName());
        BSONData.checkBSONObj(data, utf8Obj);

        // case 2: GB18030
        byte[] gb18030Bytes = BSON.encode(data, null, ClientCharsetEnum.GB18030.getName());
        BSONObject gb18030Obj = BSON.decode(gb18030Bytes, ClientCharsetEnum.GB18030.getName());
        BSONData.checkBSONObj(data, gb18030Obj);
    }

    @Test
    public void testBasicBSONDecoder() {
        BSONDecoder decoder = new BasicBSONDecoder();

        try {
            decoder.setCharset(ClientCharsetEnum.UTF_8.getName());
            Assert.fail("not support setCharset");
        } catch (UnsupportedOperationException e){
            // do nothing
        }
    }
}
