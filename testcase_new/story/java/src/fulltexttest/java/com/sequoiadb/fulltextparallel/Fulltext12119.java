package com.sequoiadb.fulltextparallel;

import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;
import java.util.List;

import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import org.testng.Assert;
import org.testng.SkipException;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;
import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.testcommon.CommLib;
import com.sequoiadb.testcommon.SdbTestBase;
import com.sequoiadb.threadexecutor.ThreadExecutor;
import com.sequoiadb.threadexecutor.annotation.ExecuteOrder;
import com.sequoiadb.utils.FullTextDBUtils;
import com.sequoiadb.utils.FullTextESUtils;
import com.sequoiadb.utils.FullTextUtils;
import org.elasticsearch.client.*;

/**
 * @FileName seqDB-12119:并发删除集合空间
 * @Author
 * @Date liuxiaoxuan 2019.5.10
 */
public class Fulltext12119 extends SdbTestBase {
    private Sequoiadb db = null;
    private List< CollectionSpace > css = new ArrayList<>();
    private List< DBCollection > cls = new ArrayList<>();
    private List< String > csNames = new ArrayList<>();
    private List< String > clNames = new ArrayList<>();
    private String textIndexName = "fulltext12119";
    private Client esClient = null;
    ThreadExecutor te = new ThreadExecutor();

    @BeforeClass
    public void setUp() {
        esClient = FullTextESUtils.createTransportClient( esHostName,
                Integer.parseInt( esServiceName ) );

        db = new Sequoiadb( SdbTestBase.coordUrl, "", "" );
        if ( CommLib.isStandAlone( db ) ) {
            throw new SkipException( "skip StandAlone" );
        }

        // 创建集合空间和集合，总共两个集合空间，每个集合空间对应2个集合
        for ( int i = 0; i < 2; i++ ) {
            csNames.add( "cs12119_" + i );
            if ( db.isCollectionSpaceExist( csNames.get( i ) ) ) {
                db.dropCollectionSpace( csNames.get( i ) );
            }
            css.add( db.createCollectionSpace( csNames.get( i ) ) );
        }
        for ( int i = 0; i < 4; i++ ) {
            clNames.add( "12119_cl_" + i );
            cls.add( css.get( i % 2 ).createCollection( clNames.get( i ) ) );
        }

        // 插入数据并创建全文索引
        for ( DBCollection cl : cls ) {
            FullTextDBUtils.insertData( cl, 10000 );
        }

        BSONObject indexObj = new BasicBSONObject();
        indexObj.put( "a", "text" );
        cls.get( 0 ).createIndex( textIndexName, indexObj, false, false );
        cls.get( 1 ).createIndex( textIndexName, indexObj, false, false );
    }

    @AfterClass
    public void tearDown() {
        if ( db != null ) {
            db.close();
        }
        if ( esClient != null ) {
            db.close();
        }
    }

    @Test
    public void test() throws Exception {
        Assert.assertTrue( FullTextUtils.isIndexCreated( esClient, cls.get( 0 ),
                textIndexName, 10000 ) );
        Assert.assertTrue( FullTextUtils.isIndexCreated( esClient, cls.get( 1 ),
                textIndexName, 10000 ) );

        String cappedName1 = FullTextDBUtils.getCappedName( cls.get( 0 ),
                textIndexName );
        String cappedName2 = FullTextDBUtils.getCappedName( cls.get( 1 ),
                textIndexName );
        List< String > esIndexNames1 = FullTextDBUtils
                .getESIndexNames( cls.get( 0 ), textIndexName );
        List< String > esIndexNames2 = FullTextDBUtils
                .getESIndexNames( cls.get( 1 ), textIndexName );

        for ( String csName : csNames ) {
            te.addWorker( new DropCSThread( csName ) );
        }

        te.run();

        Assert.assertTrue( FullTextUtils.isIndexDeleted( db, esClient,
                esIndexNames1.get( 0 ), cappedName1 ) );
        Assert.assertTrue( FullTextUtils.isIndexDeleted( db, esClient,
                esIndexNames2.get( 0 ), cappedName2 ) );
    }

    class DropCSThread {
        private String csName = null;

        public DropCSThread( String csName ) {
            this.csName = csName;
        }

        @ExecuteOrder(step = 1, desc = "删除集合空间")
        public void dropCS() {
            System.out.println(
                    this.getClass().getName().toString() + " begin at:"
                            + new SimpleDateFormat( "yyyy-MM-dd HH:mm:ss" )
                                    .format( new Date() ) );
            try ( Sequoiadb sdb = new Sequoiadb( SdbTestBase.coordUrl, "",
                    "" )) {
                sdb.dropCollectionSpace( csName );
            } finally {
                System.out.println(
                        this.getClass().getName().toString() + " end at:"
                                + new SimpleDateFormat( "yyyy-MM-dd HH:mm:ss" )
                                        .format( new Date() ) );
            }
        }
    }
}
