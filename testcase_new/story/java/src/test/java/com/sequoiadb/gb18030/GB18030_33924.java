package com.sequoiadb.gb18030;

import com.sequoiadb.testcommon.CommLib;
import org.testng.Assert;
import org.testng.SkipException;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.sequoiadb.base.*;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.testcommon.SdbTestBase;

/**
 * @Descreption seqDB-33924:设置字符集GB18030创建连接，执行序列操作
 * @Author chenzejia
 * @CreateDate 2023/12/21
 * @UpdateUser
 * @UpdateDate
 * @UpdateRemark
 * @Version
 */
public class GB18030_33924 extends SdbTestBase {
    private Sequoiadb sdb;
    private String[] sequenceNames = { "序列33924", "炽充冲33924", "㓶㓷㓸33924" };
    private String[] newSequenceNames = { "新序列33924", "础储矗33924", "㔶㔷㔸33924" };

    @BeforeClass
    public void setUp() {
        ClientCharset charset = new ClientCharset();
        charset.setCharsets( ClientCharsetEnum.GB18030 );
        sdb = GB18030Utils.createCharsetClient( coordUrl, charset );
        if ( CommLib.isStandAlone( sdb ) ) {
            throw new SkipException( "skip standAlone mode" );
        }
    }

    @Test
    public void test() {
        for ( int i = 0; i < sequenceNames.length; i++ ) {
            // create sequence
            sdb.createSequence( sequenceNames[ i ] );
            DBSequence sequence = null;
            try {
                sequence = sdb.getSequence( sequenceNames[ i ] );
                Assert.assertEquals( sequence.getName(), sequenceNames[ i ],
                        "the sequence name is not equal to the create sequence name" );
            } catch ( BaseException e ) {
                Assert.fail(
                        "create sequence [" + sequenceNames[ i ] + "] failed" );
            }
            // check sequence
            Assert.assertEquals( sequence.getNextValue(), 1 );
            Assert.assertEquals( sequence.getCurrentValue(), 1 );
            // rename sequence
            try {
                sdb.renameSequence( sequenceNames[ i ], newSequenceNames[ i ] );
            } catch ( BaseException e ) {
                Assert.fail(
                        "rename sequence [" + sequenceNames[ i ] + "] failed" );
            }
            // drop sequence
            try {
                sdb.dropSequence( newSequenceNames[ i ] );
            } catch ( BaseException e ) {
                Assert.fail( "drop sequence [" + newSequenceNames[ i ]
                        + "] failed" );
            }
        }
    }

    @AfterClass
    public void tearDown() {
        sdb.close();
    }
}
