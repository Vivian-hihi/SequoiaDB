
package com.mongodb.java.serial;

import java.net.UnknownHostException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;

import org.testng.Assert;
import org.testng.ITestContext;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.mongodb.BasicDBObject;
import com.mongodb.CommandResult;
import com.mongodb.DB;
import com.mongodb.DBCollection;
import com.mongodb.DBObject;
import com.mongodb.MongoClient;
import com.mongodb.MongoClientOptions;
import com.mongodb.MongoCommandException;
import com.mongodb.MongoCredential;
import com.mongodb.ServerAddress;
import com.mongodb.utils.MongodbTestBase;

/**
 * @Description seqDB-21934:增删用户
 * @author chimanzhao
 * @version 1.00
 * @Date 2020/6/23
 */

public class Authentication21934 extends MongodbTestBase {
    private MongoClientOptions opt = null;
    private MongoClient client1 = null;
    private DB db1 = null;
    private String username1 = "21934A";
    private String username2 = "21934B";
    private String dbName = "db21934A";
    private String clName = "cl21934";
    private int num = 30;
    private List< DBObject > list;

    @BeforeClass
    public void setup() throws UnknownHostException {
        opt = MongoClientOptions.builder().build();
        client1 = new MongoClient(
                new ServerAddress( config.getHost(), config.getPort() ), opt );
        client1.dropDatabase( dbName );
        db1 = client1.getDB( dbName );
        list = new ArrayList<>();
        for ( int i = 0; i < num; i++ ) {
            list.add( new BasicDBObject( "a", i ).append( "b", "" + i )
                    .append( "c", Arrays.asList( i + 1, i + 2, i + 3 ) )
                    .append( "d", "test-" + i + "-" + i % 3 ).append( "e",
                            new BasicDBObject( "a", 1 ).append( "b", 1 ) ) );
        }
    }

    @Test
    @SuppressWarnings("unchecked")
    public void Test() throws UnknownHostException {
        // 创建多个用户
        db1.command( new BasicDBObject( "createUser", username1 )
                .append( "pwd", username1 )
                .append( "roles", Collections.singletonList( "dbOwner" ) ) );
        // 重复创建用户
        try {
            db1.command( new BasicDBObject( "createUser", username2 )
                    .append( "pwd", username2 ).append( "roles",
                            Collections.singletonList( "dbOwner" ) ) );

            db1.command( new BasicDBObject( "createUser", username2 )
                    .append( "pwd", username2 ).append( "roles",
                            Collections.singletonList( "dbOwner" ) ) );
        } catch ( MongoCommandException e ) {
            if ( e.getErrorCode() != -295 ) {
                throw e;
            }
        }

        // 查询用户
        // 查询所有用户
        CommandResult userInfos = db1
                .command( new BasicDBObject( "usersInfo", 1 ) );
        List< BasicDBObject > users = ( List< BasicDBObject > ) userInfos
                .get( "users" );
        Assert.assertEquals( users,
                Arrays.asList( new BasicDBObject( "user", username1 ),
                        new BasicDBObject( "user", username2 ) ) );
        System.out.println( "users = " + users.toString() );
        System.out.println( "userInfos = " + userInfos.toString() );

        // 查询单个用户
        CommandResult userInfo = db1.command( new BasicDBObject( "usersInfo",
                Collections
                        .singletonList( new BasicDBObject( "user", username1 )
                                .append( "db", dbName ) ) ) );
        // TODO:SEQUOIADBMAINSTREAM-5974
        System.out.println( "userInfo = " + userInfo.toString() );
        List< BasicDBObject > user = ( List< BasicDBObject > ) userInfos
                .get( "users" );
        System.out.println( "user = " + user.toString() );
        // Assert.assertEquals( user,
        // Collections.singletonList( new Document( "user", username1 )
        // ) );

        // 创建连接
        MongoCredential mongoCredential = MongoCredential
                .createScramSha1Credential( username1, dbName,
                        username1.toCharArray() );
        MongoClient client2 = new MongoClient(
                new ServerAddress( config.getHost(), config.getPort() ),
                Collections.singletonList( mongoCredential ), opt );

        MongoCredential mongoCredentia2 = MongoCredential
                .createScramSha1Credential( username2, dbName,
                        username2.toCharArray() );
        MongoClient client3 = new MongoClient(
                new ServerAddress( config.getHost(), config.getPort() ),
                Collections.singletonList( mongoCredentia2 ), opt );

        DB db1 = client1.getDB( dbName );
        // 创建集合
        db1.createCollection( clName, new BasicDBObject() );

        DB db2 = client2.getDB( dbName );
        DBCollection cl2 = db2.getCollection( clName );
        // 增加/查询记录
        cl2.insert( list );
        List< DBObject > actList = cl2.find().toArray();
        Assert.assertEquals( actList, list );

        DB db3 = client3.getDB( dbName );
        DBCollection cl3 = db3.getCollection( clName );
        DBObject query;
        // 删除记录
        query = new BasicDBObject( "a", 0 );
        Assert.assertEquals( cl3.count( query ), 1 );
        cl3.remove( query );
        Assert.assertEquals( cl3.count( query ), 0 );

        // 删除用户
        db2.command( new BasicDBObject( "dropUser", username1 ) );
        db3.command( new BasicDBObject( "dropUser", username2 ) );
        // 重复删除用户
        db2.command( new BasicDBObject( "dropUser", username1 ) );

        // 查询所有用户
        CommandResult userInfos1 = db1
                .command( new BasicDBObject( "usersInfo", 1 ) );
        List< BasicDBObject > users1 = ( List< BasicDBObject > ) userInfos1
                .get( "users" );
        System.out.println( "userInfos = " + users1.toString() );

        // 删除集合
        cl3.drop();

        // 使用不存在的用户创建连接
        MongoCredential mongoCredentia3 = MongoCredential
                .createScramSha1Credential( "noexists", dbName,
                        "noexists".toCharArray() );
        MongoClient client4 = new MongoClient(
                new ServerAddress( config.getHost(), config.getPort() ),
                Collections.singletonList( mongoCredentia3 ), opt );
        DB db4 = client4.getDB( dbName );
        DBCollection cl4 = db4.getCollection( clName );
        try {
            cl4.insert( new BasicDBObject( "a", 1 ) );
        } catch ( Exception e ) {
            if ( !e.getMessage()
                    .contains( "Authentication has been disabled" ) ) {
                throw e;
            }
        }
        db1.dropDatabase();
        client2.close();
        client3.close();
    }

    @AfterClass
    public void tearDown( ITestContext context ) {
    }

}
