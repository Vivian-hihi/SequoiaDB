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

   Source File Name = ClientCharsetTest.java

   Descriptive Name = N/A

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who         Description
   ====== =========== =========== =============================================
          23/07/2025  fangjiabin  Initial Draft

   Last Changed =

*******************************************************************************/
package com.sequoiadb.test.db;

import com.sequoiadb.base.ClientCharset;
import com.sequoiadb.base.ClientCharsetEnum;
import org.junit.Test;

import static org.junit.Assert.*;

public class ClientCharsetTest {

    @Test
    public void charsetTest() {
        ClientCharset charset = new ClientCharset();

        // case 1: default
        assertEquals(ClientCharsetEnum.UTF_8, charset.getClientCharset());
        assertEquals(ClientCharsetEnum.UTF_8, charset.getResultsCharset());

        // case 2: setCharsets
        charset.setCharsets(ClientCharsetEnum.GB18030);
        assertEquals(ClientCharsetEnum.GB18030, charset.getClientCharset());
        assertEquals(ClientCharsetEnum.GB18030, charset.getResultsCharset());

        // case 3: set client charset
        charset.setClientCharset(ClientCharsetEnum.UTF_8);
        assertEquals(ClientCharsetEnum.UTF_8, charset.getClientCharset());
        assertEquals(ClientCharsetEnum.GB18030, charset.getResultsCharset());

        // case 4: set result charset
        charset.setResultsCharset(ClientCharsetEnum.UTF_8);
        assertEquals(ClientCharsetEnum.UTF_8, charset.getResultsCharset());
    }

    @Test
    public void charsetEnumTest() {
        assertEquals("UTF-8", ClientCharsetEnum.UTF_8.getName());
        assertEquals("GB18030", ClientCharsetEnum.GB18030.getName());
    }

    @Test
    public void hashCodeAndEqualsTest() {
        ClientCharset charset1 = new ClientCharset();
        ClientCharset charset2 = new ClientCharset();
        ClientCharset charset3 = new ClientCharset();
        charset3.setCharsets(ClientCharsetEnum.GB18030);

        assertEquals(charset1.hashCode(), charset2.hashCode());
        assertEquals(charset1, charset2);

        assertNotEquals(charset1.hashCode(), charset3.hashCode());
        assertNotEquals(charset1, charset3);
    }
}
