package com.sequoiadb.gb18030;

import java.util.ArrayList;
import java.util.List;

import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import org.bson.types.MaxKey;
import org.bson.types.MinKey;
import org.testng.Assert;
import org.testng.SkipException;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.sequoiadb.base.*;
import com.sequoiadb.testcommon.CommLib;
import com.sequoiadb.testcommon.SdbTestBase;

/**
 * @Descreption seqDB-33929:设置字符集GB18030创建连接，执行集合挂载操作
 * @Author chenzejia
 * @CreateDate 2023/12/21
 * @UpdateUser
 * @UpdateDate
 * @UpdateRemark
 * @Version
 */
public class GB18030_33929 extends SdbTestBase {
    private Sequoiadb sdb;
    private CollectionSpace cs;
    private DBCollection mCL;
    private DBCollection sCL1;
    private DBCollection sCL2;
    private String csName = "集合空间33929";
    private String mCLName = "主集合33929";
    private String sCLName1 = "子集合1";
    private String sCLName2 = "子集合2";
    private String sCL1FullName = csName + "." + sCLName1;
    private String sCL2FullName = csName + "." + sCLName2;
    private String shardKeyName = "字符串";
    private List< BSONObject > data = new ArrayList< BSONObject >();

    @BeforeClass
    public void setUp() {
        ClientCharset charset = new ClientCharset();
        charset.setCharsets( ClientCharsetEnum.GB18030 );
        sdb = GB18030Utils.createCharsetClient( coordUrl, charset );
        if ( CommLib.isStandAlone( sdb ) ) {
            throw new SkipException( "skip standAlone mode" );
        }
        if ( sdb.isCollectionSpaceExist( csName ) ) {
            sdb.dropCollectionSpace( csName );
        }
        cs = sdb.createCollectionSpace( csName );
        for ( int i = 0; i < 100; i++ ) {
            data.add( new BasicBSONObject( shardKeyName, "测a试b数c据" + i ) );
        }
    }

    @Test
    public void test() {
        // create Main Collection
        BSONObject options = new BasicBSONObject();
        options.put( "IsMainCL", true );
        options.put( "ShardingKey", new BasicBSONObject( shardKeyName, 1 ) );
        options.put( "ShardingType", "range" );
        mCL = cs.createCollection( mCLName, options );

        // create Sub Collection
        sCL1 = cs.createCollection( sCLName1 );
        sCL2 = cs.createCollection( sCLName2 );

        // attach
        options = new BasicBSONObject();
        options.put( "LowBound",
                new BasicBSONObject( shardKeyName, new MinKey() ) );
        options.put( "UpBound",
                new BasicBSONObject( shardKeyName, "测a试b数c据50" ) );
        mCL.attachCollection( sCL1FullName, options );

        options = new BasicBSONObject();
        options.put( "LowBound",
                new BasicBSONObject( shardKeyName, "测a试b数c据50" ) );
        options.put( "UpBound",
                new BasicBSONObject( shardKeyName, new MaxKey() ) );
        mCL.attachCollection( sCL2FullName, options );

        // insert data
        mCL.bulkInsert( data );

        // check attach result
        BSONObject matcher = new BasicBSONObject( shardKeyName,
                new BasicBSONObject( "$lt", "测a试b数c据50" ) );
        DBCursor mCLCursor = mCL.query( matcher, null, null, null );
        checkAttachResult( mCLCursor, sCL1.query() );
        matcher = new BasicBSONObject( shardKeyName,
                new BasicBSONObject( "$gte", "测a试b数c据50" ) );
        mCLCursor = mCL.query( matcher, null, null, null );
        checkAttachResult( mCLCursor, sCL2.query() );
    }

    @AfterClass
    public void tearDown() {
        sdb.dropCollectionSpace( csName );
        sdb.close();
    }

    private void checkAttachResult( DBCursor mCLCursor, DBCursor sCLCursor ) {
        List< BSONObject > expected = new ArrayList<>();
        List< BSONObject > actual = new ArrayList<>();
        while ( mCLCursor.hasNext() ) {
            BSONObject mCLDoc = mCLCursor.getNext();
            expected.add( mCLDoc );
        }
        while ( sCLCursor.hasNext() ) {
            BSONObject sCLDoc = sCLCursor.getNext();
            actual.add( sCLDoc );
        }
        Assert.assertEquals( actual, expected );
        mCLCursor.close();
        sCLCursor.close();
    }
}
