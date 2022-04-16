package com.sequoiadb.option;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCursor;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.testcommon.FlinkTestBase;
import com.sequoiadb.testcommon.utils.Commlib;
import com.sequoiadb.testcommon.utils.SDBAttribute;
import org.apache.flink.streaming.api.environment.StreamExecutionEnvironment;
import org.apache.flink.table.api.*;
import org.apache.flink.table.api.bridge.java.StreamTableEnvironment;
import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import java.util.ArrayList;
import java.util.List;

/**
 * @descreption seqDB-25288:创建SDB映射表，指定参数pagesize
 *              seqDB-25289:创建SDB映射表，指定参数domain
 *              seqDB-25290:创建SDB映射表，指定参数shardingkey
 *              seqDB-25291:创建SDB映射表，指定参数shardingtype
 *              seqDB-25292:创建SDB映射表，指定参数replsize
 *              seqDB-25293:创建SDB映射表，指定参数compressiontype
 *              seqDB-25294:创建SDB映射表，指定参数autosplit
 * @author YiPan
 * @date 2022/2/11
 * @updateUser
 * @updateDate
 * @updateRemark
 * @version 1.0
 */
public class CreateTable25288to25294 extends FlinkTestBase {
    private StreamTableEnvironment tableEnv;
    private final String csName = "cs_25288";
    private final String clName = "cl_25288";
    private final String filed_A = "test_a";
    private Sequoiadb sdb;
    private final String tableName = "tb_25278";
    private final int pagesize = 8192;
    private final String domain = "domain_25288";
    private String group = null;
    private final BSONObject shardingkey = new BasicBSONObject( "test_a", 1 );
    private final String shardingtype = "hash";
    private final int replsize = -1;
    private final String compressiontype = "snappy";
    private final boolean autosplit = true;

    @BeforeClass
    public void setUp() {
        StreamExecutionEnvironment env = StreamExecutionEnvironment
                .getExecutionEnvironment();
        tableEnv = StreamTableEnvironment.create( env );
        sdb = new Sequoiadb( FlinkTestBase.getCoord(), FlinkTestBase.username,
                FlinkTestBase.password );
        Commlib.dropCS( sdb, csName );
        if ( sdb.isDomainExist( domain ) ) {
            sdb.dropDomain( domain );
        }
        createDomain();
    }

    @Test
    public void test() throws Exception {
        createTable();

        // 根据CATALOG快照校验部分参数
        checkByCatalog();

        // 校验集合空间快照
        checkByCS();

        // 校验domain
        CollectionSpace cl = sdb.getCollectionSpace( csName );
        Assert.assertEquals( cl.getDomainName(), domain );
    }

    @AfterClass
    public void tearDown() {
        try {
            Commlib.dropCS( sdb, csName );
            if ( sdb.isDomainExist( domain ) ) {
                sdb.dropDomain( domain );
            }
        } finally {
            sdb.close();
        }
    }

    private void createDomain() {
        DBCursor cursor = sdb.listReplicaGroups();
        List< String > groups = new ArrayList<>();
        while ( cursor.hasNext() ) {
            BSONObject next = cursor.getNext();
            int role = ( int ) next.get( "Role" );
            if ( role == 0 ) {
                groups.add( ( String ) next.get( "GroupName" ) );
            }
        }
        sdb.createDomain( domain,
                new BasicBSONObject( "Groups", groups.toArray() ) );
    }

    private void checkByCS() {
        DBCursor cursor = sdb.getSnapshot( Sequoiadb.SDB_SNAP_COLLECTIONSPACES,
                new BasicBSONObject( "Name", csName ), null, null );
        try {
            BSONObject snapshot = cursor.getNext();
            Assert.assertEquals( snapshot.get( "PageSize" ), pagesize );
        } finally {
            cursor.close();
        }
    }

    private void checkByCatalog() {
        DBCursor cursor = sdb.getSnapshot( Sequoiadb.SDB_SNAP_CATALOG,
                new BasicBSONObject( "Name", csName + "." + clName ), null,
                null );
        try {
            BSONObject snapshot = cursor.getNext();
            Assert.assertEquals( snapshot.get( "CompressionTypeDesc" ),
                    compressiontype );
            Assert.assertEquals( snapshot.get( "ReplSize" ), replsize );
            Assert.assertEquals( snapshot.get( "ShardingKey" ), shardingkey );
            Assert.assertEquals( snapshot.get( "ShardingType" ), shardingtype );
            Assert.assertEquals( snapshot.get( "AutoSplit" ), autosplit );
        } finally {
            cursor.close();
        }
    }

    private void createTable() throws Exception {
        Schema schema = Schema.newBuilder()
                .column( filed_A, DataTypes.STRING() ).build();
        TableDescriptor.Builder option = Commlib.createBaseTableOption( schema,
                csName, clName );
        TableDescriptor tableDescriptor = option
                .option( SDBAttribute.pagesize, pagesize + "" )
                .option( SDBAttribute.domain, domain )
                // group不能和切分参数同时指定
                // .option( SDBAttribute.group, group )
                .option( SDBAttribute.shardingkey, shardingkey + "" )
                .option( SDBAttribute.shardingtype, shardingtype )
                .option( SDBAttribute.replsize, replsize + "" )
                .option( SDBAttribute.compressiontype, compressiontype )
                .option( SDBAttribute.autosplit, autosplit + "" ).build();
        tableEnv.createTable( tableName, tableDescriptor );
        Commlib.waitJobFinish( tableEnv
                .executeSql( "insert into " + tableName + " values('abc')" ) );
    }

}
