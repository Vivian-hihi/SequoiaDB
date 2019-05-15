package com.sequoiadb.fulltext;

import java.util.ArrayList;
import java.util.Date;
import java.util.List;

import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import org.bson.util.JSON;
import org.testng.SkipException;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;
import org.testng.Assert;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.testcommon.CommLib;
import com.sequoiadb.testcommon.SdbTestBase;
import com.sequoiadb.testcommon.SdbThreadBase;
import com.sequoiadb.utils.FullTextDBUtils;
import com.sequoiadb.utils.FullTextESUtils;
import com.sequoiadb.utils.FullTextUtils;
import com.sequoiadb.utils.StringUtils;

import org.elasticsearch.client.*;

/**
 * FileName: CurdProcessingIndex14376.java test content:
 * 正在处理固定集合中记录，插入/修改/删除/查询集合中的记录
 * 
 * @author liuxiaoxuan
 * @Date 2018.11.21
 */
public class Fulltext14376 extends SdbTestBase {

    private Sequoiadb sdb = null;
    private CollectionSpace cs = null;
    private DBCollection cl = null;
    private String clName = "ES_14376";
    private Client esClient = null;
    private List< String > esIndexNames = null;

    @BeforeClass
    public void setUp() {
        esClient = FullTextESUtils.createTransportClient( esHostName,
                Integer.parseInt( esServiceName ) );
        sdb = new Sequoiadb( SdbTestBase.coordUrl, "", "" );
        if ( CommLib.isStandAlone( sdb ) ) {
            throw new SkipException( "skip StandAlone" );
        }

        // create cl
        cs = sdb.getCollectionSpace( csName );
        cl = cs.createCollection( clName );
    }

    @AfterClass
    public void tearDown() {
        FullTextDBUtils.dropCollection( cs, clName );
        // check fulltext deleted
        if ( esIndexNames != null ) {
            FullTextUtils.checkIndexNotExistInES( esClient, esIndexNames );
        }
        sdb.close();
        esClient.close();
    }

    @Test
    public void test() {
        // create fulltext
        String textIndexName = "fulltext14376";
        BSONObject indexObj = new BasicBSONObject();
        indexObj.put( "a", "text" );
        indexObj.put( "b", "text" );
        indexObj.put( "c", "text" );
        indexObj.put( "d", "text" );
        indexObj.put( "e", "text" );
        indexObj.put( "f", "text" );
        indexObj.put( "g", "text" );
        cl.createIndex( textIndexName, indexObj, false, false );

        esIndexNames = FullTextDBUtils.getESIndexNames( sdb, csName, clName,
                textIndexName );

        insertData( cl, FullTextUtils.INSERT_NUMS );

        // insert/update/delete while index processing cappedcl data
        InsertThread insertThread = new InsertThread();
        UpdateThread updateThread = new UpdateThread();
        RemoveThread removeThread = new RemoveThread();

        insertThread.start();
        updateThread.start();
        removeThread.start();

        Assert.assertTrue( insertThread.isSuccess(),
                insertThread.getErrorMsg() );
        Assert.assertTrue( updateThread.isSuccess(),
                updateThread.getErrorMsg() );
        Assert.assertTrue( removeThread.isSuccess(),
                removeThread.getErrorMsg() );

        // check consistency after insert/update/delete
        FullTextUtils.checkFullSyncToES( esClient, sdb, csName, clName,
                textIndexName, ( int ) cl.getCount() );
        FullTextUtils.checkDataConsistency( sdb, csName, clName,
                textIndexName );
    }

    public void insertData( DBCollection cl, int insertNums ) {
        List< BSONObject > insertObjs = new ArrayList<>();
        for ( int i = 0; i < 100; i++ ) {
            for ( int j = 0; j < insertNums / 100; j++ ) {
                insertObjs.add( ( BSONObject ) JSON.parse( "{a: 'test_14376_"
                        + i * j + "', b: '"
                        + StringUtils.getRandomString( 32 ) + "', c: '"
                        + StringUtils.getRandomString( 64 ) + "', d: '"
                        + StringUtils.getRandomString( 64 ) + "', e: '"
                        + StringUtils.getRandomString( 128 ) + "', f: '"
                        + StringUtils.getRandomString( 128 ) + "', g: "
                        + i * j + "}" ) );
            }
            cl.insert( insertObjs, 0 );
            insertObjs.clear();
        }
    }

    private class InsertThread extends SdbThreadBase {

        @Override
        public void exec() throws Exception {
            Sequoiadb db = null;
            DBCollection cl = null;
            db = new Sequoiadb( SdbTestBase.coordUrl, "", "" );
            cl = db.getCollectionSpace( csName ).getCollection( clName );
            int insertNums = 100000;
            insertData( cl, insertNums );
            db.close();
        }
    }

    private class UpdateThread extends SdbThreadBase {

        @Override
        public void exec() throws Exception {
            Sequoiadb db = null;
            DBCollection cl = null;
            db = new Sequoiadb( SdbTestBase.coordUrl, "", "" );
            cl = db.getCollectionSpace( csName ).getCollection( clName );

            BSONObject modifier = new BasicBSONObject();
            BSONObject value = new BasicBSONObject();
            BSONObject matcher = new BasicBSONObject();
            BSONObject subMatcher = new BasicBSONObject();
            value.put( "g", "-1" );
            modifier.put( "$set", value );
            subMatcher.put( "$lt", 100000 );
            matcher.put( "g", subMatcher );
            cl.update( matcher, modifier, null );
            db.close();
        }
    }

    private class RemoveThread extends SdbThreadBase {

        @Override
        public void exec() throws Exception {
            Sequoiadb db = null;
            DBCollection cl = null;
            db = new Sequoiadb( SdbTestBase.coordUrl, "", "" );
            cl = db.getCollectionSpace( csName ).getCollection( clName );

            BSONObject matcher = new BasicBSONObject();
            BSONObject subMatcher = new BasicBSONObject();
            subMatcher.put( "$gt", 100000 );
            matcher.put( "g", subMatcher );
            cl.delete( matcher );
            db.close();
        }
    }

}
