package com.sequoiadb.gb18030;

import java.util.ArrayList;

import com.sequoiadb.datasource.DatasourceOptions;
import com.sequoiadb.datasource.SequoiadbDatasource;
import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.sequoiadb.base.*;
import com.sequoiadb.testcommon.SdbTestBase;

/**
 * @Descreption seqDB-33932:连接池设置字符集GB18030，获取连接执行数据操作
 * @Author chenzejia
 * @CreateDate 2023/12/21
 * @UpdateUser
 * @UpdateDate
 * @UpdateRemark
 * @Version
 */
public class GB18030_33932 extends SdbTestBase {
    private Sequoiadb sdb;
    private SequoiadbDatasource ds;
    private Sequoiadb conn;
    private String csName = "集合空间33932";

    @BeforeClass
    public void setUp() {
        sdb = new Sequoiadb( coordUrl, "", "" );
        if ( sdb.isCollectionSpaceExist( csName ) ) {
            sdb.dropCollectionSpace( csName );
        }
        // 连接池参数配置
        DatasourceOptions dsOpt = new DatasourceOptions();
        // 连接参数配置
        ConfigOptions nwOpt = new ConfigOptions();
        ClientCharset charset = new ClientCharset();
        charset.setCharsets( ClientCharsetEnum.GB18030 );
        nwOpt.setCharset( charset );
        // 创建连接池对象
        ArrayList< String > addrs = new ArrayList<>();
        addrs.add( SdbTestBase.coordUrl );
        ds = new SequoiadbDatasource( addrs, "", "", nwOpt, dsOpt );
    }

    @Test
    public void test() throws InterruptedException {
        ds.enableDatasource();
        // enable状态下，获取连接，由用户线程创建
        Assert.assertEquals( ds.getIdleConnNum(), 0 );
        getConnAndCheck( "GB18030" );

        // enable状态下，获取闲置连接，由后台线程创建
        Assert.assertTrue( ds.getIdleConnNum() > 0 );
        getConnAndCheck( "GB18030" );

        // disable状态下，获取连接
        ds.disableDatasource();
        Assert.assertEquals( ds.getIdleConnNum(), 0 );
        getConnAndCheck( "GB18030" );
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

    private void getConnAndCheck( String charset ) throws InterruptedException {
        try {
            conn = ds.getConnection();
            // 检查字符集设置
            Assert.assertEquals( conn.getSessionAttr().get( "ClientCharset" ),
                    charset );
            Assert.assertEquals( conn.getSessionAttr().get( "ResultsCharset" ),
                    charset );
            // 执行DDL，DML操作
            GB18030Utils.checkConnFunction( conn, csName );
        } finally {
            // 释放连接
            ds.releaseConnection( conn );
        }
    }
}
