package com.sequoiadb.gb18030;

import com.sequoiadb.base.ConfigOptions;
import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.sequoiadb.base.ClientCharset;
import com.sequoiadb.base.ClientCharsetEnum;
import com.sequoiadb.testcommon.SdbTestBase;

/**
 * @Descreption seqDB-33920:ClientOption方法测试
 * @Author chenzejia
 * @CreateDate 2023/12/21
 * @UpdateUser
 * @UpdateDate
 * @UpdateRemark
 * @Version
 */
public class GB18030_33920 extends SdbTestBase {
    private ClientCharset charset;
    private ConfigOptions options;
    private ClientCharset optionsCharset;

    @BeforeClass
    public void setUp() {
        charset = new ClientCharset();
        options = new ConfigOptions();
    }

    @Test
    public void test() {
        // check default charset
        optionsCharset = options.getCharset();
        Assert.assertEquals( optionsCharset, null );

        // clientCharset: GB18030, resultsCharset: GB18030
        charset.setCharsets( ClientCharsetEnum.GB18030 );
        options.setCharset( charset );
        optionsCharset = options.getCharset();
        Assert.assertEquals( optionsCharset.getClientCharset(),
                ClientCharsetEnum.GB18030 );
        Assert.assertEquals( optionsCharset.getResultsCharset(),
                ClientCharsetEnum.GB18030 );

        // clientCharset: GB18030, resultsCharset: UTF-8
        charset.setResultsCharset( ClientCharsetEnum.UTF_8 );
        options.setCharset( charset );
        optionsCharset = options.getCharset();
        Assert.assertEquals( optionsCharset.getClientCharset(),
                ClientCharsetEnum.GB18030 );
        Assert.assertEquals( optionsCharset.getResultsCharset(),
                ClientCharsetEnum.UTF_8 );

        // clientCharset: UTF-8, resultsCharset: UTF-8
        charset.setClientCharset( ClientCharsetEnum.UTF_8 );
        options.setCharset( charset );
        optionsCharset = options.getCharset();
        Assert.assertEquals( optionsCharset.getClientCharset(),
                ClientCharsetEnum.UTF_8 );
        Assert.assertEquals( optionsCharset.getResultsCharset(),
                ClientCharsetEnum.UTF_8 );

        // clientCharset: UTF-8, resultsCharset: GB18030
        charset.setResultsCharset( ClientCharsetEnum.GB18030 );
        options.setCharset( charset );
        optionsCharset = options.getCharset();
        Assert.assertEquals( optionsCharset.getClientCharset(),
                ClientCharsetEnum.UTF_8 );
        Assert.assertEquals( optionsCharset.getResultsCharset(),
                ClientCharsetEnum.GB18030 );
    }

    @AfterClass
    public void tearDown() {
    }
}
