package com.sequoiadb.gb18030;

import com.sequoiadb.base.*;
import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import org.bson.types.MaxKey;
import org.testng.Assert;
import org.testng.SkipException;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.sequoiadb.testcommon.CommLib;
import com.sequoiadb.testcommon.SdbTestBase;

import java.util.ArrayList;
import java.util.List;

/**
 * @Descreption seqDB-33928:设置字符集GB18030创建连接，执行数据切分操作
 * @Author chenzejia
 * @CreateDate 2023/12/21
 * @UpdateUser
 * @UpdateDate
 * @UpdateRemark
 * @Version
 */
public class GB18030_33928 extends SdbTestBase {
    private Sequoiadb sdb;
    private Sequoiadb srcSdb;
    private Sequoiadb destSdb;
    private CollectionSpace cs;
    private DBCollection hashCL;
    private DBCollection rangeCL;
    private String csName = "集合空间33928";
    private String hashCLName = "h集合33928";
    private String rangeCLName = "r集合33928";
    private String srcGroupName;
    private String destGroupName;
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
        if ( CommLib.OneGroupMode( sdb ) ) {
            throw new SkipException( "skip OneGroupMode" );
        }
        if ( sdb.isCollectionSpaceExist( csName ) ) {
            sdb.dropCollectionSpace( csName );
        }
        cs = sdb.createCollectionSpace( csName );
        ArrayList< String > dataGroupNames = CommLib.getDataGroupNames( sdb );
        srcGroupName = dataGroupNames.get( 0 );
        destGroupName = dataGroupNames.get( 1 );
        srcSdb = sdb.getReplicaGroup( srcGroupName ).getMaster().connect();
        destSdb = sdb.getReplicaGroup( destGroupName ).getMaster().connect();
        for ( int i = 0; i < 100; i++ ) {
            data.add( new BasicBSONObject( shardKeyName, "测a试b数c据" + i ) );
        }
    }

    @Test
    public void test() {
        // create hash collection
        BasicBSONObject options = new BasicBSONObject();
        options.put( "ShardingType", "hash" );
        options.put( "Partition", 4096 );
        options.put( "ShardingKey", new BasicBSONObject( shardKeyName, 1 ) );
        options.put( "Group", srcGroupName );
        hashCL = cs.createCollection( hashCLName, options );
        // split by percent
        hashCL.split( srcGroupName, destGroupName, 50 );
        hashCL.bulkInsert( data );
        // check data distribution
        Assert.assertEquals( srcSdb.getCollectionSpace( csName )
                .getCollection( hashCLName ).getCount(), 50 );
        Assert.assertEquals( destSdb.getCollectionSpace( csName )
                .getCollection( hashCLName ).getCount(), 50 );

        // create range collection
        options.put( "ShardingType", "range" );
        options.put( "ShardingKey", new BasicBSONObject( shardKeyName, 1 ) );
        options.put( "Group", srcGroupName );
        rangeCL = cs.createCollection( rangeCLName, options );
        // split by condition
        BasicBSONObject lowerBound = new BasicBSONObject( shardKeyName,
                "测a试b数c据50" );
        BasicBSONObject upperBound = new BasicBSONObject( shardKeyName,
                new MaxKey() );
        rangeCL.split( srcGroupName, destGroupName, lowerBound, upperBound );
        rangeCL.bulkInsert( data );
        // check data distribution
        Assert.assertEquals( srcSdb.getCollectionSpace( csName )
                .getCollection( rangeCLName ).getCount(), 46 );
        Assert.assertEquals( destSdb.getCollectionSpace( csName )
                .getCollection( rangeCLName ).getCount(), 54 );

    }

    @AfterClass
    public void tearDown() {
        sdb.dropCollectionSpace( csName );
        srcSdb.close();
        destSdb.close();
        sdb.close();
    }
}
