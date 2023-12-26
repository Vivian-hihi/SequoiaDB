package com.sequoiadb.gb18030;

import com.sequoiadb.exception.BaseException;
import com.sequoiadb.exception.SDBError;
import org.bson.types.ObjectId;
import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.sequoiadb.base.*;
import com.sequoiadb.testcommon.SdbTestBase;

/**
 * @Descreption seqDB-33930:设置字符集GB18030创建连接，执行lob操作
 * @Author chenzejia
 * @CreateDate 2023/12/21
 * @UpdateUser
 * @UpdateDate
 * @UpdateRemark
 * @Version
 */
public class GB18030_33930 extends SdbTestBase {
    private Sequoiadb sdb;
    private CollectionSpace cs;
    private DBCollection cl;
    private String csName = "集合空间33930";
    private String clName = "集合33930";
    String lobData = "abc@#$12跄跖跗跚跞1跎 跏跛㒈null㒉㒊2㒋㒌㒍㒎㐕㐖a㐗㐘㐙b㐚㐛c㐜 㐝㐞";

    @BeforeClass
    public void setUp() {
        ClientCharset charset = new ClientCharset();
        charset.setCharsets( ClientCharsetEnum.GB18030 );
        sdb = GB18030Utils.createCharsetClient( coordUrl, charset );
        if ( sdb.isCollectionSpaceExist( csName ) ) {
            sdb.dropCollectionSpace( csName );
        }
        cs = sdb.createCollectionSpace( csName );
        cl = cs.createCollection( clName );
    }

    @Test
    public void test() {
        // create lob
        DBLob lob = cl.createLob();

        // write gb18030 data
        ObjectId id = lob.getID();
        lob.write( lobData.getBytes() );
        lob.close();

        // read from lob and check
        lob = cl.openLob( id );
        byte[] readData = new byte[ 1024 ];
        int readLen = lob.read( readData );
        lob.close();
        String result = new String( readData, 0, readLen );
        Assert.assertEquals( result, lobData );

        // remove lob
        cl.removeLob( id );
        try {
            cl.openLob( id );
            Assert.fail( "remove lob failed");
        } catch ( BaseException e ) {
            Assert.assertEquals( e.getErrorCode(),
                    SDBError.SDB_FNE.getErrorCode(), "remove lob failed" );
        }
    }

    @AfterClass
    public void tearDown() {
        sdb.dropCollectionSpace( csName );
        sdb.close();
    }
}
