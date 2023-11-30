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
