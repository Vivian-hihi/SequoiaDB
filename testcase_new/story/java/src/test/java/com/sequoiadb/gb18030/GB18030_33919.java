package com.sequoiadb.gb18030;

import com.sequoiadb.base.ClientCharset;
import com.sequoiadb.base.ClientCharsetEnum;
import com.sequoiadb.testcommon.SdbTestBase;
import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

/**
 * @Descreption seqDB-33919:ClientCharset方法测试
 * @Author chenzejia
 * @CreateDate 2023/12/21
 * @UpdateUser
 * @UpdateDate
 * @UpdateRemark
 * @Version
 */
public class GB18030_33919 extends SdbTestBase {
    @BeforeClass
    public void setUp() {
    }

    @Test
    public void test() {
        // check default charset
        ClientCharset charset = new ClientCharset();
        Assert.assertEquals( charset.getClientCharset(),
                ClientCharsetEnum.UTF_8 );
        Assert.assertEquals( charset.getResultsCharset(),
                ClientCharsetEnum.UTF_8 );

        // check setCharsets function
        charset.setCharsets( ClientCharsetEnum.GB18030 );
        Assert.assertEquals( charset.getClientCharset(),
                ClientCharsetEnum.GB18030 );
        Assert.assertEquals( charset.getResultsCharset(),
                ClientCharsetEnum.GB18030 );
        String expected = "ClientCharset{clientCharset=GB18030, resultCharset=GB18030}";
        Assert.assertEquals( charset.toString(), expected );

        charset.setCharsets( ClientCharsetEnum.UTF_8 );
        Assert.assertEquals( charset.getClientCharset(),
                ClientCharsetEnum.UTF_8 );
        Assert.assertEquals( charset.getResultsCharset(),
                ClientCharsetEnum.UTF_8 );
        expected = "ClientCharset{clientCharset=UTF_8, resultCharset=UTF_8}";
        Assert.assertEquals( charset.toString(), expected );

        // check setClientCharset function
        charset.setClientCharset( ClientCharsetEnum.GB18030 );
        Assert.assertEquals( charset.getClientCharset(),
                ClientCharsetEnum.GB18030 );
        charset.setClientCharset( ClientCharsetEnum.UTF_8 );
        Assert.assertEquals( charset.getClientCharset(),
                ClientCharsetEnum.UTF_8 );

        // check setResultsCharset function
        charset.setResultsCharset( ClientCharsetEnum.GB18030 );
        Assert.assertEquals( charset.getResultsCharset(),
                ClientCharsetEnum.GB18030 );
        charset.setResultsCharset( ClientCharsetEnum.UTF_8 );
        Assert.assertEquals( charset.getResultsCharset(),
                ClientCharsetEnum.UTF_8 );
    }

    @AfterClass
    public void tearDown() {
    }
}
