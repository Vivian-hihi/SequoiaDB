package com.sequoiadb.gb18030;

import com.sequoiadb.base.ClientCharset;
import com.sequoiadb.base.ClientCharsetEnum;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.testcommon.SdbTestBase;

/**
 * @Descreption seqDB-33933:设置字符集为utf8创建，执行数据操作
 *              seqDB-33934:输入输出设置不同字符集创建连接，执行数据操作
 * @Author chenzejia
 * @CreateDate 2023/12/21
 * @UpdateUser
 * @UpdateDate
 * @UpdateRemark
 * @Version
 */
public class GB18030_33933_33934 extends SdbTestBase {
    private Sequoiadb sdb;
    private Sequoiadb conn;
    private String csName = "集合空间33933_33934";

    @BeforeClass
    public void setUp() {
        sdb = new Sequoiadb( coordUrl, "", "" );
        if ( sdb.isCollectionSpaceExist( csName ) ) {
            sdb.dropCollectionSpace( csName );
        }
    }

    @Test
    public void test() throws InterruptedException {
        // 创建连接，字符集为utf8
        ClientCharset charset = new ClientCharset();
        charset.setCharsets( ClientCharsetEnum.UTF_8 );
        conn = GB18030Utils.createCharsetClient( coordUrl, charset );
        GB18030Utils.checkConnFunction( conn, csName );

        // 创建连接，clientCharset为utf8,resultsCharset为gb18030
        ClientCharset clientCharset = new ClientCharset();
        clientCharset.setClientCharset( ClientCharsetEnum.UTF_8 );
        clientCharset.setResultsCharset( ClientCharsetEnum.GB18030 );
        conn = GB18030Utils.createCharsetClient( coordUrl, clientCharset );
        GB18030Utils.checkConnFunction( conn, csName );

        // 创建连接，clientCharset为gb18030,resultsCharset为utf8
        ClientCharset resultsCharset = new ClientCharset();
        resultsCharset.setClientCharset( ClientCharsetEnum.GB18030 );
        resultsCharset.setResultsCharset( ClientCharsetEnum.UTF_8 );
        conn = GB18030Utils.createCharsetClient( coordUrl, resultsCharset );
        GB18030Utils.checkConnFunction( conn, csName );
    }

    @AfterClass
    public void tearDown() {
        if ( sdb.isCollectionSpaceExist( csName ) ) {
            sdb.dropCollectionSpace( csName );
        }
        if ( conn != null ) {
            conn.close();
        }
        sdb.close();
    }
}
