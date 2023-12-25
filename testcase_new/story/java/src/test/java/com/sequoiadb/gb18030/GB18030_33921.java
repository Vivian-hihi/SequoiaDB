package com.sequoiadb.gb18030;

import com.sequoiadb.base.Sequoiadb;
import org.bson.BSONObject;
import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.sequoiadb.base.ClientCharset;
import com.sequoiadb.base.ClientCharsetEnum;
import com.sequoiadb.base.ConfigOptions;
import com.sequoiadb.testcommon.SdbTestBase;

/**
 * @Descreption seqDB-33921:设置字符集新建连接，查看字符集设置
 * @Author chenzejia
 * @CreateDate 2023/12/21
 * @UpdateUser
 * @UpdateDate
 * @UpdateRemark
 * @Version
 */
public class GB18030_33921 extends SdbTestBase {
    private ClientCharset charset;
    private ConfigOptions options;
    private Sequoiadb sdb;

    @BeforeClass
    public void setUp() {
        charset = new ClientCharset();
        options = new ConfigOptions();
    }

    @Test
    public void test() {
        // clientCharset: GB18030, resultsCharset: GB18030
        charset.setCharsets( ClientCharsetEnum.GB18030 );
        options.setCharset( charset );
        sdb = new Sequoiadb( coordUrl, "", "", options );
        BSONObject sessionAttr = sdb.getSessionAttr();
        Assert.assertEquals( sessionAttr.get( "ClientCharset" ), "GB18030" );
        Assert.assertEquals( sessionAttr.get( "ResultsCharset" ), "GB18030" );

        // clientCharset: GB18030, resultsCharset: UTF-8
        charset.setResultsCharset( ClientCharsetEnum.UTF_8 );
        options.setCharset( charset );
        sdb = new Sequoiadb( coordUrl, "", "", options );
        sessionAttr = sdb.getSessionAttr();
        Assert.assertEquals( sessionAttr.get( "ClientCharset" ), "GB18030" );
        Assert.assertEquals( sessionAttr.get( "ResultsCharset" ), "UTF8" );

        // clientCharset: UTF-8, resultsCharset: UTF-8
        charset.setClientCharset( ClientCharsetEnum.UTF_8 );
        options.setCharset( charset );
        sdb = new Sequoiadb( coordUrl, "", "", options );
        sessionAttr = sdb.getSessionAttr();
        Assert.assertEquals( sessionAttr.get( "ClientCharset" ), "UTF8" );
        Assert.assertEquals( sessionAttr.get( "ResultsCharset" ), "UTF8" );

        // clientCharset: UTF-8, resultsCharset: GB18030
        charset.setResultsCharset( ClientCharsetEnum.GB18030 );
        options.setCharset( charset );
        sdb = new Sequoiadb( coordUrl, "", "", options );
        sessionAttr = sdb.getSessionAttr();
        Assert.assertEquals( sessionAttr.get( "ClientCharset" ), "UTF8" );
        Assert.assertEquals( sessionAttr.get( "ResultsCharset" ), "GB18030" );
    }

    @AfterClass
    public void tearDown() {
        sdb.close();
    }
}
