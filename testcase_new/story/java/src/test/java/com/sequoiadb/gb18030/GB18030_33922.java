package com.sequoiadb.gb18030;

import com.sequoiadb.base.*;
import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.sequoiadb.testcommon.SdbTestBase;

/**
 * @Descreption seqDB-33922:设置字符集GB18030创建连接，执行库表操作
 * @Author chenzejia
 * @CreateDate 2023/12/21
 * @UpdateUser
 * @UpdateDate
 * @UpdateRemark
 * @Version
 */
public class GB18030_33922 extends SdbTestBase {
    private Sequoiadb sdb;
    private CollectionSpace cs;
    private DBCollection cl;
    private String[] csNames = { "集合空间33922", "侤侫侭侰33922", "㑓㑔㑕㑖33922",
            "郎凉33922" };
    private String[] clNames = { "集合33922", "獲獳獴獵33922", "〢〣〤33922",
            "㒳㒴㒵㒶33922" };

    @BeforeClass
    public void setUp() {
        ClientCharset charset = new ClientCharset();
        charset.setCharsets( ClientCharsetEnum.GB18030 );
        sdb = GB18030Utils.createCharsetClient( coordUrl, charset );
        for ( String csName : csNames ) {
            if ( sdb.isCollectionSpaceExist( csName ) ) {
                sdb.dropCollectionSpace( csName );
            }
        }
    }

    @Test
    public void test() {
        for ( String csName : csNames ) {
            for ( String clName : clNames ) {
                // create cs
                cs = sdb.createCollectionSpace( csName );
                Assert.assertEquals( sdb.getCollectionSpace( csName ).getName(),
                        csName,
                        "create cs which name is " + csName + " failed" );
                // create cl
                cl = cs.createCollection( clName );
                Assert.assertEquals( cs.getCollection( clName ).getName(),
                        clName,
                        "create cl which name is " + clName + " failed" );
                // rename cl
                cs.renameCollection( clName, "新" + clName );
                Assert.assertEquals( cs.getCollection( "新" + clName ).getName(),
                        "新" + clName,
                        "rename cl which name is " + clName + " failed" );
                // rename cs
                sdb.renameCollectionSpace( csName, "新" + csName );
                Assert.assertEquals(
                        sdb.getCollectionSpace( "新" + csName ).getName(),
                        "新" + csName,
                        "rename cs which name is " + csName + " failed" );
                // redefine cs and cl
                cs = sdb.getCollectionSpace( "新" + csName );
                cl = cs.getCollection( "新" + clName );
                // check cl function
                GB18030Utils.checkClFunction( cl );
                // drop cl
                cs.dropCollection( "新" + clName );
                Assert.assertFalse( cs.isCollectionExist( "新" + clName ),
                        "drop cl which name is 新" + clName + " failed" );
                // drop cs
                sdb.dropCollectionSpace( "新" + csName );
                Assert.assertFalse( sdb.isCollectionSpaceExist( "新" + csName ),
                        "drop cs which name is 新" + csName + " failed" );
            }
        }
    }

    @AfterClass
    public void tearDown() {
        for ( String csName : csNames ) {
            if ( sdb.isCollectionSpaceExist( csName ) ) {
                sdb.dropCollectionSpace( csName );
            }
            if ( sdb.isCollectionSpaceExist( "新" + csName ) ) {
                sdb.dropCollectionSpace( "新" + csName );
            }
        }
        sdb.close();
    }
}
